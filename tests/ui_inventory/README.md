# Firmware renderer inventory

This source audit complements the existing screen walker. It discovers painting
functions and their callers across firmware application/graphics sources, assigns stable
`repository-path::C-symbol` IDs, and leaves every unobserved production function
explicitly uncovered. Inactive preprocessor branches remain inventoried. IDs do
not depend on line numbers, discovery order, screenshot count, or a candidate's
self-reported completion count.

```sh
python3 -m unittest discover -s tests/ui_inventory -p 'test_*.py' -v
python3 tests/ui_inventory/freeze.py --repo /local/repo --tree <exact-tree-or-commit-sha> --output /new/source-freeze
python3 tests/ui_inventory/source_inventory.py --source . --output /new/source-inventory.json
python3 tests/ui_inventory/run.py --source /new/source-freeze --output /new/output-outside-source
python3 tests/ui_inventory/verify.py --evidence /new/output-outside-source --source-freeze /new/source-freeze --output /new/output-outside-source/verification.json
python3 tests/ui_inventory/extend_edits.py --evidence /new/output-outside-source
python3 tests/ui_inventory/pack_tar.py --source /new/output-outside-source --report-name combined-report.json --output /new/transfer-with-edits
python3 tests/ui_inventory/pack_tar.py --source /new/output-outside-source --output /new/transfer
```

`run.py` instruments a disposable source copy, reuses the actual geometry walker
and its actual SMC regression fixtures, and records the production renderer
functions invoked during the final `ui_frame()` preceding each snapshot. Four
battery levels, HOME live, SMC fill/pickup, waiting/readout and live ADSR routes get additional real frames. Every
snapshot receives its own PNG, including repeated names in the original SMC
fixture. `renders/` contains losslessly compressed copies, checked against the
native decoded RGB bytes; both native PNG and RGB hashes are retained.

The source inventory separately lists functions mentioning battery, playhead,
and pickup semantics. Four selected conditional branches (active ADSR playhead,
SMC peer pickup, SMC peek pickup and physical header pickup) have independent
invocation flags. Remaining semantic branch contracts stay `NOT_RUN`: reaching a
function does not prove its conditional pixels painted. The ordinary frame
names continue to use the existing firmware enum/page/voice identities; they
are state IDs, distinct from the source renderer IDs.

This is a conservative lexical call-closure audit, not a compiler AST proof.
Macro-generated painters, indirect function pointers, graphics primitives in
another subsystem, and semantically distinct branches need further audit.
Calls with the same symbol in different application files conservatively keep
all matching definitions. Unused functions therefore remain visible as gaps.
Every direct painting API invocation outside recognized function grammar is an
explicit source-audit gap rather than silently omitted. Source registration
follows quoted includes from the actual production `firmware/src/x0x.c` root.

The existing host substitutes the physical clip-grid and pad-calibration
painters. This runner separately extracts their exact production bodies and
reuses the established native painter fixtures with explicit RAM states. It
also extracts the real CK state, pure helpers, page renderer and panel renderer
into a test namespace and captures all 17 selected-field compositions as full
240x240 framebuffers. Device input, calibration persistence, MIDI routing and
hardware behavior remain outside that evidence. The original substitutes never
receive production renderer IDs. An inactive slider-lane source file stays
uncovered until registered by the production translation unit. A successful
host process cannot turn uncovered routes into acceptance.
The report measures source coverage separately, retains `F1: UNREVIEWED`,
`R22VisualReview: UNREVIEWED`, `visualApproval: false`, and unimplemented
pixel conformance checks as `NOT_RUN` and exhaustive branch coverage as partial. Independent visual review, complete
R22/shared-glyph conformance, native firmware memory fit and the Local gate must
qualify the final exact revision separately.

Source hashes and code instrumentation offsets are diagnostic provenance;
offsets never form part of a stable ID. `source-closure.json` records the input
files under firmware, host, tests, tools and experiments (including the synth
inputs used by the host build). The build's generated files, local environment
and compiler are not represented as full qualification closure by this report.

`pack_tar.py` produces directly extractable family archives capped at 800,000
bytes. Each frame has a stable state/instance PNG path, uses PNG compression 0
to expose shared backgrounds to the outer xz compressor, and passes a decoded
RGB identity check. Identical frame bytes within one archive use ordinary tar
hardlinks. `index.json` hashes every part; `frame-map.json` in the metadata part
retains the exact original/rendered/compressed/pixel hashes and renderer mapping.

Each frame carries an explicit evidence kind: complete actual UI frame,
complete one-shot boot/OTA display function, native painter composition with
fixture RAM, direct preview composition, or literal source display prefix/
statements with native input and hardware tails unrun. Literal display slices
do not mark their whole production wrapper as observed. The producer returns
nonzero for a renderer, input authentication, parsing or evidence-integrity
failure. A valid measurement can report PASS while source coverage is PARTIAL;
the explicit unobserved-ID list is retained for external lane qualification.
A renderer exit of zero and a scoped `verify.py` PASS cannot certify F1/R22
product acceptance.

`extend_edits.py` runs the existing physical lane-edit/persistence fixture in
its own isolated copy of the prepared source, preserving the primary attempt.
It adds all declared part/lane edit states to `combined-report.json` and retains
the separate invocation, exact fixture hash, instrumented inputs and native PNGs.
Host project writer/reader persistence is tested; no device persistence is used.
