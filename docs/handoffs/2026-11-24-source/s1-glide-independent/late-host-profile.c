/* Host-only fixture: the real renderer and Sound, with 512-frame DMA halves
 * and a reproducible dense sequence. No native-image hooks or bank contents. */
#define main original_host_main
#include "x0x_host.c"
#undef main
#include "app/original_runtime.h"
#include "app/s1c_part.h"

#ifndef ATTRIBUTION
#define ATTRIBUTION 0
#endif
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr, "fixture line %d: %s\n", __LINE__, #x); exit(1); } } while (0)
#define PROFILE_HALVES 384u

#if ATTRIBUTION
static uint64_t prepare_ns[2], render_ns[2], prepare_count[2], render_frames[2];
static uint64_t s1_dsp_ns, s1_dsp_frames, arp_samples, arp_boundaries;
static unsigned measure;
void __real_s1_part_render(s1_dry_part *, float *, float *, unsigned);
void __wrap_s1_part_render(s1_dry_part *part, float *left, float *right, unsigned n)
{
    if (!measure) {
        __real_s1_part_render(part, left, right, n);
        return;
    }
    uint64_t start = host_ns();
    __real_s1_part_render(part, left, right, n);
    s1_dsp_ns += host_ns() - start;
    s1_dsp_frames += n;
}
int __real_S1Sequencer_willFire(const S1Sequencer *, const S1SequencerParams *, int);
int __wrap_S1Sequencer_willFire(const S1Sequencer *seq, const S1SequencerParams *params, int held)
{
    int edge = __real_S1Sequencer_willFire(seq, params, held);
    if (measure) {
        arp_samples++;
        arp_boundaries += edge != 0;
    }
    return edge;
}
static unsigned part_index(const original_runtime_part *part)
{
    REQUIRE(part->kind == ORIGINAL_SOUND_SYNTHONE || part->kind == ORIGINAL_SOUND_MIAMI);
    return part->kind == ORIGINAL_SOUND_MIAMI;
}
int __real_original_runtime_prepare(original_runtime_part *, float);
int __wrap_original_runtime_prepare(original_runtime_part *part, float bpm)
{
    if (!measure) return __real_original_runtime_prepare(part, bpm);
    uint64_t start = host_ns();
    int status = __real_original_runtime_prepare(part, bpm);
    uint64_t elapsed = host_ns() - start;
    unsigned i = part_index(part);
    prepare_ns[i] += elapsed;
    prepare_count[i]++;
    return status;
}
int __real_original_runtime_render(original_runtime_part *, original_runtime_workspace *,
                                   float *, float *, unsigned);
int __wrap_original_runtime_render(original_runtime_part *part, original_runtime_workspace *work,
                                   float *left, float *right, unsigned n)
{
    if (!measure) return __real_original_runtime_render(part, work, left, right, n);
    uint64_t start = host_ns();
    int status = __real_original_runtime_render(part, work, left, right, n);
    uint64_t elapsed = host_ns() - start;
    unsigned i = part_index(part);
    render_ns[i] += elapsed;
    render_frames[i] += n;
    return status;
}
#endif

static void load_fixture(const char *objects, const char *factory)
{
    static uint8_t pack[1 << 17];
    FILE *f = fopen(factory, "rb");
    REQUIRE(f);
    size_t size = fread(pack, 1, sizeof pack, f);
    REQUIRE(size > 0 && size < sizeof pack && !ferror(f));
    REQUIRE(fclose(f) == 0 && s1_part_set_pack(pack, (uint32_t)size));
    f = fopen(objects, "rb");
    REQUIRE(f);
    unsigned object, bytes, mask = 0;
    while (fread(&object, sizeof object, 1, f) == 1) {
        REQUIRE(object < OBJ_NOBJ && !(mask & (1u << object)));
        REQUIRE(fread(&bytes, (sizeof bytes), 1, f) == 1 && bytes <= PLAT_STORE_MAX);
        REQUIRE(fread(store[object], 1, bytes, f) == bytes);
        store_len[object] = (int)bytes;
        mask |= 1u << object;
    }
    REQUIRE(feof(f) && !ferror(f) && (mask & (1u << OBJ_SOUND)));
    REQUIRE(fclose(f) == 0);
    boot();
    REQUIRE(store_len[OBJ_SOUND] == 4 + sizeof proj.sound);
    REQUIRE(!memcmp(store[OBJ_SOUND] + 4, &proj.sound, sizeof proj.sound));
    REQUIRE(engine_bass_status(0) > 0 && engine_bass_status(1) > 0);
    engine_arp_settings arp;
    REQUIRE(engine_arp_settings_read(0, &arp) && arp.on);
}

