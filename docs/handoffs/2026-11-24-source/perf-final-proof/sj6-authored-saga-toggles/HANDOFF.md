# SuperJ6 authored Saga toggle checkpoint (host only)

Exact standalone base: `perf-final-proof/sj6-saga-arp/checkout`, touched paths
pinned in PREIMAGE.sha256. Production and test-only patches are separate. Both
apply and reverse-apply clean on their exact standalone pre/post sources.

Admitted fields: Build-9 physical toggle IDs 163–178 only. The pinned
`4ea86b9…/experiments/superj6-original-reference-20261115/timing/saga_core.hpp`
selects a step on only for the live value 1; the preset defaults in
`5d18b18…/experiments/superj6-conductor-source-20261115/control_contract.json`
include step-toggle 1 exactly. The authenticated archive contains 493
complete binary 16-step grids, 147 rows omitting the whole grid, 7,888
literal toggle values, 377 rests, no duplicates or ambiguous values.
Reader also admits signed zero as OFF and preserves first-occurrence
precedence. A first nonbinary authored toggle rejects the grouped load.

The successful pre-note factory boundary updates only authored bits for
an owner with no valid saved block and no queued project/pad edit, before
the existing AMP2 persistence writes a v1 block. The existing 32-byte
shared owner payload contains observed toggle state; no format changes.
The first adopted grid becomes saved owner state (subsequent program or
engine changes keep it by the existing shared owner rule). Absent fields,
project mode, edit state, gate, length, offsets, octave, and both normal
and external tempo stay unchanged. No inferred SEQ enable or fallback
for an unknown destination. The custom typed source transaction is
tested, not a claim that firmware's stock four-program selector already
loads every original row. No native build, fit, or timing claim.

Red is the exact standalone source with only the test patch added; it
fails the new mask assertion. Green verifies both real engine owners,
deferred commit, exact mask with high step/present-off/omitted semantics,
saved file round-trip, preset-change persistence, previous saved project
priority, queued edit priority, invalid first-word refusal, peer
isolation, and all 640 original masks through the factory boundary.
Decoded adopted masks replay both short-frame event traces at 120 BPM:
on 0, off 1034, muted boundary 2068, note at 4135; off-grid transport
entry waits and clips below MIDI 0. Existing Saga test also passes
2,176 assertions, unchanged SMC pad/grid gates, speed and S1 transition.

Replay: apply both patches; run `tests/host/sj6-smc-ui.sh` with
`X0X_SUPERJ6_FACTORY=1 X0X_S1C=1`, host defs `-DX0X_SUPERJ6_FACTORY=1
-DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1
-DX0X_S1C=1`, and
`SMC_UI_TEST_FILE=tests/host/ui_superj6_authored_saga_test.c`.
Use exact generated build inputs from the Saga donor. If the donor
build wrapper needs imports, put exact-ble-cold-fw/tools on PYTHONPATH.
For corpus, run generate-toggle-fixture.py with the locally authorized
archive and a temporary saga_toggles.inc, add `-DSJ6_SAGA_CORPUS` and that
temporary include path. No archive, bank, or generated masks are here.

## Shared candidate read-only check

See full-control-apply-check.log. Reader, ARP slot/helper and test patches
apply clean. Shared engine.c has two textual boundary conflicts, with no
behavioral reason to reintroduce the old LFO-only helper. After authored
Vibrato is reviewed/promoted, reconcile only these logical engine hunks:

1. The guarded per-owner `seen,on` mailbox and helper declaration beside
   `sj6_factory_lfo_pending`; retain the live unified factory helper
   declaration, not the old `sj6_commit_factory_lfo` prototype.
2. Validate `row->saga_invalid` under the existing built-in/init + ARP
   guards, before acquiring factory state, and copy both words when the
   same locked grouped stage is accepted.
3. After the latest unified factory helper successfully reaches the
   pre-note slot (and cancellation guards), call the guarded toggle
   helper before AMP2 persistence. Keep the new helper and its current
   handling of chorus/vibrato/LFO, never restore the obsolete call.
4. Re-run the focused original corpus and untouched sibling regressions
   on the exact promoted tree. No linked AMP1 dependency in this path.

Remaining original evidence request: prove the Build-9 setter transition
from raw saved 147–162 step offsets and 145 count to integer live fields,
with edge/neighbor values and import order, and the exact saved 144 gate
mapping including literal zero and fractional values. The existing
provisional local mapper rounds these offsets and quantizes gate
(1–100%), and is not evidence for original saved-word behavior. Destination
and start/sync listeners remain out of scope.
