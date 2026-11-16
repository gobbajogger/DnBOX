/* Synthetic wire/input boundaries. No private model values. */
#define main retained_protocol_main
#include "sj6_model_unit_test.c"
#undef main

int main(void) {
    unsigned char p[530];
    int16_t table[512];
    Sj6MathModel m;
    unsigned i;
    synthetic(p, sizeof p);
    w32(p + 24, 10); /* Original interpolation defines this virtual sample as +0. */
    seal(p, sizeof p);
    memset(&m, 0xa5, sizeof m);
    {
        int ok = sj6_math_open(&m, p, sizeof p) == 0;
        expect("original-compatible virtual-zero endpoint accepted", ok);
        expect("virtual endpoint returns zero without source access", ok && sj6_math_at(&m, 0, table, 10, 0) == 0.f);
    }
    reject_case(p, sizeof p, 24, 11);

    synthetic(p, sizeof p);
    w16(p + 82, 0); /* Pure synthetic DC tests int16 quantizer boundary. */
    wf(p + 48, 32767.5f);
    seal(p, sizeof p);
    expect("positive boundary model opens", sj6_math_open(&m, p, sizeof p) == 0);
    expect("positive nearest-even overflow rejects instead of clipping", sj6_math_build(&m, table, 512) != 0);
    wf(p + 48, -32768.5f);
    seal(p, sizeof p);
    expect("negative tie model opens", sj6_math_open(&m, p, sizeof p) == 0);
    expect("negative tie rounds to representable even integer", sj6_math_build(&m, table, 512) == 0);
    for (i = 0; i < 512; ++i) expect("every negative tie is exactly INT16_MIN", table[i] == -32768);
    wf(p + 48, -32768.75f);
    seal(p, sizeof p);
    expect("negative overflow model opens", sj6_math_open(&m, p, sizeof p) == 0);
    expect("negative overflow rejects instead of clipping", sj6_math_build(&m, table, 512) != 0);
    printf("%u inclusive-loop/quantizer boundary checks, %u failed\n", checks, failed);
    return !!failed;
}
