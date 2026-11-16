# Local UI defect gate

This is a fail-closed local test suite, not GitHub Actions and not a declaration
that the firmware or website is accepted. No device or network operation occurs.

The first producer slice covers actual drum control identity, glyph/value
collisions, requested content/empty-region geometry, and painted captions.
See `first_slice/` for its exact invocation and source requirements. Unsupported
screens and remaining contracts must remain `NOT_RUN` or `REVIEW_REQUIRED`.
The Web/Remote producer lives in its own repository and supplies separately
pinned browser/DOM evidence. A firmware-only result cannot qualify Web/Remote.

Run aggregator mechanics tests:

```sh
python3 tests/ui_defects/test_gate.py
```

The aggregator mechanics tests are synthetic unit fixtures. They do not count
as product render tests or acceptance evidence. They prove that missing checks,
non-PASS status, stale sources, absent/changed evidence bytes, wrong screenshot
dimensions, unaccepted human review and failed first attempts block acceptance.

Full delivery verification requires both reports, both complete source closure
hashes and connector readback receipts for the actual pushed revisions:

```sh
python3 tests/ui_defects/gate.py verify \
  --firmware-report /private/evidence/firmware/report.json \
  --firmware-source <64-hex-full-closure> \
  --firmware-pushed-receipt /private/evidence/firmware/head-readback.json \
  --web-report /private/evidence/web/report.json \
  --web-source <64-hex-full-closure> \
  --web-pushed-receipt /private/evidence/web/head-readback.json
```

Evidence paths may be relative to the report directory or absolute beneath it.
Artifact hashes, full PNG decoding and actual dimensions are checked against local bytes. At least one applicable rendered screen and an explicit screen/state coverage inventory are required. An inventory supplied by a producer is not independent proof of exhaustiveness.
A report's labels alone are not evidence. A pushed ref readback is necessary but
not sufficient: each producer must bind its exact source closure to that tree.
The aggregator cannot independently infer engine semantics, exhaustive screen
coverage, visual quality, source ancestry, real-time performance or hardware
behavior from arbitrary JSON. Those require the actual producer measurements,
source audits and independent human/native/device checks.

Keep every development RED, changed-source GREEN and final qualification attempt.
`record-run` appends an attempt ledger keyed by source, full input closure and
case. A failed first run stays failed on an unchanged-input rerun. A repaired
product or justified harness change creates a different explicit input closure;
it does not rewrite or relabel the original failure.

No required contract is silently waived. The raw requested content fraction and
empty-rectangle thresholds remain visible even when they flag deliberate space.
Universal monotonic pixel-change rules do not establish truthful oscillator,
discrete-control or coupled-parameter semantics. In particular, drum Attack is
an amount, not an audio rise-time parameter, and kick Pitch is coupled to pitch
depth. A symbolic rising-edge cue must not be presented as an actual time law.

The first slice is allowed to land while current product findings are red. DONE,
merge, installable qualification and release remain blocked by their applicable
full gates. Do not interpret test-runner success as a product PASS.

## Canonical design contract

The owner-authored [UI source of truth](../../docs/ui/UI-SOURCE-OF-TRUTH.md)
is the single canonical design document. It is byte-pinned by
`canonical-spec.json`; `gate.py verify` authenticates the actual repository file
and rejects reports missing that exact document hash. A document hash match is
identity only, not a per-page conformity PASS. Runtime/legacy-data discrepancies
and unqualified replacement engines are recorded separately in
[engine admission notes](../../docs/ui/ENGINE-ADMISSION.md), not edited out of
the owner's words. The first slice is not the complete per-page token gate.

```sh
python3 tests/ui_defects/test_canonical_spec.py
python3 tests/ui_defects/canonical_spec.py --root . \
  --descriptor tests/ui_defects/canonical-spec.json
```

The second command explicitly reports `PAGE_CONFORMANCE_NOT_RUN`. All firmware
pages, every website route and Remote desktop/portrait/landscape need their own
complete source-pinned inventories and real token/interaction/render evidence.
The current producer does not satisfy that broader coverage.
