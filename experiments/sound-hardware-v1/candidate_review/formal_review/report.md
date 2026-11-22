## Code Review Results

**Formal review is complete. All four P2 findings were fixed by the Sound owner and independently verified. No unresolved findings remain in the reviewed research package.** Full Sound qualification and production hardware admission remain open.

**Scope:** the staged research package on `lane/sound`, against base and dispatch HEAD `b90faafd2de71063448beea67aa5d22938a5c64b`. The final scope manifest contains **342 files**, all matching their staged bytes when captured. It covers `experiments/sound-hardware-v1/**`, `docs/lanes/sound.md`, `docs/sound/hardware-first-owner-report.md`, and the added recovery rows in `docs/LIBRARY-INDEX.md`. Inherited consolidated changes outside these paths are excluded from new candidate authorship. The formal-review files and the owner's final packaging manifest are later evidence-only additions.

**Intent:** verify the isolated candidate, compact profile, tests, replay tools, and stated evidence limits before preserving the research source in the existing Sound lane.

**Mode:** markdown report-only. The review agent made no project, Git, production, or candidate DSP edits. The Sound owner applied the runner fixes.

**Reviewers:** correctness for DSP/state behavior; testing for acceptance and failure detection; maintainability for replay and packaging; performance for bounded work and timing claims; project-standards for the applicable repository rules; adversarial for counterexamples. All six produced valid terminal artifacts. A fresh independent validator then examined all four findings in one batch.

**Curated deliverables:** [report.md](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/report.md), [review.json](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/review.json), [MANIFEST.json](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/MANIFEST.json), and [reproduction instructions](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/README.md). The publication directory contains the persona outcomes, validator result, fix receipts, portable scripts, and negative-control results. It omits compiled binaries, caches, copied skill instructions, relocated build trees, and duplicate trace fixtures.

### P2 -- Moderate

The locations below identify the pinned pre-fix revisions. `resolved_findings.json` retains their exact source hashes, original evidence, current fix hashes, and validator reasons.

| # | Reviewed location | Issue and verified resolution | Reviewer | Confidence |
| --- | --- | --- | --- | ---: |
| 1 | `candidate_review/check_compact_public_boundaries.py:7` | Hard-coded retained libraries could not consume a fresh compact run. Required `--review-dir` selection now uses fresh outputs and refuses an existing receipt. | maintainability | 100 |
| 2 | `candidate_review/verify_candidate.py:263` | A false acceptance result still exited successfully. The current runner preserves its receipt and exits 1 for failure. | testing | 100 |
| 3 | `candidate_review/verify_candidate.py:93` | Truncated references could pass under the unchanged declaration. Preflight now verifies all three pinned reference hashes before reserving output or evaluating tests. | adversarial | 100 |
| 4 | `candidate_review/verify_compact_profile.py:144` | An existing-output rerun overwrote the shared declaration before failing. The runner now reserves a new label first and stores its declaration inside that run. | correctness | 100 |

- **#1:** The old path probe selects the retained directory despite an explicit fresh-directory argument. The fixed checker passes all **130 public API cases** from fresh libraries, rejects repeat or missing-library runs, and preserves prior files.
- **#2:** The exact old runner reports a false gate and exits **0**. The final runner exits **1** for a false gate and **0** for a true gate, retaining both receipts. These focused tests stub compiler and DSP work and exercise the actual aggregation/output/exit logic.
- **#3:** The old runner can accept **4 endpoints instead of 5,120** when only the first CSV row of each case remains. The unchanged final references pass and record their verified hashes. Truncated CSV, one-byte JSON and brief changes, a missing CSV, and an incomplete declaration all exit **2 before output reservation or compiler execution**. These controls use retained candidate curves and stub nonreference leaves; they are not a new full audio-acceptance run.
- **#4:** The historical collision reproducer confirms that the declaration changes before `FileExistsError`. The fixed collision guard preserves retained files. A fresh compact run passes and records the digest of its own declaration.

All four findings received `validated: true` from the [fresh validator](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/validator-result.json). Their owner fixes are closed in the final review; [resolved_findings.json](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/resolved_findings.json) preserves the historical findings and verification evidence.

### Actionable Findings

**None in the reviewed source snapshot.** `actionable-findings.json` is empty. No new DSP defect remained after the earlier allocator correction, and no DSP source changed during the formal runner fixes.

### Coverage

**Independent numerical checks.** The compact profile matches the accepted core for **441 layer-parameter boundary cases**, plus **130 additional public API cases**. Stereo output and complete part state are bit-identical over **262,144 frames across two parts and four sample rates**; enabled render counters also match. The counters-off contract and invalid build-profile rejection were checked. The short live-control timeline passes for both amplitude and filter envelopes: a running attack/release keeps its cached contour, the next decay uses updated settings, and a sustained envelope adopts the new sustain level. See the [compact equivalence receipt](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/compact_profile_review/RESULT.json) and [public API/live-control receipt](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/receipts/boundary-and-live-controls.json).

The prior original-amplitude comparison remains limited to the two supplied 48 kHz recordings used during implementation fitting. At the predeclared maximum absolute error threshold of **0.001**, the two cases pass with maximum errors **6.9401957e-7** and **7.9466463e-7** over **5,120 exact timestamped endpoints**. Wrong time-grid, linear-envelope, and squared-velocity controls are rejected. These results do not establish held-out original behavior, the original 44.1 kHz grid, or complete instrument equivalence. The [original-amplitude receipt](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/frozen_03/RESULT.json) retains the metrics and negative controls.

