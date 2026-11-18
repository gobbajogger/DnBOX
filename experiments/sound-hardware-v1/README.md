# Hardware-first Sound research package

This package belongs to the existing Sound lane. It is a tested isolated candidate and a reproducible evidence set; no runtime engine selector or production DSP is changed by adding it.

Start with [the owner report](../../docs/sound/hardware-first-owner-report.md), [candidate documentation](hardware_candidate/README.md), and [independent review](candidate_review/INDEPENDENT-REVIEW.md). The accepted behavior core is `hardware_candidate/src`; the separately measured compact proposal is `hardware_candidate/size_variants/compact_profile`. Both remain research-only.

## Run the accepted core

From this directory:

```sh
make -C hardware_candidate all
make -C hardware_candidate test
make -C hardware_candidate sanitize
python3 hardware_candidate/tests/assess_evidence.py --verify-inputs-only
python3 hardware_candidate/tests/assess_evidence.py --output-dir build/new-assessment
python3 candidate_review/verify_candidate.py --label rerun_01
python3 candidate_review/verify_compact_profile.py --label compact_rerun_01
python3 candidate_review/check_compact_public_boundaries.py --review-dir candidate_review/compact_rerun_01
```

Use a new review label; existing snapshots and results are retained rather than overwritten. The assessment helper locates the pinned stock DSP in the enclosing repository or accepts `--stock-dsp PATH`. It checks hashes before using it. The numeric observations are under `mapping/results/implementer_brief`. Original engine source and app binaries are not inputs to this candidate build.

The independent acceptance runner verifies all three reference hashes from its frozen declaration before creating an output directory or compiling. Changed, missing or truncated fixtures are rejected. A failed scoped acceptance returns a nonzero process status. Historical receipts retain the harness version that produced them; the repaired replay scripts and their negative tests are separately identified in the formal review.

Use `python3 candidate_review/run_callback_audit.py --label callback_rerun_01` for a fresh callback measurement. The callback trace harness has its own pinned frozen snapshot and probes. Its original commands and results are retained under `candidate_review/callback_audit`. Running it measures an engine/event slice, not the full firmware callback or target deadline safety.

## Read results without broadening their meaning

- `hardware_candidate/evidence`: writer tests, sanitizers, observed curve fit, source provenance, state sizes and controlled-host timing.
- `candidate_review/frozen_01`: the allocator failure, kept as a negative reference.
- `candidate_review/frozen_02` and `frozen_03`: corrected independent amplitude, lifecycle, partition and error results.
- `candidate_review/compact_profile_review`: independent boundary and bitwise-equivalence proof for the separate compact proposal.
- `resource_review`: native complete-image links, simultaneous memory ledger, adapter source, exact build inputs and remaining flash deficits when present.
- `mapping/results/implementer_brief`: measured/numeric behavior and control identities, not copied original algorithms.

The original selected component's 42-capture qualification is separate reference evidence. The owner report keeps it distinct from this candidate's smaller original-amplitude comparison. No claim of complete Synth One/SuperJ6 sound, controls, effects, native timing or hardware admission follows from these tests.


## Native reproduction

The original native commands are preserved unchanged as evidence. To recreate their relative workspace in a fresh directory, use:

```sh
python3 reproduce_native.py --toolchain /path/to/jieli-linux-toolchains-20260730.1 --sdk /path/to/AC79-SDK --output /new/scratch/directory --prepare-only
```

Remove `--prepare-only` for the baseline and replacement builds. Preparation has been executed and checked after relocation. The underlying native commands were executed in the original research workspace; a second complete native rerun through this packaging wrapper has not been claimed. Existing output directories are refused. Physical-link failures and diagnostic overflow images remain distinct. Whole disassembly logs are retained as `.log.gz`; decompress them when inspecting call frames.

Toolchain archive SHA-256: `a1545cddbc451a06bac43eb5cd96d831470c42a97c5c0b1f98879a20fc7647bd`. SDK archive SHA-256: `929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6`. They are recovery inputs, not firmware payloads.

The source manifests inside native receipts describe the original experiment, including rebuildable diagnostic objects and ELF files. They are provenance records, not a claim that every generated binary is distributed here. The package manifest describes the actual published files. Native source overlays, build scripts, linked-size results and losslessly compressed disassembly are included; executable firmware images and duplicate SDK/toolchain trees are omitted.

## Next hardware experiment

[The narrow-adapter decision](resource_review/NARROW-STOCK-ADAPTER-DECISION.md) identifies a bounded stock-envelope experiment and exact cost limits. It does not promise a fit: stock leaves only 156/164 bytes of net image growth above the required reserve. Synth One needs a separate qualified voice topology. No codec/history sweep is proposed.
