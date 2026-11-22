# Portable Sound followup package: independent bounded review

Review base: `9be11840b11d256f22a9af19d290cfe96f31a429`, plus the explicitly requested new/untracked research package. Root is the sole canonical writer. This reviewer made no canonical edits, opened no original engine implementation, invoked no compiler/native toolchain/emulator/full suite/RDC/GitHub, and spawned no agents.

**Final verdict: PASS for bounded research artifact preservation. All three findings below were corrected and their original failure cases now reject. No actionable finding remains in the reviewed source.** This verdict does not upgrade the production or lane admission scope.

## Admission scope

This is a review of portable research evidence preservation. It cannot admit a production renderer, full Sound lane, whole-image fit, native callback timing, or merge into consolidated. The C envelope's existing independent final review and the shared fastmath full-link review are separate evidence; this reviewer does not repeat or upgrade them.

## Findings corrected in the portable entry points

### P2: Optimized Python bypasses the bounds harness and fabricates a successful run

In the initial `verify_bounds.py`, critical checks and required calls are inside `assert` expressions: initialization at line 79, boundary rejection at line 146, state-changing calls at lines 172–197, rendering at line 203, equivalence checks at lines 207–209, and negative rejection at line 225. Python `-O` or `PYTHONOPTIMIZE` removes all these expressions. Result fields at lines 250 and 253 remain unconditional `True`.

The compiler-free isolated reproduction uses an intentionally broken library double and does not execute C. Normal bytecode rejects it. Optimized bytecode returns success despite **564 boundary rows with `pass=false`, zero init calls and zero render calls**, while reporting both success booleans and **576 calls / 31,488 frames**. This is an actual failure of the new evidence harness under a standard Python execution mode, not a claim that the owner's normal host run used optimization or was invalid.

Required correction: use explicit runtime requirements that evaluate their call arguments, or refuse optimized Python before creating output. Keep the recovered historical runner/results unchanged.

Evidence: `NEGATIVE-RECEIPT.json`, `reproduce_harness_failures.py`, exact initial `verify_bounds.before.py` (SHA256 `d2fb26a739f3794971f92697327ed0372d05535f2f5da59d3961002989ea29bd`).

### P2: Unpinned case metadata can hide six live cases while preserving the claimed sample count

Initial `coefficient_followup/check.py` lines 32–45 load static/live JSON without an identity check, although those JSON fields choose the stimulus, byte offsets and frame counts. The scoped gate at line 67 checks the aggregate 379,288 count. Repeating a valid case therefore satisfies the count while skipping other packet words. The same pattern exists in the new component test's `fixtures()` at lines 37–54.

The isolated valid-JSON mutation keeps all ten case IDs and both pinned binary packets unchanged. It redirects the six other live offsets and stimuli to the first unchanged case. The unchanged Python proposal then produces **379,288 exact finite comparisons** and still rejects the +1ULP attack control, while **98,304 live reference words are never compared**. The old component `fixtures()` also accepts it and selects only one distinct live stimulus/reference. This proves the metadata defines test coverage and belongs inside the input identity boundary.

Required correction: pin static observations JSON, live observations JSON and the associated live case plan before replay (and before compilation in the portable checker). Make the checker length check explicit rather than an optimization-sensitive assertion.

Evidence: `NEGATIVE-RECEIPT.json`, `COMPONENT-METADATA-NEGATIVE.json`, `duplicated_live_metadata.json`, `reproduce_component_metadata.py`. Initial checker SHA256 `d964cbc105f534b2a49dee0a6f43cb814651ebe5cbcd400d88aebfe7869b425d`; initial component test SHA256 `0f25a47aa44fabed04327489ffc37bd4f582725d343c1b0ae7a4a88a4d62e7a0`.

### P2: Optimized retained-red checking accepts an empty PCM input with exit status zero

The initial new component test's `retained_red()` puts its input length check in `assert` at line 206. Under optimized Python, an empty file reaches `difference()`, which correctly returns `same_length=false` without `different_words`. The final `row.get('different_words', 0)` at line 215 treats every malformed comparison as zero differences and returns zero.

The isolated helper-only reproduction confirms normal execution raises `AssertionError`, while optimized bytecode returns exit status zero for the same empty PCM. No C setup or execution occurs.

Required correction: reject the wrong input length explicitly and ensure malformed comparisons cannot return success. This change belongs in the new portable test, not the retained original output.

