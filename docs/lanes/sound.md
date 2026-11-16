# Sound lane living status

FROZEN by owner instruction2026-11-16 10:49UTC. No new implementation/tests until owner resume.

Last reviewed implementation head:e6c76e3b470d61b1c2f9e2a884efd6be18fa9373.
This freeze records existing results only.

Updated2026-11-16 10:49UTC. Owner: Sound. Target: integration/dnbox-consolidated;
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

## Owner freeze and finish-line coverage

Owner requested immediate backup and stop. No work is marked finished. No
owned DSP/native/test job remains active. Next action is owner-directed resume;
previous time targets are superseded by this freeze, not successful delivery.

Independent e6c review PASS_COMPONENT_ONLY reproduced the new source adapter,
metadata and model vectors. Native baseline independent review verifies5,380
listed paths match e742,20artifact identities,4ELFs and deterministic app+loader
package reconstruction. Isolated e742 preflight now16/0 after genuine matching
artifacts were supplied; this does not clear runtime/new-engine/combined gates.
Receipts:docs/sound/freeze-20261116/.

Original56preset reference-control/raw-versus-compact/common authenticFX replay
also completed:56presets,5,376,000frames,bit-identical pairs. Fourteen outputs
exceed unity in BOTH original and candidate, max3.23241091. No attenuation or
clipping workaround was introduced. This replay does NOT prove the new public
adapter includes fullFX, or an independently implemented targetFX is identical.

Source references consumed: RULES(1).md v0 (19:50VLAT; change log through20:10),
MIGRATION-STATE-20261116.md v0 and the authenticated engine handoff/source.
OPEN-ITEMS.md has not been located in the accessible input set; complete
request coverage is therefore NOT claimed. Existing finish-line table remains
open, including the original-clock P2 test interaction, authentic full graph,
preset/format/migration controls, resource/RT and required final visual review.

Top3risks: (1)stock flash/pool admission,172B baseline pool margin; compact model
still does not fit; (2)authentic full graphs/closed FX and missing original final
render oracle; (3)extended real-control/save-safe schema with legacy identities
preserved. No PSRAM or invented originals/capacity/timing.

Preservation correction: both earlier published heads now have genuine Git
bundles in Library, not just source tar backups. sound-20261116-1040-11a2f97b.bundle
and sound-20261116-1040-e6c76e3b.bundle require public8a0ef30c base. Exact raw
commit objects, bundle verification and local restoration verified all293/319
logical sourcefiles; Library roundtrip hashes match. Earlier source archives
remain independently recoverable. Latest freeze head gets its own bundle after
push. No installer, secret, main/force/Actions/device operation occurs here.
