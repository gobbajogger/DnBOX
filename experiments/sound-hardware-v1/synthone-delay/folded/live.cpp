// Original Soundpipe and two independent full-length folded histories.
#include "full_fixture.hpp"
#include "fold.hpp"
#include <cstdio>

struct OriginalLines {
    sp_data *sp;
    sp_vdelay *left, *right, *later, *fill;
    OriginalLines() {
        sp_create(&sp);
        sp->sr = 44100;
        for (sp_vdelay **p : {&left, &right, &later, &fill}) {
            sp_vdelay_create(p);
            sp_vdelay_init(sp, *p, 10.f);
        }
    }
    ~OriginalLines() {
        for (sp_vdelay **p : {&left, &right, &later, &fill}) sp_vdelay_destroy(p);
        sp_destroy(&sp);
    }
    void reset() {
        for (auto *p : {left, right, later, fill}) sp_vdelay_reset(sp, p);
    }
    void tick(float l, float r, float delay, float feedback, float &a, float &b) {
        float y1, y2;
        left->del = right->del = delay * 2.f;
        later->del = fill->del = delay;
        for (auto *p : {left, right, later, fill}) p->feedback = feedback;
        sp_vdelay_compute(sp, left, &l, &a);
        sp_vdelay_compute(sp, right, &r, &y1);
        sp_vdelay_compute(sp, fill, &r, &y2);
        sp_vdelay_compute(sp, later, &y1, &b);
        b += y2;
    }
};

int main(int argc, char **argv) {
    CHECK(argc == 2);
    FILE *capture = std::fopen(argv[1], "wb");
    CHECK(capture);
    OriginalLines src[2];
    FoldedDelay<float> full[2];
    FoldedDelay<_Float16> half[2];
    unsigned phase[2] = {129, 947};
    constexpr unsigned rate = 44100;
    for (unsigned i = 0; i < rate * 15; ++i) {
        for (unsigned owner = 0; owner < 2; ++owner) {
            auto &state = phase[owner];
            state = state * 1664525u + 1013904223u;
            float l = float(int(state >> 15) - 65536) / 262144.f;
            float r = .3f * std::sin(float(i) * (.0137f + float(owner) * .00097f));
            if (i < rate / 3 || (i > 9 * rate && i < 10 * rate)) l = r = 0.f;
            const float common = i < 5 * rate ? .19f : i < 7 * rate ? 2.5f :
                i < 10 * rate ? .015f + 2.4f * float(i - 7 * rate) / float(3 * rate) :
                i < 12 * rate ? .000362811785f : 2.5f;
            float sec = owner ? 2.500362811785f - common : common;
            float feedback = i < 6 * rate ? 0.f : i < 10 * rate ? .9f : .39f;
            if (i == 11 * rate + owner * 97u) {
                src[owner].reset(); full[owner].reset(); half[owner].reset();
            }
            float out[3][2];
            src[owner].tick(l, r, sec, feedback, out[0][0], out[0][1]);
            full[owner].frame(l, r, sec, feedback, out[1][0], out[1][1]);
            half[owner].frame(l, r, sec, feedback, out[2][0], out[2][1]);
            CHECK(std::fwrite(out, sizeof out, 1, capture) == 1);
        }
    }
    CHECK(std::fclose(capture) == 0);
}
