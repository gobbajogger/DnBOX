/* Original factory controller and renderer. Replace only the eight-line reverb module. */
#include "full_fixture.hpp"
#include "oracle_controls.hpp"

extern "C" int sp_compact_create(sp_revsc **);
extern "C" int sp_compact_init(sp_data *, sp_revsc *);
extern "C" int sp_compact_compute(sp_data *, sp_revsc *, float *, float *, float *, float *);

constexpr unsigned rate = 44100, factor = 30;
enum Mode { original, multirate, shortened };
struct Measurement {
    Mode mode;
    unsigned phase{};
    float sum[2]{}, prior[2]{}, current[2]{};
    float input[2]{}, wet[2]{};
};
static Measurement* running;
static void measure_revsc(sp_data* sp, sp_revsc* reverb, float* l, float* r, float* outl, float* outr) {
    Measurement& s = *running;
    s.input[0] = *l; s.input[1] = *r;
    if (s.mode == original) {
        sp_revsc_compute(sp, reverb, l, r, outl, outr);
    } else if (s.mode == shortened) {
        reverb->lpfreq = .5f * reverb->sampleRate;
        reverb->feedback = std::pow(reverb->feedback, 1.f / factor);
        sp_compact_compute(sp, reverb, l, r, outl, outr);
    } else {
        reverb->lpfreq = .5f * reverb->sampleRate;
        s.sum[0] += *l; s.sum[1] += *r;
        if (++s.phase == factor) {
            s.prior[0] = s.current[0]; s.prior[1] = s.current[1];
            s.sum[0] /= factor; s.sum[1] /= factor;
            sp_compact_compute(sp, reverb, s.sum, s.sum+1, s.current, s.current+1);
            s.phase = 0; s.sum[0] = 0; s.sum[1] = 0;
        }
        const float t = float(s.phase) / factor;
        *outl = s.prior[0] + t * (s.current[0] - s.prior[0]);
        *outr = s.prior[1] + t * (s.current[1] - s.prior[1]);
    }
    s.wet[0] = *outl; s.wet[1] = *outr;
}
#define S1FX_EXISTING_PARAMETERS 1
#include "capture_fx.hpp"

struct Render {
    FullPair pair;
    s1frontend_oracle::OracleControls controls;
    SourceFx fx[3]{SourceFx(rate),SourceFx(rate),SourceFx(rate)};
    Measurement measures[3]{{original},{multirate},{shortened}};
    float* input = nullptr;
    FILE* output;
    float mono_mode = 0;
    Render(Fixture& f, FILE* output): pair(f, rate, false, false, 0), controls(pair.o), output(output) {
        for (unsigned k = 1; k < 3; ++k) {
            SourceFx& x = fx[k];
            sp_revsc_destroy(&x.reverbCostello);
            sp_compact_create(&x.reverbCostello);
            sp_data slow = *x.sp; slow.sr = rate / factor;
            sp_compact_init(&slow, x.reverbCostello);
        }
        std::printf("full_bytes=%zu compact_bytes=%zu full_lines=", fx[0].reverbCostello->aux.size, fx[1].reverbCostello->aux.size);
        for(const auto& line : fx[0].reverbCostello->delayLines) std::printf("%d,",line.bufferSize);
        std::putchar('\n');
    }
    static void trace(const OracleState& s, unsigned phase, unsigned frame, void* opaque) {
        if (phase != 1) return;
        auto& x = *static_cast<Render*>(opaque);
        if (x.mono_mode != s.parameters[isMono]) {
            x.mono_mode = s.parameters[isMono];
            for(auto& fx : x.fx) fx.source_reset_delays();
        }
        float outputs[3][4]{};
        for (unsigned k = 0; k < 3; ++k) {
            auto& fx = x.fx[k]; running = &x.measures[k];
            std::memcpy(fx.parameters.data(), s.parameters, sizeof(float) * 150);
            fx.frame(x.input[frame], &outputs[k][2], &outputs[k][3], s.lfo1_0_1, s.lfo1_1_0, s.lfo2_0_1, s.lfo2_1_0);
            std::memcpy(outputs[k], running->wet, sizeof running->wet);
            CHECK(std::memcmp(running->input, x.measures[0].input, sizeof running->input) == 0);
            for(float v : outputs[k]) CHECK(std::isfinite(v));
        }
        CHECK(std::fwrite(outputs, sizeof outputs, 1, x.output) == 1);
    }
    void block(unsigned n) {
        float l[128]{}, r[128]{}; input = l;
        pair.o.trace_fn = trace; pair.o.trace_context = this;
        compact_oracle_window = true; pair.o.process(n, 0, l, r); compact_oracle_window = false;
        pair.o.trace_fn = nullptr; pair.o.trace_context = nullptr;
    }
};
static unsigned word(FILE* f) { unsigned v{}; CHECK(std::fread(&v, sizeof v, 1, f) == 1); return v; }
int main(int argc, char** argv) {
    CHECK(argc == 6);
    unsigned selected = std::stoul(argv[4]); CHECK(selected < 694);
    Fixture fixture(argv[1]);
    FILE* f = std::fopen(argv[2], "rb"); CHECK(f);
    std::vector<unsigned char> pack(165292);
    CHECK(std::fread(pack.data(), 1, pack.size(), f) == pack.size() && std::fclose(f) == 0);
    compact_data = pack.data(); compact_size = pack.size();
    FILE* out = std::fopen(argv[5], "wb"); CHECK(out);
    Render render(fixture, out);
    f = std::fopen(argv[3], "rb"); CHECK(f && word(f) == 0x53315043 && word(f) == 694);
    for (unsigned row = 0; row < 694; ++row) {
        CHECK(word(f) == 144);
        for (unsigned i = 0; i < 144; ++i) {
            unsigned id = word(f); float value; CHECK(std::fread(&value, sizeof value, 1, f) == 1);
            if (row == selected) CHECK(render.controls.set_parameter(id, value));
        }
    }
    CHECK(std::fgetc(f) == EOF && std::fclose(f) == 0);
    render.pair.o.resetSequencer();
    for (unsigned pos = 0; pos < 3 * rate;) {
        if (pos == 0) render.pair.o.startNote(54,100);
        if (pos == rate/2) render.pair.o.startNote(61,93);
        if (pos == rate) {render.pair.o.stopAllNotes(); render.pair.o.startNote(66,116);}
        if (pos == 3*rate/2) render.pair.o.stopAllNotes();
        unsigned n = std::min(128u, rate / 2 - pos % (rate / 2));
        render.block(n); pos += n;
    }
    CHECK(std::fclose(out) == 0);
}
