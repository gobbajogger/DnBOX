# Synth One: current fit boundary

The refined Synth One comparison is a host sound benchmark. The independent 906-program run has all stereo residuals at or below −40 dBr on the declared six-note render, not an integrated Elite/Pro build or playback qualification.

Exact native compact diagnostic (same S1C effect allocation on both boards): Elite 244 B and Pro 256 B free app flash; audio pool 9,100 B free before 8,192 B reserve and 512 B required remaining, leaving 396 B truly available. That image uses compact effects, not the above host graph. Its combined existing per-Synth-One FX arena is 4,252 B, including the delay; the room slice is 2,100 B. The +4,662 B direct DSP flash projection and +14,021 B standalone-runtime projection have different local dependencies; neither is the integrated flash overage.

## Exact delay horizon change (host)

The source host initialized all four variable delay buffers for 10 seconds even though the admitted physical control is bounded to 2.5 seconds. The reached output branches read `2t, 2t, t, t`. New physical backing retains the original 10-second **logical** cursor and interpolation, with (5s+8) slots worth of frames for each long branch and (2.5s+8) for each short branch. Fails closed outside the tested physical range. This is a history capacity change, not a changed time control.

- Whole accepted comparison: 3,684,000 frames across 44.1/48 kHz, no changed Float32 samples, including active feedback, filter/mix, retime, resets and resolved source tempo-sync changes.
- 44.1 kHz delay history shrank from 7,056,016 B to 2,646,136 B per owner. Combined with uncompressed room (98,760 B) and Float32 stereo widen (8,820 B), histories still total 2,753,716 B per owner. These are implementation sizes, not information-theoretic lower bounds. No native fit/timing.
- Source/audit capsule SHA-256 `90d9de0edf518ad25dd37af9a6da912d981dc86aea16b8945f27c591812ce50f`.

## Independent exact room storage census

Captured all eight real Float32 RevSC rings every 4,096 output frames for the matched-input Dark Theater Organ cue. Instrumented room return remained sample-for-sample identical to the original. On warm sustained snapshots: full-snapshot DEFLATE 70,439–77,067 B; full-snapshot LZMA 69,368–75,740 B; optimistic 256/1024 B random-access DEFLATE pages with offset table 86,941–91,015 / 81,448–85,534 B. Tests choose the best of plain, XOR, byte shuffle and combined, without charging mode bytes. These are offline space measurements only. Straightforward lossless packing in this witness does not fit the existing 2.1 KB room or approach the smaller lossy-format trials, and would add decoder/write costs to qualify.

The global mix delay's 14-bit packing is already linked (138,601 B), so the 19,799 B nominal saving is not available twice. Permanent room histories cannot borrow independently live main FX, another synth's state or the LCD canvas. Work proceeds on effect-specific history representation plus independently validated app/performance savings.
