#include "sj6_blocks.h"
#include "lfo_laws.hpp"
#include "conductor.hpp"
#include <cstdint>
#include <cstdio>
#include <cstring>

static uint32_t bits(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return u;
}

int main()
{
    int lfo_d = 0, lfo_n = 0, beat_d = 0, beat_n = 0;
    for (int i = 0; i < 50; i++) {
        float phase = i / 50.f, rate = i * 0.3f, dt = 0.01f, a = -1.f, b = -2.f;
        auto st = original_conductor_v1::lfo_phase(phase, rate, dt, a);
        int ps = sj6_lfo_phase(phase, rate, dt, &b);
        lfo_n++;
        if (st != original_conductor_v1::Status::State || ps || bits(a) != bits(b)) lfo_d++;
        float tempo = 90.f + i, norm = i / 49.f;
        st = original_conductor_v1::lfo_rate(norm, false, tempo, {}, a);
        ps = sj6_lfo_rate_tempo(norm, tempo, &b);
        lfo_n++;
        if (st != original_conductor_v1::Status::State || ps || bits(a) != bits(b)) lfo_d++;
        std::int64_t ix = 0, iy = 1;
        st = original_conductor_v1::lfo_index(phase * 3.f - 1.f, ix);
        ps = sj6_lfo_index(phase * 3.f - 1.f, &iy);
        lfo_n++;
        if (st != original_conductor_v1::Status::State || ps || ix != iy) lfo_d++;
        for (unsigned w = 0; w < 6; w++) {
            st = original_conductor_v1::lfo_modulate(0.4f, phase * 2.f - 1.f, 0.3f, w, a);
            ps = sj6_lfo_modulate(0.4f, phase * 2.f - 1.f, 0.3f, w, &b);
            lfo_n++;
            if (st != original_conductor_v1::Status::State || ps || bits(a) != bits(b)) lfo_d++;
        }
        for (unsigned kind = 1; kind <= 4; kind++) {
            st = original_conductor_v1::shape_table_sample(kind, (unsigned)i, a);
            ps = sj6_shape_sample(kind, (unsigned)i, &b);
            lfo_n++;
            if (st != original_conductor_v1::Status::State || ps || bits(a) != bits(b)) lfo_d++;
        }
    }
    for (int i = 0; i < 400; i++) {
        float last = (i % 20) * 0.01f;
        float acc = (i % 7) * 0.001f;
        float beat = last + (i % 11) * 0.004f;
        float tempo = 60.f + (i % 9) * 10.f;
        original_conductor_v1::Beat s = original_conductor_v1::Beat::from_f32(1, last, acc);
        original_conductor_v1::Beat r;
        original_conductor_v1::Command cmd[32];
        std::size_t n = 99;
        auto st = original_conductor_v1::beat_catchup(s, original_conductor_v1::f64_from_f32(beat), original_conductor_v1::f64_from_f32(tempo), true, cmd, 32, n, r);
        Sj6Beat ps{1, original_conductor_v1::f64_from_f32(last), original_conductor_v1::f64_from_f32(acc)};
        Sj6Beat pr{};
        Sj6Tick ticks[32];
        int pn = 77;
        int pst = sj6_beat_catchup(&ps, original_conductor_v1::f64_from_f32(beat), original_conductor_v1::f64_from_f32(tempo), 1, ticks, 32, &pn, &pr);
        beat_n++;
        if (st != original_conductor_v1::Status::Clock || pst || n != (std::size_t)pn || r.accumulator != pr.accumulator || r.last != pr.last) beat_d++;
        else for (int k = 0; k < pn; k++) if (bits(cmd[k].value) != bits(ticks[k].value)) beat_d++;
    }
    std::printf("lfo diffs %d / %d\nbeat diffs %d / %d\n", lfo_d, lfo_n, beat_d, beat_n);
    if (lfo_d || beat_d) {
        std::printf("FAIL timing\n");
        return 1;
    }
    std::printf("PASS timing\n");
    return 0;
}
