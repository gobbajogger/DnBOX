# Completed independent review receipt

`report.md` and `review.json` contain the final result. The six full persona
outcomes are in `personas/`; the single fresh independent validator returned
true for all four historical findings in `validator-result.json`. All four
owner fixes were independently checked. `actionable-findings.json` is empty.

This directory covers the isolated research source and its evidence package.
Full Sound qualification, native flash admission, complete callback timing and
the owner's before/after Local gate remain separate requirements.

## Provenance

`scope_manifest.json` pins the 342 staged files reviewed against
`b90faafd2de71063448beea67aa5d22938a5c64b`, including the final documentation
addenda. The formal-review directory itself and the parent's final package
manifest are later evidence-only additions. The inherited consolidated merge
outside the explicit paths is not new candidate authorship.

`resolved_findings.json` preserves the four stable finding numbers, original
quotes, exact pre-fix hashes, verified owner fixes and validator reasons.
Historical execution paths and command arguments remain unchanged in receipts.
`ARTIFACT-ORIGINS.json` maps the source receipts into this curated directory.
`EXISTING-EVIDENCE.json` pins the existing numeric fixtures and retained results
without copying their trace CSVs or relocated build trees.

The small `historical/verify_candidate_pre_reference_guard.py` file is the exact
intermediate runner needed to reproduce finding #3. It is not an active
acceptance command. Other historical runners already exist in
`candidate_review/history/` in the surrounding research package.

## Reproduce the runner controls

Run these commands from the `experiments/sound-hardware-v1` directory. Choose a
new output path for every command; the scripts refuse existing directories.
Python 3 and the package's existing files are sufficient for the first five
commands. They exercise Python orchestration with explicit stubs or path probes;
they do not execute original engines or claim a new full audio-acceptance run.

```sh
python3 -B candidate_review/formal_review/scripts/check_compact_output_preservation.py --package . --output /tmp/dnbx-compact-collision
python3 -B candidate_review/formal_review/scripts/check_acceptance_exit_status.py --package . --output /tmp/dnbx-current-status
python3 -B candidate_review/formal_review/scripts/check_acceptance_exit_status.py --package . --source-script candidate_review/history/verify_candidate.py --false-exit 0 --output /tmp/dnbx-historical-status
python3 -B candidate_review/formal_review/scripts/check_reference_integrity.py --package . --output /tmp/dnbx-reference-integrity
python3 -B candidate_review/formal_review/scripts/check_public_boundary_path.py --package . --output /tmp/dnbx-boundary-path
```

The historical status command intentionally expects the old incorrect zero
exit after a false gate. The reference-integrity command reproduces the old
four-endpoint acceptance from retained curves, then verifies the current
5,120-endpoint input set and five rejection cases. It does not rerender DSP.
The path probe stops at library selection before loading native code.

For a fresh compact equivalence run followed by the real 130-case public API
check and short amplitude/filter live-control timelines, use the package's
existing compact runner, then the helper below. A host C compiler is required.
The compact runner creates one new labeled directory in `candidate_review`.

```sh
python3 -B candidate_review/verify_compact_profile.py --label formal_replay_01
python3 -B candidate_review/formal_review/scripts/check_boundary_and_live_controls.py --package . --review-dir candidate_review/formal_replay_01 --output /tmp/dnbx-boundary-live-controls
```

The helper writes the new `PUBLIC-BOUNDARIES.json` into that fresh compact
directory, rejects a repeat without changing it, rejects missing libraries,
and compiles/runs only the short candidate live-control tests. These checks
contain no native firmware build or timing loop. Existing original amplitude
qualification and callback receipts remain pinned in `EXISTING-EVIDENCE.json`.

## Publication contents

`MANIFEST.json` hashes every distributed file except itself. No compiled
binaries, Python bytecode caches, copied skill instructions, full relocated
workspaces, or duplicate trace fixtures are included.
