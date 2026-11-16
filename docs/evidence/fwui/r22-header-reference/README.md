# Current R22 header source/cache regression

The full host run at integration 7f6d8a44 failed because its header comparison required pixels from the retained pre-v2 8be header. That historical failure is not a reason to revert the reviewed R22 design. This test replaces only the header oracle with an authenticated d13 source/helper extraction. The original retained 8be arc and master-wave comparisons are unchanged.

The oracle has separate snapshot symbols and reference-only caching disabled. Current drawing primitives, fonts and theme inputs remain shared: this is source/cache regression evidence, not an independent visual golden or owner design acceptance.

Actual authenticated ordinary run: 17,148 checks PASS. Includes six themes, all parts, battery levels, selected drum voice, tempo, pattern/song/save transitions, pickup/clip context, real GLOBAL/list entry, held SEL help/release and ASK/cancel. Four actual test-only production mutations (battery draw, battery cache key, selected-voice cache key, tempo cache key) each fail the header pixel assertion. Four authentication negatives reject changed oracle/receipt before compile.

Run `sh tests/host/composed_ui_identity_test.sh` and `python3 tests/ui_defects/header_reference/test_authentication.py`. For isolated mutation evidence, run `python3 tests/ui_defects/header_reference/mutations.py --source . --output <new-directory>`.

Original local log hashes remain in results.json; normalized published log hashes are separate. No production change, complete inventory, full visual acceptance or native qualification is established.