Evidence: `COMPONENT-METADATA-NEGATIVE.json`, `empty_retained_pcm.f32le` and `reproduce_component_metadata.py`.

## Positive evidence within this scope

- Both portable scripts reject an existing output directory and a symlink to that directory with nonzero exit. The sentinel and directory contents remain unchanged. These early failures occur before any compiler or library call (`OUTPUT-SAFETY-RECEIPT.json`). Output filenames and input locations are fixed relative to the script, and compiler arguments are lists; no shell interpolation or user-controlled output filename traversal was found.
- All three preserved bounds C/header inputs match the isolated source hashes. `RECOVERED-RUNNER.py`, `RECOVERED-RESULT.json`, and the saved bounds patch match their original files exactly. The new runner pins all six reference/candidate inputs before creating output or compiling.
- The fixed coefficient implementation, coefficients wrapper, sweep/stress sources, both reports, recovered result, CSVs and fixed-object/resource receipts match the isolated originals. `candidate.h` and `probe.c` have the expected include-path portability change; the numerical bodies remain unchanged.
- Retained CSV totals corroborate the README/report counts: old three-method sweep 9,600,096 comparisons with 250,955 mismatches retained; fixed integer portion 3,200,032 comparisons with zero mismatches; stress 2,160,000 comparisons with zero mismatches; total fixed comparisons 5,360,032. The recovered packet result has all five methods at 379,288 samples / zero differing words and its negative has 8,332 / 16,384 differing words.
- The README distinguishes original 44,100-Hz observations from broader mathematical-hypothesis sweeps, finite approximation from universal correct rounding, the 792-byte host object from target fit, and the historical 79-byte bounds object saving from the separate linked-image saving. No new whole-instrument/native timing/production admission claim was found.

## Final targeted recheck

The root changed only the new portable entry points and their integrity tests; retained original source/results remain intact. The reviewer reran the same isolated negatives against the actual changed files, using `compile(..., optimize=0)` and `compile(..., optimize=1)` with no compiler or native-library execution:

- Bounds: the intentionally broken library is rejected in both modes at the first invalid boundary, after six initialization calls. No success `RESULT.json` is emitted. The portable source has zero remaining `assert` statements; explicit `require` calls retain their C-call argument evaluation.
- Coefficients: each of the three metadata inputs is independently changed, including the original duplicate-live-offset mutation. All six normal/optimized cases raise the pinned-metadata error before any compiler call and emit no success result.
- Component fixtures: both modes reject the same duplicated live metadata. The unchanged inputs still load ten cases / 379,288 samples. Both new `FixtureIntegrityTest` tests pass in each mode; `EnvelopeTest.setUpClass` is never invoked by this review.
- Retained-red: both modes reject the empty PCM with an explicit length error. A separate actual `python -B -O ... --retained-baseline empty.f32le` invocation exits **1**, confirming the CLI failure status.

Final source identities:

| Source | SHA256 |
| --- | --- |
| Portable bounds verifier | `336f85ca8d9a6bf39b3d81095432b37759fafef1670937d0e56f3d848a2b19bc` |
| Portable coefficient checker | `8122646a5a83262775438508f23f22c4d85c19d1f2520202d1266556c7b2e412` |
| New component test and fixture guards | `7a066f0cd56015700422b7543c1c3532fa59bd2b7c18169680cae41c95bddf03` |

The recheck is retained in `FINAL-RECHECK.json`, `recheck_fixed_harnesses.py`, final source copies and two integrity logs. Original failure receipts and before-source copies remain unchanged. The root separately reported actual guarded optimized-Python host passes for the envelope/fastmath/UBSan slice, bounds and five-method coefficient replay; those are owner-run evidence, not tests independently rerun by this reviewer.

## Coverage limits

Correctness and testing lenses produced the three findings above. Reuse and quality review checked the preserved sources/evidence against their originals and found only the documented portability edits. Efficiency review found no blocking new concern in these bounded host harnesses; scalar sweeps and fixed arithmetic were preserved rather than reimplemented.

The review admits the corrected portable package for source-only research preservation. The owner's reported actual host passes are retained as supplied; this reviewer intentionally did not rerun compilation or full numeric/native qualification. Before packaging, the owner was notified to refresh `PORTABLE-RESULT.json` and `HOST-RESULT.json` from the final guarded runs so their source identities accompany the corrected scripts; historical `RECOVERED-*` evidence must remain unchanged.
