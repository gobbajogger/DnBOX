# Lane: sound
Owner: sound agent. Branch: lane/sound-v2. Target: integration/dnbox-consolidated.
Base: e742be7bd0d01323c899de6e156f40b85ee451db. Updated: 2026-11-17 01:09 VLAT.

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

Memory reuse checkpoint: serialized held-note snapshot now aliases owner, native10532→8996 bytes (1536saved/part). Full original suite38cases354907688checks/114196frames0failures; C/C++/oracle576000frames0differences; all29 native units compile. Independent review pending, still no combined fit. Source manifest retains prechange hash. Next: coordinate FX lifetime reuse and further representation savings, no musical feature cuts.

Independent reviewer reran e955 and compared output bytes to prior5e541: exact match. Overlay oracle/test protection regression added,4 runner tests PASS. Native29-object unlinked code33406+readonly3230+compact165292 exceeds current13.4KB XIP headroom; flash remains a separate blocker, no protected-map/user-slot workaround claimed.

Packed held-note checkpoint: source-domain record now4 bytes (note/velocity0..127,transpose-24..24); invalid constructor args use255 sentinel and both insertion routes reject rather than wrap. All802816 valid triples retain exact fields. Native owner7972 bytes, all29 units compile. Final original suite38cases354907688checks0failures and576000frame C/C++/oracle compare0differences; audio bytes match prior5e541. Independent review pending; preFX-only, app/FX lifetime+flash still blockers. Descriptor2 experiment rejected/reverted after shared raw oracle initialization failed, so no descriptor or test-oracle changes shipped.

Explicit pi32v2 C allocation contract now7972bytes/alignment4, checked by static_assert in actual native compile; receipt docs/sound/v2-native-owner-layout.json. Invalid packed insertions preserve whole Held object; typed record is internal runtime only, not a persisted/upstream schema change.

Independent final packed866 review: fresh 12-scene/576000-frame replay0 differences/0 render heap; exact audio comparison against pre-alias/pre-pack5e541 also PASS, SHA2564b32746b36590998cf7c215ec07d5890d394414e8b2702b7a1a5e847c8ed6b34. Four runner negatives PASS. Component evidence only; no app/full gate approval.

Factory audit checkpoint: all640 SuperJ6 presets/12banks parse and90549 parameter entries are finite/unique per preset; 174identities,172known plus neonlimiterAmount240/neontubeRageAmount34 unknown occurrences. Coverage intentionally FAIL, no dropping/aliasing. Seven synthetic validator regressions PASS. S5 still FAIL. Original bank publication blocked by automated disclosure review; checked exact owner rule13, no secrets, archive/official-page provenance, redistribution rights remain unresolved (not prohibited). No retry/route-around; parent handles exact owner clarification. Original bytes preserved locally. Continuing factory frontend and exact unknown-field research without public bank payload. See docs/sound/v2-factory-input-status.md.

Original SynthOne factory recovery: officialMIT source6466a371 (license blobbe8551b8) supplies694 entries across12 non-User banks; all original Gitblob hashes verified. This is separate from the blockedSuperJ6 corpus. Source-derived144-command frontend plus common original FX exploratory replay passes all694/66,624,000frames,0different/0nonfinite; prior56-program output before/after also byte-identical. Not an Apple app/tuning/hold/modwheel, xruns, long-tail or nativeFX qualification. Copied-fixture fresh runner replay in progress. Generator initially globbed newMANIFEST.json as a bank; fixed to explicit authenticated manifest entries and added synthetic regression. No runtime edits.

Measured originalFX48k arrays: delay7,680,016B,reverb107,440B,widen9,600B before objects. Correct actual delayTime index43/domain≤2.5s reaches5s primary delay and2.5s secondary; earlier index36 inference withdrawn. Even naive reachable-range rings~2.88MB are unadmitted and far exceed combined native free memory. Next: preserve full source evidence, independently review exact resource bounds and resolve faithful runtime design with integ/root. No musical cuts/PSRAM.
