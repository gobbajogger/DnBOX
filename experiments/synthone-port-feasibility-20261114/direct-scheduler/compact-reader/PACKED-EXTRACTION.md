# Stateless packed-word extraction experiment

The compact-v1 format, aliases, predictors, finite-word checks and all public ownership/failure contracts are unchanged. No table word, oscillator equation, precision or live state is changed.

Only two substitutions are made in compact.c:
1. Extract the at-most-32-bit field using at most five bounded byte loads, not one iteration per bit. Width0 performs no load; width32 avoids shift-by32. A fifth byte requires nonzero shift. Existing block validation proves the field lies within its exact encoded extent.
2. s1c_read retains its first validated header view instead of evaluating the same pure header twice. Binding extent/alignment/tag, header, output extent/alignment and whole-binding/payload disjointness checks remain, in their original order. Borrowed bytes/binding remain immutable and lifetime-owned externally. No concurrency or forged-object admission is added. Public s1c_binding_valid is unchanged.

Added host tests cover all33 widths, four predictor modes, all64 field positions, full and one-word terminal blocks, independently packed uint32 codes, read-only canonical payloads ending at guard pages, invalid binding/tag/header/span/coordinate/output preservation, all213044 canonical words,14 negative zeros and52 guards. Fixtures are optional and must match the exact authenticated hashes in the runner; neither raw nor compact fixtures belong in public Git.

Run: python compact-reader/run_word_checks.py --source-pin MANIFEST_SHA256 --out NEW_PRIVATE_DIRECTORY [--compact EXACT_COMPACT --raw EXACT_RAW]
Complete source closure and optional fixture pins are validated before output creation or compiler execution. Default LSan-under-ptrace failures remain failures; separately recorded same-binary detect_leaks=0 pass is not leak proof.

Producer-only private qualification also covers38 original seq/arp cases at O2/Os/sanitizer plus17 full-common-FX scenarios with band-limit off AND on (34 source configurations). Three rotated paired orders per state compare12049200 stereo frames. This is not all original presets, five-minute hardest-product qualification, independent original-FX fidelity, targetCPU or device proof. Actual enabled polyphonic row histograms read rows4..35; disabled reads rows0..3. Do not generalize the cheaper unbandlimited workload to all controls.

Genuine retained three-API pi32v2 diagnostic: baseline2754B executable; candidate2796B (+42B), zero data/BSS, no undefined symbols, binding12B/align4. This is a reader-only executable, NOT full target fit. Host active band-limited scenes improve substantially, but native cycle cost is unmeasured. Full original binary64 policy failure and full-FX history/resource barriers remain open. No product integration, memory borrowing or feature reduction is authorized here.

Frozen for independent affected review; own measurements are not independent acceptance. The raw oracle, numerical source, all other56 predecessor files and header contracts are byte-identical.
