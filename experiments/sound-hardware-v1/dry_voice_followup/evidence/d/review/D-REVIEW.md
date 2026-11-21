# Independent review of frozen Variant D

## Verdict

**No implementation blocker found. D is ready for root's one authorized serial 180-note comparison using the unchanged portable metric.** This is readiness to measure, not sound PASS or production admission. The reviewer performed no compiler, test-executable, audio, metric, native, emulator, RDC, or GitHub operation.

Reviewed source: `/workspace/scratch/e803955aec60/s1_dry_voice_d`.

| Frozen identity | SHA256 |
| --- | --- |
| Declaration | `9e6c7241b36ae4ca4fc0fae2cc764536a8612517c98cec3e2d149236c33cc28a` |
| Core C source | `dab6afb5374a826bdd696f750709f22eaf658da8d9ddbf959646e95318cb43b1` |
| Host executable recorded by preparation | `1a9691cedd3836364dc3d4e96f7fed87604b00570efabf729e7e4e6e7f6e8751` |
| D-linked ownership executable | `4fa487ff457af566aaa6b454bdadf1fb3d69219354507adb59e9df2e80b6784d` |

## Exact implementation delta

`src/s1_dry_voice.c:141-147` implements the direct solve `d=w/(1.0f+5.0f*c)`. The only changed nonzero-resonance commit expression is `float nonlinear = 5.0f*d` at line 196. This matches `NONLINEAR-HYPOTHESIS.md:230-255`.

A normalized whole-file comparison, accounting only for that helper-body replacement and that one commit expression, finds no other executable change from frozen C. The complete `R==0.0f` branch at `src/s1_dry_voice.c:155-171` is **byte-identical** to C, including the Chicago helper call, signed-zero multiplication, forward stages, and lag shifts. The Chicago helper itself is unchanged and remains used by that preserved branch.

The five other renderer source files—header, host, fastmath, and envelope source/header—are byte-identical to C. All six canonical imported files under `DnBOX/experiments/sound-hardware-v1/dry_voice_followup/variants/d` match D's declared six-file freeze.

The following behavior is therefore retained: pole coefficient preparation, `R=0.8f*physical_resonance`, q/w/c evaluation order, centered weights, two held-input substeps and arithmetic mean, state/history shifts, reset, physical ranges, oscillator phase, squared velocity, component gain, pre-filter VCA, envelopes, note events, and existing release behavior. There is no new state, output gain, fitted slope, limiter, parameter clamp, or retirement rule. Host state remains 160 bytes with 92 prepared bytes.

For legal existing coefficients, `c` is nonnegative and the direct-solve denominator is at least one. The diagnosis's conservative bound remains below 1.101. No iteration or early termination remains in the nonzero-resonance solve. Removing saturation intentionally removes C's prior `|input_to_poles| <= |input|+R` bound; the source and documentation accurately limit the replacement's stability claim to the fixed-coefficient exact-real model. This review does not extend it to arbitrary modulation, all binary32 trajectories, or global original-filter linearity.

## Ownership evidence is reused correctly

The **test source** is reused unchanged from `/workspace/scratch/e803955aec60/s1_c_ownership/tests/ownership_test.c`; its SHA256 is `bf7bba35b180e3291146b7267ccd4c16ec3286126c64478ad73ee72cfa81450e`. The **test result is a fresh D execution**, not a copied C result. `logs/build-ownership-attempt01.json` records linkage against D's `build/s1_dry_voice.o`, and the execution receipt pins the resulting D ownership binary. Its stdout matches the saved result and final declaration exactly.

The source still begins with a historical “frozen C” comment. `READY.md:99-101` explicitly explains the unchanged test source and D linkage, so this does not misidentify the tested executable.

The test exercises two active owners with separate controls and states:

- Independent baselines use one-frame API calls (`tests/ownership_test.c:157-174`).
- Interleaved execution uses irregular requested groups of 1, 17, and 64 frames, clipped at each owner's own event boundaries (`48-65,176-220`).
- Every interleaved sample is compared bit-for-bit with its owner's baseline; the complete 160-byte state is checked at each block endpoint (`198-214`).
- The other active owner's voice and controls must remain unchanged (`190-216`).
- Timelines include four/five live control edits, asymmetric note-offs at 2049/3071, cutoff endpoints, resonance up to 0.98, depth 1.2, and a zero-to-nonzero resonance transition (`91-148`).

Saved results consistently record **8,192 sample-word comparisons, 310 full-state checkpoints, 310 untouched-owner checks, and 16,384 finite-output checks**. Each logical owner receives 4,096 independent and 4,096 interleaved frames. These are actual bounded dynamic ownership/partition checks. They are not reference fidelity, exhaustive dynamic stability, or an accepted dry-note count.

The host selftest remains C's configuration/gate/reset test. C's old 38 nonlinear solver fixtures were not reused to certify D's different feedback law.

## Evidence and preservation

The review independently reconciled the final declaration, pre-build/pretest/preparation chain, all declared source/binary/test-evidence hashes, and the 33-file evidence manifest. The hypothesis copy matches the named diagnosis document. The C source/declaration freeze is unchanged. All three D test receipts match their recorded command, working directory, pre-execution binary hash, exit status, stdout result, and empty stderr. The three build receipts name D's files and report successful terminal status.

`prepare.py` is a frozen historical preparation driver and contains Python assertions for its own checks. This review does not treat it as a portable fail-closed gate or rerun it. Root's already reviewed portable entry supplies the explicit checks and fresh-output behavior for the forthcoming comparison; the frozen D facts were independently checked here.

The gate stays fixed at gain 1, RMS error at most 1 dB, spectral error at most 3 dB, and zero nonfinite samples, with no fitted gain, alignment, normalization, or tail editing. The previous analytical gain predictions remain predictions until D's comparator reports its results. Native cost/fit, callback deadlines, dynamic reference cases, presets, full controls, FX, and production activation remain outside this review.

No source modification, new candidate, optional rerender, or additional test sweep is required by this review. Root remains sole canonical writer. All reviewer writes are confined to the review directory; original engine algorithms, assets, and source-aware candidates were not opened.
