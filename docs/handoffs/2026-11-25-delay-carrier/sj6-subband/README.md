# SuperJ6 same-arena fixed-tap subband experiment

C host feasibility experiment only. Two real subbands are isolated before
16:1 decimation, encoded separately with byte-sized μ-law, then synthesized
through their original frequency bands. The upper branch uses bandpass
sampling: after reconstruction it keeps the 3 kHz image where it belongs
instead of using the much lower alias as the audible echo. Both branches
recirculate the combined reconstructed time-domain output. The two filter
centers sum to 320 samples of delay; ring offset plus phase accounts for
this and makes the total 11,025 samples at 44.1 kHz (250 ms).

State is 1,284 B shared stereo input scratch, 2,784 B of packed stereo
subband cells, 20 B of state bookkeeping: **4,088 B** in the C host ABI.
This is the persistent state only, excluding transient locals/stack
and 3,080 B filter/codec lookup constants. The direct implementation
does about 160 FIR weighted products per stereo frame; no target cost,
flash fit, or performance improvement is claimed.

Against the existing Build9-verified isolated host kernel, same signal,
static tap, wet 0.90, feedback ≈0.499, exact guarded compact baseline:

| Probe | Compact carrier (dB) | Prototype carrier (dB) | Compact strongest out-of-band component (dB) | Prototype strongest out-of-band component (dB) |
|---|---:|---:|---:|---:|
| 1 kHz first repeat | -5.888 | -1.389 | -15.683 | -50.726 |
| 1 kHz fourth repeat | -28.502 | -5.562 | -38.287 | -43.614 |
| 3 kHz first repeat | -65.546 | -1.377 | -22.107 (245 Hz alias) | -50.749 |
| 3 kHz fourth repeat | -66.711 | -5.498 | -23.275 (245 Hz alias) | -46.478 |

All per-row values refer to original carrier magnitude; arbitrary peak
alignment or normalization is not used. Low-level residual peaks need not
be aliases. At 3 kHz, first-echo normalized waveform residual improves
from +0.022 dB to -16.597 dB. Measured left-only excitation did not leak
into right. The abandoned 6 kHz branch yields no echo at all. 6 kHz,
other band gaps, low-level truncation, clock sync, 1.5s and 16s modes,
retime, live LFO, ping-pong, and actual instrument rendering are not
qualified. It's not a product candidate.

Reproduce:
```sh
python filter_gen.py > filters.inc
cc -O2 -std=c11 -ffp-contract=off -fPIC -shared subband.c -o subband.so
python probe.py > result.json
```
The script expects the existing `../delay-now/guarded-sj6.so` and
`../sj6-fx-bench/bench.py` for exact guarded-baseline comparison. The
C reference is the established host translation, not a fresh AU run.
