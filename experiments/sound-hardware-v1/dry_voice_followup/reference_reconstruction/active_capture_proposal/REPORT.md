# One new original active-filter reference

**Terminal success; host renderer allocation released.** Exactly one original-only active-filter capture ran, with no compiler/linker invocation, retry, source repair, control adjustment, normalization, alignment or tail edit. No further capture/build work is allocated here.

## New numeric reference

- Exact case ID: `filter-modulation/1.2_2000_0.9`.
- New mono output: **1,058,304 bytes**, SHA-256 **`3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74`**.
- Layout: original left mono, little-endian IEEE754 binary32.
- Sample rate: **44,100 Hz**; velocity **100**; notes **36, 60, 84**.
- Requested cutoff: **2,000 Hz**; resonance binary32 **0.8999999761581421**; depth binary32 **1.2000000476837158**.
- Active filter ADSR: **0.2 / 0.3 / 0.4 / 0.5**, represented by the exact binary32 values in the pinned case plan.

The raw numeric stream is [outputs/filter-modulation_1.2_2000_r0.9.original-mono.f32le](outputs/filter-modulation_1.2_2000_r0.9.original-mono.f32le). The numeric-only reference manifest is [reference_capture_manifest.json](reference_capture_manifest.json), SHA-256 `ea1a572e0d0b682a938fc83bdb7f7dd535bb645efd4b749576d039af64710044`.

## Genuine terminal counts

| Quantity | Actual |
| --- | ---: |
| Renderer invocations | 1 |
| Process exit code | 0 |
| Timeout | No; one 15-second bound |
| Fresh original instances / completed notes | 3 |
| Silent warmup frames, all notes | 529,344 |
| Retained frames, all notes | 264,576 |
| Total original reference frames | 793,920 |
| Process calls, 64 frames each | 12,405 |
| Note-off frame in each retained note | 66,112 |
| Warmup mono nonfinite samples | 0 |
| Retained mono nonfinite samples | 0 |
| Compiler/linker invocations | 0 |
| Retries or additional active cases | 0 |

Each note retains all 88,192 declared frames after 176,448 silent warmup frames. The full stdout, stderr and terminal receipt were preserved. [metadata/active-capture.receipt.json](metadata/active-capture.receipt.json) is the genuine process receipt, not a reconstructed success summary.

## Control distinction from the physical baseline

The physical baseline used filter ADSR **0.05 / 0.05 / 1 / 0.5**. This exact active case uses the historical modulation ADSR **0.2 / 0.3 / 0.4 / 0.5**. Only filter depth (parameter 19), filter attack (22), filter decay (23) and filter sustain (24) differ. The other **146 physical controls remain bitwise identical**; filter release remains 0.5. Common oscillator, amplitude, tuning and event controls remain unchanged.

The exact 600-byte active control-vector SHA-256 is `6f7e9834054579c2d61e7e33072c2651107e3e987a75aa59c1b222d3f5164ff4`. The one executed argv array and all three runtime input identities are in [executed_command.json](executed_command.json).

## Original-reference identity and boundaries

The unchanged successful executable is **63,936 bytes**, SHA-256 `edf9f733ff449ecf345f9843952dea5d7cdab00825de88e4d7c3c61e6135f91d`. Its new public wrapper SHA-256 is `9ddfb70be9b9d26cc7d2b27f20ce228aeab197c3a9f8a65decc5c42a99f5a2b4`. Before this active capture, that executable reproduced the physical baseline original mono SHA-256 `b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2` exactly. The baseline result, source closure and its frozen proposal/report were not rewritten.

All stated input, wrapper, executable, baseline-result and runtime identities were rechecked. Explicit guards rejected unpinned include/library/LD_PRELOAD injection variables, verified fresh output/log paths and checked current local process capacity after root released the native sequence and D180 run. No candidate engine or target firmware ran in this capture.

This is a **new original-reference capture from the baseline-validated reconstruction**. It does not recover the historical paired bytes. The old paired SHA-256 `f0f95c5ec0e496beaec004602b347bea41be069388e69c42f9f28b69c110e98a` includes the original and historical candidate columns and cannot authenticate this new mono file.

The historical **18-file / 54-note missing paired-PCM status remains unchanged**. The six depth-zero aliases and all eight standalone-filter observations remain unchanged. This additive event provides **one new active case / three new note observations**; no other active case was rendered. See [coverage_addendum.json](coverage_addendum.json).

The independent-engine comparison is outside this capture task. No candidate pass, factory-program coverage, native memory fit or real-time qualification is inferred. The existing gain-1, RMS ≤ 1 dB, spectral ≤ 3 dB and zero-nonfinite gates remain unchanged.

## Public handoff and isolation

[PUBLIC_MANIFEST.json](PUBLIC_MANIFEST.json) lists the new numeric PCM, numeric manifest, public command/receipt metadata, renderer logs and this report for publication. It includes no original algorithm, header, object, linked executable or raw original-source compiler diagnostic. The full isolated original closure remains comparison recovery evidence under reference ownership.
