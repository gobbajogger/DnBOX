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
| S6 | FAIL | Native pi32v2 pre-FX owner10532 bytes measured; historical baseline margin172 is340 below rule minimum; no combined fit. See sound-memory.md |
| S7 | FAIL | Consolidated still contains Drone; coordinated removal required |

## Now / next
- Now: isolate exact consolidated source, recover existing 56079 renderer/source closure and sound components without rebuilding unchanged tests.
- Next: carry original factory control bindings into the adapter test and qualify factory-wide finite output; then integrate the actual runtime with stock-memory accounting.
- Fresh rebuild: 28 objects compiled from retained source, 12 scenes / 576000 frames, zero differing channel words versus source-derived oracle, zero heap calls during rendering. This is pre-FX component evidence only, not S1/S2 or hardware xrun PASS. See docs/sound/v2-pre-fx-evidence.md.

## Blockers and remedies
- Prep files/gate not integrated yet: use attached RULES.md v2 as authorized input; keep sound edits isolated and merge prep target once accepted.
- Existing sound PR #952 is on sound/engine-rebuild, not lane/*: preserve it unchanged as donor; this is its sole v2 continuation, not a second sound writer. Do not close it before rule 9–10 checks.
- Local baseline Git history has absent older objects: exact HEAD tree is available and exported; remote connector is authoritative for ancestry and writes.

- Evidence defect fixed: old adapter runner hashed source but reused old dependency objects. Regression reproduced this; new runner recompiles every object and rejects link inputs without a compile command. Independent review caught unbound compile paths and missing harness detection; both regressions now pass with full 89-file closure verification and exactly one adapter substitution. Source build still requires a trusted separately pinned closure.

- Native ABI probe: pinned compiler rejects C++17; C++14 plus matching toolchain includes compiles type probe. Actual10532-byte pre-FX owner excludes FX and is not a link result. Integ notified of baseline margin shortfall.

## Top risks
1. Original DSP graph and complete factory bank differ from the component adapter evidence.
2. Combined engine stock memory not qualified by historical baseline maps.
3. UI/build shared paths require coordination with fwui and integ; no edits there in this checkpoint.

## Deletes
None.

## Conflicts
No rule changes. Prior rules/briefs are history only; existing code and evidence are reusable inputs under RULES.md v2.

Native recovery checkpoint: exact56079 full-sequence recipe rebuilt29 runtime objects with authenticated original SDK C++ headers, object forbidden-symbol scan PASS/0. SourceScheduler10532 bytes independently reproduced. No firmware link/device/combined-memory PASS. See sound-memory.md and docs/sound/v2-native-component-result.json.

Runtime source closure copied byte-identically into firmware/src/dsp/synthone with71-file hash manifest and MIT notices; all29 runtime units recompiled successfully from new repo paths. Not enabled in app/build. Next is replacement-safe C ABI/owner budget with integ, factory bindings and post-FX; simple union growth adds13624 bytes for two parts and is not admissible.

Schema checkpoint: recovered source-derived SuperJ664 slots/218 native IDs and SynthOne150 physical descriptors, contract/audit and reproducible generator. Strict cc -Wall -Wextra -Werror rejected donor misleading indentation; formatting-only fix passes. Existing descriptor/blob test verifies64 unique slots,218-word roundtrip,888 corrupt-byte rejections,finite/alias/transaction checks. Regenerated both descriptor tables byte-identically. DSP bindings remain NOT_ADMITTED; S3 not PASS.

C ABI checkpoint: cold caller-owned init/retire plus physical parameters/note/reset/render, with span/alignment/alias guards. Expanded actual audio test covers C/C++/source equality across12 scenes576000 frames and invalid bounds; native wrapper compile PASS. Integ notified; no pool allocation or app activation.
