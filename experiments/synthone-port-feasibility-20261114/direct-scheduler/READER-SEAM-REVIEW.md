# Independent morph-read boundary review

## Verdict

**ACCEPTED for the exact frozen source/host/native-object slice. No actionable findings.** This is not target executable, storage placement, firmware fit, stack, CPU, whole-instrument, device, publication or production acceptance.

Reviewed base: #942, `1c4c0fff8142226e9daee90db94b362cac40ded3`.

- Frozen candidate: `bd3df55b46cf7f614e8a40a97b49d7ba776ef7d94f0dacac85559e84e5f08e88`.
- Source manifest: `f384f98bb6af1d70fcd6cdd77268f776227c919918ca5f592e673f4db5eee64a`.
- Public result: `3272e13067bfc85c39714fc8a9e9946aa7057c5fba604451d514d3c60f32e6b5`.

All 336 frozen file lengths/hashes and all 32 source-manifest lengths/SHA-256/Git-blob identities were independently verified before review and the frozen bytes were verified again afterward. The 20-entry baseline source manifest was independently checked. The candidate preserves the original oracle sources, generators/manifests, both legacy fixtures, original DSP/voice dependency closure and common declarations. There were no candidate edits or Git actions; this reviewer wrote only `review/` and did not access denied paths or upload SDK/toolchain material.

## Source and contract examination

- Admission checks fixed count/topology, full final-owner overlap, cross-span exclusion, pointer arithmetic extents/alignment, every unique finite/range word and positive-zero guard. The row map is fixed; no caller-controlled map, callback or mutable cache is admitted. Invalid initialization returns before retiring the live object. RuntimeState owns its small metadata copy and borrows the immutable bytes; readability, lifetime, immutability and nonconcurrent use remain caller obligations, not arbitrary-pointer validation.
- Raw reinitialization retires reader mode; reset/restore retain it. The final Scheduler span and actual packed input extent are protected against output aliases. Independent live owners share immutable payload without sharing mutable reader state.
- Independent body comparison found the complete original voice body verbatim after reversing exactly its two morph-call names. Its helper bodies are exact, its sixteen bindings reference the actual note fields, and no alternate class definition or base-to-derived reinterpretation is introduced. Independently reversing the C derivative's reviewed guards, row bookkeeping, four checked reads and four bit copies produces the entire exact original C function, including operand order, interpolation and phase arithmetic.
- Symbol inspection shows original voice still calls `sp_oscmorph2d_compute`, oracle still calls original `S1NoteState::run`, and the runtime has distinct `s1_runtime_morph_compute` and `s1_morph_read_word` references. No unresolved private helper symbols exist. Compiling the three changed runtime units at O0, linking them with the exact independently built closure and running the reader fixture also passes, so optimized-away helper definitions are not required.
- The C format tag is structural metadata, not authentication of canonical corpus content. Corpus identity remains a separately pinned build/test input, accurately disclosed in the candidate.
- Unexpected read failure is a failed process result, not successful zero/old-sample fallback. An already advanced voice/control state or partially written block is not rolled back. The caller must treat false as failure; no transactional render guarantee is accepted.

## Fresh independent execution

Each host run staged and regenerated its exact input closure into `review/`, rather than reusing producer executables.

- Reader O2 and Os: PASS each, 10 cases, 9,722,664 checks, 1,701 positive PCM frames, 2,580 snapshots, 54 events, 639,132 exhaustive reader-word comparisons, 722,244 reader calls, zero oracle reader calls, 7,371 oscillator cases, 7 protected-write probes and zero heap calls during actions. Actual scheduled path records 10,308 morph calls, 2,304 row48 selections, 4,608 row49 selections and 189 releasing-alias calls. Three wrong-data controls independently produce finite/audible scheduled differences over 1,536 frames.
- Legacy original O2 and Os: PASS each, all 54 cases, 55,654,337 checks, 18,162 frames, 19,302 snapshots and 648 events. Legacy alias O2: PASS, 7 cases with its exact original metrics. The producer's alias Os receipt was hash-verified but was not rerun by this reviewer.
- Standalone direct reader O2 and Os: PASS each, 426,343 checks, every 213,044 logical word, 126 malformed/alias/range negative cases.
- Standalone morph component O2 and Os: PASS each, 9,477 cases, 199,249 checks, 37,908 observed actual reader calls. Runtime raw descriptor payload pointers are NULL; bitwise original-C output/state comparison still passes.
- All 10 independently rebuilt mutation cases compile and link, then fail the attributable runtime assertions. All five scheduled-path/audio mutations have a finite, audible original-PCM witness. Admission/lifetime mutations remain separately classified; compile failures are not detections.
- Reviewer-only read-fault injection, changing only the test observer: PASS. At all three rates, each of the eight morph read positions makes Scheduler return false (24 cases); each of four primitive positions preserves its output on failure (12 cases); raw reinitialization bypasses reader injection (3 cases). No rollback claim was tested or inferred. The first reviewer harness attempt failed compilation because of a test-main macro collision; its diagnostic is preserved as a harness setup failure and is not a candidate failure or mutation detection.
- Native input/strict numeric environment checks: PASS, three invalid-input early rejections plus two genuine 20-runtime-object profile builds. All 20 objects' executable sections match between clean and poisoned `CCC_OVERRIDE_OPTIONS` parent environments. Compiler/header identities, compiled local sources and resulting object hashes were verified. Both profiles report Scheduler/RuntimeState 9,972 bytes, direct binding 12 bytes/alignment 4. A separate layout object is not counted as a runtime object.

### Sanitizer qualification

**Default LeakSanitizer: FAIL for all three independently built reader/unit/component binaries**, each exit 1 with the actual `LeakSanitizer does not work under ptrace` diagnostic preserved. The exact same three binaries pass with `ASAN_OPTIONS=detect_leaks=0`, retaining AddressSanitizer, UndefinedBehaviorSanitizer and float-cast-overflow; stderr is empty. This is explicitly **not a leak-check pass**.

## Coverage boundaries and next owner

The 819,400-byte raw morph payload and separate sine/tuning paths remain required. No codec/container/parser, metadata-only Scheduler admission, asset placement, native executable/firmware link, actual combined memory/stack/CPU workload, target numerical/acoustic fidelity, whole-engine/global-FX sound or device operation was run or accepted. The next source-preservation/integration decision belongs to the existing accountable owner under its separate gates.

No API-visible effective model/effort selector or inspection metadata was exposed to this review worker. No model/effort setting change or independently attested selector value is claimed.

## Evidence pointers

`INTAKE-VERIFIED.json`, `HOST-RUNS.json`, per-run `COMPILED-SOURCES.json` and commands/results, `units/RESULT.json`, `mutations/reader-mutations.*/RESULT.json`, `faults/stdout`, `provenance/RESULT.json`, and `native-inputs/native-inputs.*/RESULT.json`. The receipt in `INDEPENDENT-REVIEW.json` binds this report to the frozen candidate and independent evidence.
