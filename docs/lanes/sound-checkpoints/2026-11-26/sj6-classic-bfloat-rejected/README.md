# Rejected: bfloat16 short Classic chorus history

The host-only short-ring gain/headroom microbench replaces only the signed
16-bit word encoding with bfloat16 round-to-nearest-even. All finite
representable codes, signs including signed zero, and adjacent ties pass.
No per-preset or master gain change. Memory is still 512 B per owner for
the same timing and taps.

Do not adopt this shortcut: 65,536-frame stress-tone/wet errors were
−56.06 to −57.45 dBr, far worse than the same-size E4M11 format's roughly
−80 to −82 dBr. The quiet-tone result was already worse than Q15.
Although bit-casting bfloat is cheap, this candidate was not worth a
native qualification. No realistic alias, timing or full-fit claim.

Reproduce: `cc -O2 -ffp-contract=off probe.c -lm -o probe && ./probe`.
