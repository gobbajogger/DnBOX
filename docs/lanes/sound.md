# Lane: sound
Owner: sound agent. Branch: lane/sound-v2. Target: integration/dnbox-consolidated.
Base: e742be7bd0d01323c899de6e156f40b85ee451db. Updated: 2026-11-17 00:35 VLAT.

## Checklist
All final checklist items remain FAIL until exact combined-source proof exists.
| ID | Status | Current evidence / next check |
|---|---|---|
| C1 | FAIL | Local gate not run on this lane; independent test-and-merge owns it |
| C2 | PASS | This commit descends directly from verified consolidated e742be7b; no deletions |
| C3 | FAIL | Independent review not run; no UI changed |
| C4 | FAIL | Full hard-limit scan not run; current change adds a pre-FX adapter and host tests only |
| C5 | FAIL | Outstanding S1–S7 below |
| C6 | FAIL | Draft continuation; not merged |
| S1 | FAIL | Reuse pinned 56079bbd source closure and public renderer from prior sound source; factory-wide finite/xrun proof remains missing |
| S2 | FAIL | Existing original-versus-compact 56-preset full-FX replay is scoped evidence, not every factory program |
| S3 | FAIL | Existing 64-control schema/model component checks do not prove full routed engine |
| S4 | FAIL | Original 14/56 comparison input available; complete intended-engine A/B pending |
| S5 | FAIL | Complete 640/12-bank mapping not verified |
| S6 | FAIL | Prior baseline native links are not combined all-engine fit; exact stock-memory proof pending |
| S7 | FAIL | Consolidated still contains Drone; coordinated removal required |

## Now / next
- Now: isolate exact consolidated source, recover existing 56079 renderer/source closure and sound components without rebuilding unchanged tests.
- Next: carry original factory control bindings into the adapter test and qualify factory-wide finite output; then integrate the actual runtime with stock-memory accounting.
- Fresh rebuild: 28 objects compiled from retained source, 12 scenes / 576000 frames, zero differing channel words versus source-derived oracle, zero heap calls during rendering. This is pre-FX component evidence only, not S1/S2 or hardware xrun PASS. See docs/sound/v2-pre-fx-evidence.md.

## Blockers and remedies
- Prep files/gate not integrated yet: use attached RULES.md v2 as authorized input; keep sound edits isolated and merge prep target once accepted.
- Existing sound PR #952 is on sound/engine-rebuild, not lane/*: preserve it unchanged as donor; this is its sole v2 continuation, not a second sound writer. Do not close it before rule 9–10 checks.
- Local baseline Git history has absent older objects: exact HEAD tree is available and exported; remote connector is authoritative for ancestry and writes.

- Evidence defect fixed: old adapter runner hashed source but reused old dependency objects. Regression reproduced this; new runner recompiles every object and rejects link inputs without a compile command. Source build still requires a trusted separately pinned closure.

## Top risks
1. Original DSP graph and complete factory bank differ from the component adapter evidence.
2. Combined engine stock memory not qualified by historical baseline maps.
3. UI/build shared paths require coordination with fwui and integ; no edits there in this checkpoint.

## Deletes
None.

## Conflicts
No rule changes. Prior rules/briefs are history only; existing code and evidence are reusable inputs under RULES.md v2.
