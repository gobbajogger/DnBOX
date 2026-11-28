# Map and test the held-note critical path

The starting point is [the 48/49-dropout paired diagnostic](owner48-release-dsp-result.md)
at `d5c0f2cfbf106f34a26d7824f9903033ecc8cea3`. A fresh canonical Elite replay
reproduced the exact receipt and package (48 XRUN, 34/128 held blocks over
the physical period, 12,751.21 µs worst held; 1,376 B XIP, 43,316 B pool
margin past the 8,192 B reserve). The 10,080 B display RAM cut remains intact.

## Why these paths

[Native PC weights](owner48-held-pc-profile.txt) came from the exact
uninstrumented Elite ELF, pinned emulator, canonical controller, fresh
virtual NOR, strict/no-skip, with window `FM1_TPROF=pcs.csv,3200,4100` and
a 4,200 ms shot. Each core logged 80,991,179 weights; the denominator
includes foreground/interrupt time, not just audio. No cycles were skipped.
Use [the mapper](owner48-map-held-pcs.py) on the raw emulator CSV and same ELF.

- Core 0: Synth One voice loop 21.01%, outlined morph 10.64%, LFO 1.87%,
  envelope 1.55%. Equivalent waveform control pairs actually render
  `k=2` (pulse to saw), not `k=1` as a preset-only reading implied.
  The owner RAM's I1/I2 targets are 1.0, current values ~0.9998104,
  and the morph maps position to `floor(3 * pos)`. BLE is off and the
  general waveform path has zero sampled execution. The existing
  precision/filter law is not changed.
- Core 1: `d9_render` 24.88%, 808 cymbal 8.69%, 808 voice loop 5.74%,
  808 metal bank 4.16%. Modeled misses and weighted PCs are emulator
  evidence, not real hardware cache measurements. Native RAM shows all
  four sampled 909 voices have `inc=1`, `incf=47139328` (~1.01097548×),
  nonzero phase fractions. Fractional-pitch interpolation is **not a
  cold path** in this fixture. Do not blindly outline it.
- The 808 lane renderer mutates a shared right output pointer and adds
  to buses in a fixed order; concurrent reuse is not safe just because
  the lane filters are disjoint. The existing worker's scratch/state
  separation must be retained. The final mix still needs ordered input.

## Same-controller strict Elite comparison, applied individually

All runs used the same six dry voices, both real sequenced kits,
132 held hits per kit, 128 held blocks, zero skips and zero oversize.
The tests failed the zero-dropout deadline even when a single peak moved.

| Independent arm | Full-run XRUN | Held worst, µs | Held misses >11.61 ms | Free app XIP |
| --- | ---: | ---: | ---: | ---: |
| Reproduced exact base | **48** | 12,751.21 | **34** | **1,376 B** |
| Pass voice index directly | 60 | 12,837.71 | 47 | 1,424 B |
| Hold unchanged morph decode | 61 | 12,804.46 | 52 | 1,184 B |
| Exact frozen ADSR check | 61 | 12,712.25 | 48 | 1,072 B |
| Pass shared per-frame voice controls | 61 | 12,781.21 | 48 | 1,328 B |
| Count only the quiet bass suffix | 61 | 12,759.13 | 51 | 1,296 B |
| One monotonic bass meter scale per span | 54 | 12,766.96 | 40 | 1,232 B |
| Inline actual two-oscillator pulse/saw path | 60 | 12,634.17 | 52 | 1,216 B |
| Master previous-prefix overlap | 47 | 12,680.00 | 37 | 1,312 B |
| 808 symmetric filter-product reuse | 59 | 12,800.17 | 48 | 1,408 B |
| 808 internal voice no-alias | 55 | 12,778.54 | 46 | 1,344 B |
| 909 deferred tom-current reads | 49 | 12,807.08 | 39 | 1,296 B |
| 909 symmetric biquad products | 66 | 12,875.00 | 54 | 1,232 B |
| Lossless display hash and matrix paint trims | 65 | 12,827.54 | 54 | 1,280 B |

Host exactness alone is insufficient: the first four voice arms matched
3,357,120 stereo frames in 37,440 spans including whole synth state, but
did not advance the timed result. The ordinary ADSR matrix didn't show
a reduced envelope call count, so it is not evidence of hitting the
frozen fast path. The quiet suffix model checked 9,312,896 cases and bass
meter reduction checked 400,000, including signed zeros, NaNs, threshold
edges and saturation. The pulse/saw helper matched 83.5 MB of host
voice output/state. The master overlap matched host DSP and state for
3,355,865 frames but is mixed natively: one fewer whole-run dropout,
three more held deadline misses, and less flash. No native audio
bit-match is claimed for a rejected patch.

None of the single-arm results in this table justify replacing the
two-board checkpoint. See the separate prestage comparison below. This is still the diagnostic path only; neither a wet FX
or release pass nor a production change.

The real near-unity fractional 909 sampler path was also prototyped with
an aligned decoder and proven sufficient sample runway. Its host
full-state/four-output comparison passed 23,280 blocks at each of two
optimization levels. The native link consumed 400 B, leaving **976 B**
free, below the 1,024 B diagnostic floor. It was not timed or
included. See [patch](owner48-held-909-fractional-runway.patch) and
[native sampler input readback](owner48-held-909-fractional-ram.txt).

## Viable diagnostic alternate: use some of the existing wait for mix preparation

Apply [the prestage patch](owner48-held-bass-prestage.patch) alone after
reconstructing the four-patch base. On the reverse stress route only,
Core 0 prepares Bass A's level-meter value and mono effect-send source
before waiting for the 808's ordered contribution. The fixture has
retired B, and that route keeps both bass jobs serial on Core 0, so the
temporary mono source fits in B's idle buffer. The meter result is still
published in the original order, and all dry and effect buses still
accumulate in the original stage order. It changes no floating-point
expression in those loops, DMA size, voice count, or kit controls.

| Canonical loaded test | Elite before → prestage | Pro before → prestage |
| --- | ---: | ---: |
| Full-run XRUNs | 48 → **43** | 49 → **46** |
| Worst held block, µs | 12,751.21 → 12,747.92 | 12,687.83 → 12,746.67 |
| Held blocks over physical period | 34/128 → 34/128 | 39/128 → 33/128 |
| Held blocks over 10,809 µs target | 125/128 → 128/128 | 126/128 → 128/128 |
| Free app XIP | 1,376 → 1,168 B | 1,384 → 1,176 B |
| Pool beyond unchanged reserve | 43,316 B | 43,316 B |

Raw timings: [Elite](owner48-held-bass-prestage-elite.json),
[Pro](owner48-held-bass-prestage-pro.json). Both preserve 10,080 B
display RAM savings, six actual on/off events, 132 held kit hits each,
zero skipped cycles, and no oversized draw request. All timings are
native-target code in the pinned emulator on fresh virtual NOR.

This is a trade-off, not an across-the-board replacement: total
dropouts fall on both, but Pro's worst held block is 59 µs slower, every
held block exceeds the engineering target, and 208 B of app flash is used.
The onset/release alternate acts only on the special fixture, relies on
the retired part's exclusive buffer, and is not safe to enable for normal
playback without new ownership/failure tests. Keep both source checkpoints.

Separate full native fingerprint pairs matched all 160 ordered pre-DMA
blocks on [Elite](owner48-held-bass-prestage-elite-fingerprints.txt) and
[Pro](owner48-held-bass-prestage-pro-fingerprints.txt). The separate
fingerprint instrumentation is not included in the timing above. It
does not prove parity for wet effects or other musical scenes.
