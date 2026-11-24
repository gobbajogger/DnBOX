/* SPDX-License-Identifier: GPL-3.0-only */
/* Device configuration of the compact Synth One engine (option B: its own delay and reverb).
 * RAM per part (pi32v2): the engine core and widen allpasses live in the part's backend union
 * (<= 3,720 B); reverb lines and the delay pool borrow the part's Chicago FX2 arena (4,252 B),
 * which is idle while the part plays Synth One. Changing a size here changes those budgets:
 * tests/host/s1c_programs_test.c checks both. */
#ifndef S1C_CONFIG_H
#define S1C_CONFIG_H
#define S1C_RVB 1
#define S1C_RVB_DEC 2                 /* reverb network at sr / 2, linear interpolation back up */
#define RVB_L0 350                    /* line lengths at sr / 2 */
#define RVB_L1 414
#define RVB_L2 476
#define RVB_L3 548
#define S1C_RVB_BFP 1                 /* 8-bit block-float reverb lines */
#define WAP_L0 31                     /* widen: three h16 Schroeder allpasses */
#define WAP_L1 73
#define WAP_L2 131
#define S1C_DLN 5                     /* C_T(R) + C_T(C_2T(R)) == C_T(R + C_2T(R)) */
#define S1C_DLMONO 1                  /* no auto-pan and no phaser: one line serves L and R */
#define S1C_HFR 0                     /* no HF regeneration: decimated long delays just get darker */
#define S1C_HFR_ATT 0.0005f
#define S1C_HFR_REL 0.0005f
#define S1C_FAST 1                    /* per-block coefficients, table sines */
#define S1C_KR 8                      /* voice control rate: every 8 samples */
#define S1C_CSM 1                     /* only the 46 smoothed parameters keep a current value */
#define S1C_EXT 1                     /* caller-owned buffers, exact-size delay lines */
#define S1C_OPT_A 1
#define S1C_ADPCM 1                   /* per-program delay format; the device uses h16 or 8-bit block float */
#define S1C_FASTMATH 1
#ifndef S1C_NO_SEQ
#define S1C_NO_SEQ 1                  /* the shared firmware arp (engine_arp) plays S1 arps */
#endif
#define S1C_DL_H16 1                  /* delay lines: half floats only */
#define S1C_DL_FBK 0.25f              /* compact delay: feedback trim for decimated lines (by ear vs original 062/032/289) */
#define S1C_DL_FBMAX 0.98f
#define S1C_DL_WET 1.6f               /* compact delay: wet trim for decimated lines */
#define S1C_RVB_NOMOD 1               /* fixed reverb taps (no modulation): -304 B XIP, tail level within 1 dB */
#define S1C_ARENA_BYTES 4252u         /* sizeof(original_chicago_fx2_compact) on pi32v2 */
#endif