static void dense_sequence(void)
{
    pattern_t *pat = &proj.pat[0];
    for (unsigned kit = 0; kit < NKIT; kit++) {
        pat->drum[kit].len = 16;
        pat->drum[kit].rate = RATE_16;
        for (unsigned voice = 0; voice < NDRUM; voice++)
            for (unsigned step = 0; step < 16; step++)
                if (kit == 0 ? (step + voice) % (2u + voice % 3u) == 0
                             : (step + voice * 2u) % 8u == 0)
                    sm_set(pat->drum[kit].hit[voice], step);
    }
    for (unsigned voice = 0; voice < NBASS; voice++) {
        pat->bass[voice].len = 16;
        pat->bass[voice].rate = RATE_16;
        for (unsigned step = 0; step < 16; step++)
            pat->bass[voice].step[step] = (bstep_t){48 + 5 * voice + step % 7,
                                                   G_NOTE | (step % 4 == 0 ? BS_ACCENT : 0), 100};
    }
    for (unsigned part = 0; part < NPARTS; part++) seq.ppat[part] = 0;
    seq.ch[PART_303A] = 0;
    seq.ch[PART_303B] = 1;
    seq.bpm = 145;
    seq.clk_src = SEQ_CLK_INT;
    engine_guard_enable(0);
    plat_cpu2_set(0);  /* The host cannot reproduce native overlap or DMA deadlines. */
    seq_start(&seq);
}

static void note(unsigned channel, unsigned key, unsigned velocity)
{
    unsigned status = (velocity ? 0x90u : 0x80u) | channel;
    REQUIRE(mi_w - mi_r < MQ);
    host_midi_put((status >> 4) | status << 8 | key << 16 | velocity << 24);
}

int main(int argc, char **argv)
{
    REQUIRE(argc == 6);
    load_fixture(argv[1], argv[2]);
    FILE *pcm = fopen(argv[3], "wb");
    FILE *csv = fopen(argv[4], "w");
    midi_log = fopen(argv[5], "w");
    REQUIRE(pcm && csv && midi_log);
    dense_sequence();
    fputs("half,total,909,808,s1,miami,break,fx,master,seq,prepare_s1,prepare_miami,"
          "render_s1,render_miami,prepare_s1_calls,prepare_miami_calls,render_s1_frames,render_miami_frames,"
          "s1_dsp,s1_dsp_frames,arp_samples,arp_boundaries\n", csv);
    unsigned arp_steps = 0, previous_step = 0;
    unsigned midi_inputs = 0, drums[2] = {0, 0};
    for (unsigned half = 0; half < PROFILE_HALVES; half++) {
        now_ms = (uint32_t)(((uint64_t)half * 512000u) / 44100u);
        if (half % 16 == 0) {
            for (unsigned channel = 0; channel < 2; channel++)
                for (unsigned key = 0; key < 3; key++) {
                    note(channel, 52 + channel * 3 + key * 5, half % 32 == 0 ? 108 : 0);
                    midi_inputs++;
                }
        }
        uint32_t before[ENG_PROF_N];
        for (unsigned i = 0; i < ENG_PROF_N; i++) before[i] = eng_prof[i];
#if ATTRIBUTION
        memset(prepare_ns, 0, sizeof prepare_ns);
        memset(render_ns, 0, sizeof render_ns);
        memset(prepare_count, 0, sizeof prepare_count);
        memset(render_frames, 0, sizeof render_frames);
        s1_dsp_ns = s1_dsp_frames = arp_samples = arp_boundaries = 0;
        measure = 1;
#endif
        int32_t output[1024];
        uint64_t start = host_ns();
        engine_render(output, 256);
        engine_volume(output, 256);
        engine_render(output + 512, 256);
        engine_volume(output + 512, 256);
        uint64_t elapsed = host_ns() - start;
#if ATTRIBUTION
        measure = 0;
#endif
        REQUIRE(seq.playing && mi_w == mi_r);
        REQUIRE(engine_bass_status(0) > 0 && engine_bass_status(1) > 0);
        REQUIRE(fwrite(output, sizeof *output, 1024, pcm) == 1024);
        fprintf(csv, "%u,%llu", half, (unsigned long long)elapsed);
        for (unsigned i = 0; i < ENG_PROF_N; i++) fprintf(csv, ",%u", eng_prof[i] - before[i]);
#if ATTRIBUTION
        const uint64_t values[] = {prepare_ns[0], prepare_ns[1], render_ns[0], render_ns[1],
                                   prepare_count[0], prepare_count[1], render_frames[0], render_frames[1],
                                   s1_dsp_ns, s1_dsp_frames, arp_samples, arp_boundaries};
        for (unsigned i = 0; i < sizeof values / sizeof values[0]; i++)
            fprintf(csv, ",%llu", (unsigned long long)values[i]);
#endif
        fputc('\n', csv);
        unsigned step;
        uint32_t age;
        if (engine_arp_view(0, &step, &age) && step != previous_step) {
            previous_step = step;
            arp_steps++;
        }
        for (unsigned kit = 0; kit < NKIT; kit++)
            for (unsigned voice = 0; voice < NDRUM; voice++)
                if (eng_hit[kit][voice]) drums[kit] |= 1u << voice;
    }
    REQUIRE(!fclose(pcm) && !fclose(csv) && !fclose(midi_log));
    REQUIRE(drums[0] == 2047 && drums[1] == 2047 && arp_steps >= 8);
    printf("fixture: %u halves, %u MIDI inputs, arp transitions %u, drum masks %u/%u, Sound %zu bytes\n",
           PROFILE_HALVES, midi_inputs, arp_steps, drums[0], drums[1], sizeof proj.sound);
}