**Replay and packaging.** A private copy of the indexed package builds and runs its candidate tests and assessment. Stock inputs resolve after relocation and match their pinned hashes. Callback and zero-stage probe dependencies compile; the short traces reproduce **168 exp2 / 112 log2** calls for the selected burst and **32 / 24** zero-stage entries in the two event cases. No native build or long timing loop was repeated for this follow-up. The portable reproduction scripts were executed after their path changes. The [relocation receipt](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/receipts/relocation-and-equivalence.json) records the commands and results.

**Workflow receipt.** The actual `ce-code-review` workflow completed scope/criteria discovery, six persona reviews, source-detail hydration, the `findings-mechanics.py` merge, and one fresh validator batch. There were **zero malformed returns, suppressed findings, quote-gate demotions, validator drops, or degraded blockers**. No different-provider reviewer CLI was available within the session's permissions; no external peer job started, and the local adversarial fallback completed. No cross-model validation shortcut was taken. A capacity deferral occurred before the successful validator launch. No applicable explicit or unambiguous implementation plan was found, so plan-settlement suppression was not evaluated.

**Documentation checks.** The owner report, lane status, S1 addendum, narrow-adapter decision, package instructions, and recovery-index additions preserve the distinction between observed evidence and production admission. The narrow-adapter arithmetic is consistent: the compact replacement adds **1,928 bytes**, while stock permits only **156 / 164 net bytes** of image growth above the reserve. The raw owner already occupies **900 bytes once per part**. No fit or timing gain is inferred from reorganizing it. These figures agree with the [narrow-adapter decision](sandbox:/workspace/scratch/67e685478953/superj6_research/sound_lane/experiments/sound-hardware-v1/resource_review/NARROW-STOCK-ADAPTER-DECISION.md). Recovery rows retain the owner's verified archive metadata; this review did not repeat downloads. New authored source/test/prose passes the scoped whitespace check; captured CSV/native output formatting remains unchanged.

The final [callback admission contract](sandbox:/workspace/scratch/67e685478953/superj6_research/sound_lane/experiments/sound-hardware-v1/resource_review/CALLBACK-ADMISSION-CONTRACT.md) is also included in the scope. It keeps the conditional **256 command-slot / 64 MIDI-packet / 31 physical-key-entry** ceilings separate from internal event expansion. It correctly distinguishes the existing 12-bit command payload from the probe's 436 raw binary32 stores and identifies the 24 ms fault timeout as incompatible with inferring an 11.61 ms worst-case deadline guarantee. This was a documentation claim check of the resource worker's source audit, without a new timing measurement.

**Remaining qualification.** The compact image still exceeds physical flash by **748 / 740 bytes**, and misses the required reserve by **1,772 / 1,764 bytes**. The host benchmark measures the selected two-part engine plus its fixed event/control burst. The caller has no event-delivery cap, and the benchmark excludes the complete firmware's other instruments, effects, mixing/USB, HAL, and interrupt interference. Complete native callback time, xruns, and stack high-water remain unmeasured. Full S1/SuperJ6 control, filter, oscillator, FX, preset, lifecycle, and listening qualification remains open. The owner's exact-head before/after Local gate is separate and receives no PASS from this review. The [native resource review](sandbox:/workspace/scratch/67e685478953/superj6_research/sound_lane/experiments/sound-hardware-v1/resource_review/CANDIDATE-RESOURCE-REVIEW.md) and [callback audit](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/callback_audit/RESULT.json) retain those limits.

### Exact reviewed inputs

| Input | SHA-256 |
| --- | --- |
| Accepted `dnbx_core.c` | `9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa` |
| Accepted `dnbx_core.h` | `a863f6b04f65202a3ba7f5640b4dc89e0730a15c096894bf9d914caaf7c8671c` |
| Permitted `fastmath.h` | `fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09` |
| Compact `dnbx_core.c` | `3d8f898428b297478abb35692de56952451caed38cc02db53e16a19a54a7408b` |
| Compact `dnbx_core.h` | `86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962` |
| Final `verify_candidate.py` | `6acb326fa4df8142690cca89d55d1a66e61cd573e6499e75918fb0c2f28cc774` |
| Reviewed staged scope manifest | `ef28caa0f91078fe466524f0cc5b52d5608b344a33ed813d3f50729bed0aeadd` |

The [scope manifest](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/scope_manifest.json) records every reviewed file's SHA-256 and Git blob ID. Existing numeric fixtures and retained behavior/timing receipts are pinned separately in [EXISTING-EVIDENCE.json](sandbox:/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review/EXISTING-EVIDENCE.json).

---

### Verdict

> **Ready to merge -- reviewed research package only.** All four P2 findings are resolved and independently verified. This clears the research-source review; the owner retains responsibility for branch/gate decisions. Full Sound, production firmware activation, and hardware admission remain unapproved by this receipt.

**Actionable findings: none.** The complete curated publication directory is `/workspace/scratch/67e685478953/superj6_research/candidate_review/formal_review`.
