# Synth One-only native voice sharing and Room8 replay

## Voice sharing

`morph-and-env.patch` follows `osc-phase.patch`. It shares the original waveform index/blend and amplitude decay math across voices inside each sample. For wet testing, apply against the rendering section of the 64-frame effects worker, keeping the existing effects arena and algorithms. No production flag changed.

Pinned 360 MHz Elite emulator, strict no-skip, 8-second disposable benchmark. The four-voice wet test uses existing compact Delay (300 ms, actual stride 38) and Room, not either new research prototype.

| Workload | Change | Held peak (µs) | Held halves above 10,809 µs | DMA XRUNs | Free app flash | Pool margin |
|---|---|---:|---:|---:|---:|---:|
| 6 dry | Prior optimized | 14,400.92 | 128/128 | 147 | 4,148 B | 26,492 B |
| 6 dry | Phase cache | 12,813.92 | 128/128 | 147 | 4,100 B | 26,492 B |
| 6 dry | Phase plus morph decode | 12,667.42 | 128/128 | 147 | 4,020 B | 26,492 B |
| 6 dry | Phase plus envelope decay | 12,540.00 | 128/128 | 147 | 4,036 B | 26,492 B |
| 6 dry | Phase, morph and envelope | 12,356.33 | 128/128 | 147 | 3,956 B | 26,492 B |
| 4 wet | Prior overlap baseline (-Os) | 12,213.71 | 128/128 | 147 | 660 B | 15,228 B |
| 4 wet | Phase cache | 11,904.83 | 128/128 | 33 | 580 B | 15,228 B |
| 4 wet | Phase, morph and envelope | 11,637.21 | 128/128 | 1 | 388 B | 15,228 B |

All listed dry captures are byte-identical over 320,000 signed-16 stereo frames. Wet DMA captures diverge after cadence changes and are not a sound equivalence test. Wet fully combined still exceeds the physical period once, misses the project target every held half, and is not qualified for Pro, other scenes, or shipping.

## Room8 same-input wet-kernel replay

Use the owner-provided archive `DnBOX-room8-onset-correction.zip` (SHA256 `318b8e21c2378e8fb8c78b137bec36fad0629781b0d8b68567feb2f72688bdf4`). Use its existing executable source and `score.py`. Use the same taped stereo post-input-compressor/pre-reverb samples, raw original wet return columns, and recorded feedback. No time shift or level normalization. These are matches at the isolated reverb boundary, not an entire preset, board or output release. `organ` repeats effectively the 113 material from a separate source capture and is not a third independent preset.

Input SHA256:
- `062-room.tap`: `4987532f4449ba3d7fb5d331f3ae5a68ac118ad686c6650569a64cf31a2a1e09`
- `113-room.tap`: `c89e3a43ed7ec91b18143ca58b22764e70177284c9e73a5b76928ce74434334b`
- `original-room.tap`: `09d3fcf1ed1c92bc441b3c1c37dcda796c99ec3b6b08b6e8769205cd1e079716`

Put `room8-float-ingress-probe.h` beside the supplied C sources and change the include in a diagnostic copy of `replay_taps_trim.c` from `taps_trim.h` to this header. Compile the isolated variants with `cc -O2 -ffp-contract=off variant.c -lm -o variant`, run each with the tap and an output path, then run the supplied `score.py`. The new float probe retains the exact same tap ages and trimmed tank, replacing only packed ingress history with full precision; it is a causal control, not a RAM candidate. Float ingress leaves the missing tail substantially unchanged. Untrimmed taps reduce 062's tail error but not 113's. Do not fix by adjusting individual preset gain, or by assuming higher ingress precision will restore the recirculating tail. The new source-aware Delay archive still needs full-sized original feedback histories and is reference-only.

See `room8-matched-input-results.json` for the measured score subset. No reference audio or archive is included in this receipt.
