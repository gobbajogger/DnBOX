# Sound lane living status

Updated2026-11-16 10:21UTC. Owner: Sound. Target: integration/dnbox-consolidated;
base8a0ef30ca3e60e9c2c2721f1c3b7b303e12b229b. Existing public checkpoint
sound/engine-rebuild@11a2f97bec7ad300d0acc02721175194e5a82f40; this working
update must be identified by the actual containing commit, never a future SHA.

## Implemented and measured

- Actual SynthOne pre-FX adapter, source dependency56079bbdcb753c37d65aedd17ce49ba71c261797.
  New12-scene source-host A/B:576,000frames,zero differing float channel words,
  529,280nonzero left frames,peak0.628623188,zero heap during render. Red counterfactual
  bypassed source held-key/arp state:700,576different words. Public key start/stop
  routing fixes it. Listening fixtures and scoped logs:docs/sound/ and tests/fixtures/synthone/.
- Fresh unchanged authentic full-source scheduler comparison:38cases,114,196frames,
  354,907,688checks,120,289state snapshots,1,081events,zero differences/failures.
- SuperJ6 procedural model:24whole raw-record model/golden vectors,7,812,906frames,
  finite/exact retained host outputs. These are not original AU/full-instrument nulls.
- Source-backed64SuperJ6 descriptors and150SynthOne native physical descriptors.
  New888-byte218-word serialized blob is standalone, never a reinterpretation of
  existing32-byte cells. CRC/finite/corruption/alias tests pass. All SuperJ6 DSP
  binding statuses explicitly NOT_ADMITTED; these descriptors do not finish64DSP routes.
- Genuine pinned candidate1 Elite/Pro x PADCAL0/1 native links pass. Static pool
  free8,364B,only172B above8,192B floor. PADCAL1 flashfree13,464/13,472B.
  This exact baseline excludes latestUI and new-engine activation. See measured
  source/config/artifact pins and top3 defensible savings in
  docs/sound/stock-memory-20261116.json. Compact model still DOES NOT FIT.

## Finish line and remaining scope

| Required outcome | Current result | Next evidence |
|---|---|---|
| SynthOne56preset real control laws + original A/B + fullFX | NOT_RUN on new adapter | source/control/FX composition and56preset red/green/headroom logs |
| Save-safe new engine4/5 admission, legacy2/3 preserved | NOT_RUN | firmware-owner shared schema/migration tests |
| SuperJ6 complete64bindings/preset apply/tuning/clock/FX/MIDI | FAIL/incomplete graph | every route actual output/state differential, original640preset coverage |
| Original SuperJ6 reconstruction fidelity | NOT_ACCEPTED | whole played/raw/onset/loop metrics and owner-approved sound threshold |
| Model original-loop x non-dyadic/gain fixture | OPEN P2 | restore endpoint per scene, clock+mutant rerun |
| Stock combined flash/pool/audio/stack/RT fit, noPSRAM | FAIL for naive new-model addition | qualified exact combined map + worst-case allocator/timing |
| R017 AudioKit/Juno60 sounding preset mapping | NOT_RUN | authentic preset mapping and listening/source evidence |
| Chicago/Miami/909/808/Break/other presets and affected QA | NOT_RUN under expanded Sound scope | exact consolidated source intake and regression matrix |
| R019 both original A/B benches | PARTIAL source-host only | complete original graph/fullFX, not common rewritten oracle |
| R029/R037 applicable independent/emulator gates | NOT_RUN | request/spec-linked exact-head logs |
| Independent final combined review + local gates + device | NOT_RUN | exact combined qualified inputs, no hardware inference from host |

Next source/evidence checkpoint10:45UTC. SynthOne56preset/sourceFX checkpoint
11:30UTC; SJ6 graph/binding go-no-go11:00UTC, full64binding checkpoint13:30UTC
conditional on closing authentic missing providers. These are next deliverables,
not release promises. No finished lane/device/release claim. No PSRAM, BT-off,
shortened non-fitting delay, removed reverb, networkGit, Actions or device writes.
All shared UI/runtime/storage writes remain firmware-owner only.
