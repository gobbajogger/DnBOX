/* SPDX-License-Identifier: GPL-3.0-only */
/* The tuples come from tests/host/gen_arc8_table.c. Drop entries with zero
 * rounded coverage because px_blend does no drawing for them. */
#define ARC8_PIXELS \
    PIXEL(-2, -8,  4, 0x3ee5608du) \
    PIXEL(-1, -8,  7, 0x3ef27cddu) \
    PIXEL( 0, -8,  8, 0x3f000000u) \
    PIXEL( 1, -8,  7, 0x3f06c190u) \
    PIXEL( 2, -8,  4, 0x3f0d4fb9u) \
    PIXEL(-4, -7,  7, 0x3ec79a25u) \
    PIXEL(-3, -7, 14, 0x3ed40043u) \
    PIXEL(-2, -7, 16, 0x3ee1c158u) \
    PIXEL(-1, -7, 16, 0x3ef094d3u) \
    PIXEL( 0, -7, 16, 0x3f000000u) \
    PIXEL( 1, -7, 16, 0x3f07b595u) \
    PIXEL( 2, -7, 16, 0x3f0f1f53u) \
    PIXEL( 3, -7, 14, 0x3f15ffe0u) \
    PIXEL( 4, -7,  7, 0x3f1c32ecu) \
    PIXEL(-5, -6, 11, 0x3eb48300u) \
    PIXEL(-4, -6, 16, 0x3ec0214bu) \
    PIXEL(-3, -6, 16, 0x3ecd9f8bu) \
    PIXEL(-2, -6, 16, 0x3edd0868u) \
    PIXEL(-1, -6, 16, 0x3eee0de5u) \
    PIXEL( 0, -6, 16, 0x3f000000u) \
    PIXEL( 1, -6, 16, 0x3f08f90cu) \
    PIXEL( 2, -6, 16, 0x3f117bcbu) \
    PIXEL( 3, -6, 16, 0x3f19303cu) \
    PIXEL( 4, -6, 16, 0x3f1fef59u) \
    PIXEL( 5, -6, 11, 0x3f25be80u) \
    PIXEL(-6, -5, 11, 0x3ea0d255u) \
    PIXEL(-5, -5, 16, 0x3eaaa505u) \
    PIXEL(-4, -5, 16, 0x3eb6b160u) \
    PIXEL(-3, -5, 16, 0x3ec54b5bu) \
    PIXEL(-2, -5, 16, 0x3ed6a638u) \
    PIXEL(-1, -5, 16, 0x3eea8ca3u) \
    PIXEL( 0, -5, 14, 0x3f000000u) \
    PIXEL( 1, -5, 16, 0x3f0ab9b0u) \
    PIXEL( 2, -5, 16, 0x3f14ace5u) \
    PIXEL( 3, -5, 16, 0x3f1d5a53u) \
    PIXEL( 4, -5, 16, 0x3f24a74fu) \
    PIXEL( 5, -5, 16, 0x3f2aad7fu) \
    PIXEL( 6, -5, 11, 0x3f2f96d7u) \
    PIXEL(-7, -4,  7, 0x3e8dbb30u) \
    PIXEL(-6, -4, 16, 0x3e953408u) \
    PIXEL(-5, -4, 16, 0x3e9ea3f5u) \
    PIXEL(-4, -4, 16, 0x3eaaa505u) \
    PIXEL(-3, -4, 14, 0x3eba18b5u) \
    PIXEL(-2, -4,  6, 0x3ecd9f8bu) \
    PIXEL( 2, -4,  6, 0x3f19303cu) \
    PIXEL( 3, -4, 14, 0x3f22f3a5u) \
    PIXEL( 4, -4, 16, 0x3f2aad7fu) \
    PIXEL( 5, -4, 16, 0x3f30ae05u) \
    PIXEL( 6, -4, 16, 0x3f3565fbu) \
    PIXEL( 7, -4,  7, 0x3f392269u) \
    PIXEL(-7, -3, 14, 0x3e815515u) \
    PIXEL(-6, -3, 16, 0x3e87b5cbu) \
    PIXEL(-5, -3, 16, 0x3e9009fbu) \
    PIXEL(-4, -3, 14, 0x3e9b3ca0u) \
    PIXEL(-3, -3,  2, 0x3eaaa505u) \
    PIXEL( 3, -3,  2, 0x3f2aad7fu) \
    PIXEL( 4, -3, 14, 0x3f3261b0u) \
    PIXEL( 5, -3, 16, 0x3f37fb03u) \
    PIXEL( 6, -3, 16, 0x3f3c2519u) \
    PIXEL( 7, -3, 14, 0x3f3f5575u) \
    PIXEL(-8, -2,  4, 0x3e5fe990u) \
    PIXEL(-7, -2, 16, 0x3e6727f5u) \
    PIXEL(-6, -2, 16, 0x3e7099dbu) \
    PIXEL(-5, -2, 16, 0x3e7d5e3bu) \
    PIXEL(-4, -2,  6, 0x3e87b5cbu) \
    PIXEL( 4, -2,  6, 0x3f3c2519u) \
    PIXEL( 5, -2, 16, 0x3f40a871u) \
    PIXEL( 6, -2, 16, 0x3f43d989u) \
    PIXEL( 7, -2, 16, 0x3f463601u) \
    PIXEL( 8, -2,  4, 0x3f48059bu) \
    PIXEL(-8, -1,  7, 0x3e45b0ebu) \
    PIXEL(-7, -1, 16, 0x3e498105u) \
    PIXEL(-6, -1, 16, 0x3e4e8ee0u) \
    PIXEL(-5, -1, 16, 0x3e559165u) \
    PIXEL( 5, -1, 16, 0x3f4a9ba7u) \
    PIXEL( 6, -1, 16, 0x3f4c5c48u) \
    PIXEL( 7, -1, 16, 0x3f4d9fbfu) \
    PIXEL( 8, -1,  7, 0x3f4e93c5u) \
    PIXEL(-8,  0,  8, 0x3e2aaaabu) \
    PIXEL(-7,  0, 16, 0x3e2aaaabu) \
    PIXEL(-6,  0, 16, 0x3e2aaaabu) \
    PIXEL(-5,  0, 14, 0x3e2aaaabu) \
    PIXEL( 5,  0, 14, 0x3f555555u) \
    PIXEL( 6,  0, 16, 0x3f555555u) \
    PIXEL( 7,  0, 16, 0x3f555555u) \
    PIXEL( 8,  0,  8, 0x3f555555u) \
    PIXEL(-8,  1,  7, 0x3e0fa46bu) \
    PIXEL(-7,  1, 16, 0x3e0bd450u) \
    PIXEL(-6,  1, 16, 0x3e06c678u) \
    PIXEL(-5,  1, 16, 0x3dff87dbu) \
    PIXEL( 5,  1, 16, 0x3f600f05u) \
    PIXEL( 6,  1, 16, 0x3f5e4e63u) \
    PIXEL( 7,  1, 16, 0x3f5d0aebu) \
    PIXEL( 8,  1,  7, 0x3f5c16e5u) \
    PIXEL(-8,  2,  4, 0x3dead785u) \
    PIXEL(-7,  2, 16, 0x3ddc5abbu) \
    PIXEL(-6,  2, 16, 0x3dc976fbu) \
    PIXEL(-5,  2, 16, 0x3dafee30u) \
    PIXEL(-4,  2,  6, 0x3d8bd37bu) \
    PIXEL( 4,  2,  6, 0x3f6e8591u) \
    PIXEL( 5,  2, 16, 0x3f6a023bu) \
    PIXEL( 6,  2, 16, 0x3f66d120u) \
    PIXEL( 7,  2, 16, 0x3f6474a8u) \
    PIXEL( 8,  2,  4, 0x3f62a50fu) \
    PIXEL(-7,  3, 14, 0x3da55655u) \
    PIXEL(-6,  3, 16, 0x3d8bd37bu) \
    PIXEL(-5,  3, 16, 0x3d550575u) \
    PIXEL(-4,  3, 14, 0x3cf6e095u) \
    PIXEL(-3,  3,  2, 0x3834aaabu) \
    PIXEL( 3,  3,  2, 0x3f7ffd2du) \
    PIXEL( 4,  3, 14, 0x3f7848fbu) \
    PIXEL( 5,  3, 16, 0x3f72afa8u) \
    PIXEL( 6,  3, 16, 0x3f6e8591u) \
    PIXEL( 7,  3, 14, 0x3f6b5535u) \
    PIXEL(-7,  4,  7, 0x3d677bd5u) \
    PIXEL(-6,  4, 16, 0x3d2bb50bu) \
    PIXEL(-5,  4, 16, 0x3cc06b55u) \
    PIXEL(-4,  4, 16, 0x3834aaabu) \
    PIXEL( 4,  4, 16, 0x3f7ffd2du) \
    PIXEL( 5,  4, 16, 0x3f79fca5u) \
    PIXEL( 6,  4, 16, 0x3f7544b0u) \
    PIXEL( 7,  4,  7, 0x3f718843u) \
    PIXEL(-6,  5, 11, 0x3c9d8555u) \
    PIXEL(-5,  5, 16, 0x3834aaabu) \
    PIXEL( 5,  5, 16, 0x3f7ffd2du) \
    PIXEL( 6,  5, 11, 0x3f7b13d5u)

/* A 16-bit word holds X, Y and alpha, five bits each. Keep the exact binary32
 * turns in a separate, word-aligned array so neither array needs padding. */
#define PIXEL(x, y, alpha, bits) ((x) + 8) | (((y) + 8) << 5) | ((alpha) << 10),
static const uint16_t arc8_pixels[] = { ARC8_PIXELS };
#undef PIXEL
#define PIXEL(x, y, alpha, bits) bits,
static const uint32_t arc8_turns[] = { ARC8_PIXELS };
#undef PIXEL
#undef ARC8_PIXELS
