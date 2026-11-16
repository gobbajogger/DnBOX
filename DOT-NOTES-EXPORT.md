# DnBOX saved-context export

Snapshot prepared for the November 16, 2026 backup. This is a plain-language account of the DnBOX context saved by Dot, not a dump of system instructions. It separates historical notes from the current instruction. Saved reports are not fresh proof of branch state, hardware behavior, qualification or delivery.

## Current instruction and backup state

The owner has frozen product development. The current authorized work is to finish preservation, not stop because one transfer route failed. The latest request replaces the earlier incomplete-backup stopping point: do not use network Git pushes from the unsigned-in shell; commit local WIP on backup branches, make and hash bundles, package the non-rebuildable workspace, upload all backup files to the private Library folder DnBOX-backup-20261116, and publish the manifest and three exports on backup/dot-20261116-manifest through the GitHub connector. Do not delete anything. Stop non-backup agents. Reply once after every manifest row has been uploaded.

Earlier today Dot reported an incomplete backup at commit 73d829c2a86d3d00a354325b27f5be6a1cdf962c. That is not the final backup. The requested nested manifest branch conflicted with an existing backup/dot-20261116 branch, so the current flat manifest branch is being used without deleting the existing branch. The earlier freeze/wait instruction must not be read as a reason to abandon the newly renewed backup request.

Local discovery reported 27 Git working locations across 16 independent Git stores, including 20 locations with a committed HEAD across nine stores and seven unborn/index-only locations. The discovered workspace was about 12.9 GB before the present backup. These figures are inventory observations, not proof that everything has been preserved. The shell has Git LFS but no GitHub login; the connector test write worked.

Prior lane preservation receipts cover Sound bd605508, Firmware UI 3ed862ac, Website 7262bb68, Remote 3f22e000, Integration 331afc9f and QA e742be7b. Thirty-nine read-only hash checks matched those stored receipts/materialized copies. A Sound archive of 506,876,743 bytes was split into three Library parts and round-trip checked; other lane archives and bundles have their own narrower receipts. Those packets must not be mistaken for a complete whole-workspace backup. Historical recovery records still say the R12 PDF, original morph artifacts and raw browser receipt were not recovered; a reconstruction is not the original.

## Product and musical purpose

DnBOX is the chosen public spelling, with a lowercase n. The project turns inexpensive M-Vave SMK-37 Elite/Pro hardware into an expressive, intuitive and dependable electronic-music instrument. The aim is exceptional craft and real creative freedom within strict hardware constraints, including stage and session use, not merely a large feature count or something adequate for the price. The owner hopes to demonstrate it in a real-world video.

Chicago is the 303 engine. Miami is not a 303. Later work also includes Juno/Jun60 and Drone, 909/808 drums, BREAK samples, sequencer, FX and external/SMC workflows. Engine fidelity, sound-bank usefulness, optimization equivalence, presets and complete app integration are separate outcomes and require separate evidence.

The owner plays live finger drumming, FX and synth chords/melodies. His music context includes Ableton Live, Logic Pro, Loopy Pro, SP-404 mk2, iPad and SMK/SMC controllers, with the LOOKOUT collective. Those use cases explain the emphasis on immediate response, held-note continuity, musical exploration, safe performance controls and recoverable work.

## Working style and coordination

The owner favors concise, direct, evidence-based language, candid criticism and practical decisions. Avoid filler and unsupported confidence. Explain a specific problem and next action rather than using a generic issue label. Ask only when a consequential ambiguity remains after recovering the available context. Keep handoffs short while preserving links to the full relevant evidence.

Use the smallest useful number of large, accountable workstreams, with one writer per shared scope. Reuse existing branches, PRs, tests and verified artifacts. Do not mistake a worker assignment, draft PR, source acceptance or schedule admission for delivered functionality. Verify that a recipient actually consumed the input and made substantive progress. Respect current ownership, uncertain admissions and prior denials; do not create a second writer or route around an access refusal.

The latest operational lane arrangement was Sound, Firmware UI, Integration/Release, Remote and Website, plus independent test-and-merge. Dot was again the main owner after an earlier transfer to Grok, with Grok Orchestrator as backup watcher. The later backup freeze overrides product execution. Older Grok bots were explicitly stopped; historical owner or task receipts are not proof of a current writer.

The owner requested GPT-6.1 Sol with ultra reasoning for Dot agents, standard service only and no fast mode. Classic work was a fallback when Dot capacity was unavailable, using Astra with the requested high reasoning where supported. Do not claim effective model settings merely because they were requested. No Cursor cloud agents or GitHub Actions. No PSRAM, force-push, main writes, device writes or flashing under the latest operating limits.

The canonical roadmap is split into Working, Pending and Complete, with the roadmap index/history linking them. Coordinate a sole roadmap writer. Keep live integration target, qualified release source, website source and deployed pages separate; do not assume they contain the same commits. Retain source-linked requirement coverage, owner, exact input, evidence and next action.

## Preservation, publication and data boundaries

The owner has progressively broadened preservation permissions for development inputs, coefficients and his own audio captures. Historical exclusions must not override the current explicit backup request. The current backup omits secrets and the named SuperJ6 IPA/AU installer material, and skips rebuildable caches or content recoverable from Git. Do not publish private credentials, tokens or account information. A file being excluded from public Git never means it may be discarded.

A prior scoped decision approved 12 already tracked coefficient/mapping/factory headers and prohibited rewriting them or repository history on that basis. Earlier notes kept newly supplied fitter coefficients private unless separately approved; subsequent direct owner permissions must be checked when applying that historical boundary. Public redistribution rights and actual access restrictions remain separate from ownership and backup intent.

Suitable public source, tests and documentation normally belong in the project repositories, with important private or unsuitable artifacts in the owner's Library. Record exact version/hash, provenance, recoverable location and restoration instructions. Verify uploaded bytes where possible. Keep originals and useful reconstructions, identifying them accurately.

## Firmware correctness and resource requirements

Preserve held notes across menus, popups and part/page changes until the physical release. A held input remains owned by its original part. REC and Free Play must handle the full 37-key range and the agreed drum/BREAK mappings, not a convenient inner subset. Per-pattern engine selection should restore the selected engines when returning to a pattern rather than inheriting another pattern's choice.

Sequencer notation and controls should support clear pitch/chord names, inversions/extensions, Juno/Drone held notes and piano-roll workflows. Straight 1/8 and 1/4 rates were requested across relevant paths. SMC lock state, peer independence, held FX ownership, MIDI/transport behavior and LED feedback must not regress.

Historically required floors were at least 8,192 bytes of free flash on both models and at least 8,192 bytes of pool reserve, with exact SDK/model/PADCAL configurations recorded. JieLi SDK V1.2.14 is the later baseline; earlier V1.2.1 results must not qualify a V1.2.14 composition. The required controller pin was a334208b, 61,552 bytes, CRC32 564a8f55, with direct byte/provenance comparison required rather than comparing differently scoped padded image lengths or CRCs.

Protect BTIF, USRTRIM, efuse, key_mac and bootloader/calibration regions. A proposed mandatory m-bootloader path had reported USRTRIM erase and model-identity hazards and was not a safe rollout merely because it was requested. Preserve samples and user projects during reset, install and recovery.

Native builds, host tests, emulator results and real-device tests answer different questions. Exact input matters: a standalone donor's resource or timing result does not qualify a later combination. Reuse unaffected proof, rerun affected checks and label unavailable checks honestly. Do not turn raw suite failures into clean passes because isolated reruns passed. Browser-only checks were excluded from a historical firmware merge bar, but that did not qualify the website itself.

## Performance and audio history

The 0.0.24.1 device build was reported too laggy for live use. LCD blocking and unbounded dirty rectangles were important causes investigated later. Bounded tile DMA and frequent input servicing were retained as key behavior. The later L14 work exposed peer/page/state coherence problems during header and body transfers; tests needed real callback injection rather than a counter-only stub. Preserve controls, note releases, MIDI, recording and PANIC while fixing paint coherence.

The owner later reported Chicago arp plus a held Juno triad, BREAK and a four-on-the-floor 909 kick producing page-entry lag and audio dropouts. Replacing Chicago with Miami was more responsive and sounded clean in the reported comparison. This is an owner observation, not a complete timed benchmark. Suspicious PERF zeros for Miami/BREAK and idle 808 spikes cannot be used as reliable execution-time evidence. Meter repair needs a properly qualified shared elapsed clock and denominator, including concurrency/fallback cases.

The historical severe-load acceptance calls for declared workload and duration, synths, drums, FX tails, MIDI, SMC and UI interaction, with zero observed dropouts and accounting for late blocks, underruns, discontinuities and lost events. A host CPU comparison with excessive variance is inconclusive, not proof of never-slower execution.

## Release and installer history

The 0.0.25 and 0.0.26 histories include many partial candidates and changing acceptance decisions. In particular, an earlier exact-native bit-identity demand for #872 was later replaced by the owner's defined heavy timing/dropout gate: under 11,609 microseconds, zero late blocks and all 648 halves on both models, with accepted one-half pad slips. The cancellation of the final host suite must not be undone by stale automation instructions.

The initial 600b3b45 0.0.26 packages were delivered, then withdrawn after boot/update failures. A corrected 94197fc8 candidate was later published and the owner reported smooth installation, automatic restart and normal boot on Elite. That bounded observation is not Pro testing, exact installed-package hash proof, or full functional qualification. Subsequent Chicago musical-scene dropouts remain a separate failure.

The 25.1 fallback was considered good by the owner and was made available through MAIN. Historical removal of broken 26 selection/URLs is recorded as completed, not permission to delete current backups. Installer sample backup should warn and continue where that is actual approved behavior; documentation must distinguish old fail-closed branch behavior from deployed source. Never substitute stock FM-1 recovery instructions for the real DnBOX route.

Naming work covers DnBOX labels, application/package filenames and truthful USB/MIDI/BLE model identity. Compatibility aliases and old release URLs should be retained or fail closed, not silently dropped by regeneration. Proposed repository renames were planning only because GitHub repository redirects do not preserve Pages URLs.

## UX, website, Remote and learning

The owner wants dark-only, authentic instrument utility rather than generic mockup styling. Baud Girl was the favored design reference because it shows meaningful interactions. Research should explain what excellent firmware pages include, why, and how the actual install/backup/recovery/control journey works. SMK firmware comparisons come first; other grooveboxes are secondary.

Design must cover the entire journey: discovery and musical purpose; try real firmware; first sound and save/perform; phone, tablet, computer and external gear; install, update and recover; presets/sample management; accessibility, mobile, keyboard and understandable errors. Firmware workflows should be audited before screen polish. Protect work without suppressing happy accidents or experimentation.

Fresh independent reviews should be candid and blinded to previous ratings or target scores. Test actual interactions and available real emulator behavior, preserving version and limits. Device behavior must be labeled untested when only source or emulation was inspected. Plans, mockups and runtime integration have distinct approval gates.

Remote is not the same as an offline editor, device companion or firmware emulator. The later Remote baseline used one panel system, dense pages and large labeled glyphs. The latest owner direction removed edit/tool modes: editing, zoom and pan should use gestures. Per-step velocity/chance/pitch/pan lanes use thin value bars dragged to set, not mixer fader caps; the mixer keeps faders. Compare revisions in equivalent states against the accepted baseline.

The original Blender startup scene was recovered. The desired transition is short entry into the box directly to the current HOME UI, not the obsolete endpoint or a generic replacement. A 0.514-second preview and nonblocking scheduling evidence existed, but embedding a 90 KB video or runtime integration was not automatically approved. Direct-HOME startup remains until the endpoint/resources are qualified.

## Presets, storage and compatibility

DNPR/native record import, patch application, save/recall, web librarian, stock bank and plugin compatibility are distinct seams. Preserve engine/schema/target checks, provenance, factory precision and stable factory IDs. A native record format alone does not establish plugin compatibility. Factory bank auditions do not prove J6 DSP fidelity.

Historical F1/W3 review found engine-ID/target mismatch handling, mixed-batch warnings and per-record preview problems. Browser hold ownership needed counted ownership across keyboard, canvas, accessibility and unique pointer identities, including cancellation and blur/reset. Saved-slot/project recall and both-model budgets remain necessary before integration.

Storage safety notes include transient read failures being mistaken for blank/invalid slots and erased during SAVE, reset/settings fault hazards, retained-state startup ordering and cross-client lifetime concerns. Those source/model findings must remain separate from observed hardware data loss.

## Open commitments retained in saved task notes

The saved commitment list was last maintained mostly on November 13 and is historical, not current completion proof. Its continuing scopes were: reconcile the old 0.0.25 delivery; align the user manual with final source; close all approved 26 scope and functional issues; boot/update regression evidence; safe m-bootloader assessment; real device identity; whole-journey website and firmware UX; actual firmware emulator; original startup animation; truthful release/admin qualification; shared design guidance; naming/application ELF; safe repository-rename plan; professional single-writer coordination; and reusable Classic self-handoff. The latest backup freeze prevents resuming these as product work.

The same saved list marked temporary broken-26 selection removal, withdrawn-package URL removal and MAIN 25.1 installer availability as done. These statuses require preserving their receipts, not repeating them or broadening them to unrelated outcomes.

## Coverage and limits

This export draws on the saved profile, current-state and ownership/data-boundary notes, the main LoopyX0X37 history and coordination continuations through November 13, the saved commitment ledger, and the current conversation's November 16 backup corrections. Older duplicate legacy notes were not separately reinterpreted as current instructions. It deliberately omits unrelated personal/account details, private system instructions, credentials and internal memory locations. It summarizes remembered facts and cautions rather than claiming a fresh audit of every repository or recovered artifact.


## Verified backup continuation outcome

The continuation completed and verified 27 Git bundles plus six Git recovery files, followed by 40 additional workspace, evidence and recovery items in the private DnBOX-backup-20261116 Library folder. All 73 payload items have successful upload receipts and matching folder readback names and byte counts; the payload total is 3,672,954,701 bytes. These counts exclude these final control documents.

The main workspace archive has 15 parts totaling 2,604,423,226 bytes and preserves 151,907 entries, 7,147,696,313 logical bytes, and 3,491,558,262 unique regular-file bytes. A complete streamed read verified every payload SHA256, hardlink relationship and symlink target against the inventory, with no missing or mismatched entry. Additional temporary evidence preserves 1,355 selected paths and 55,551 further paths, plus the separately reviewed UI packet ZIP.

All 27 bundles were restored independently with matching WIP trees and successful fsck results. Fourteen required explicit shallow export boundaries because original history was incomplete. The old remote ref build/0.0.22 at 2456968ba39b521610364f3ea17a118b456269c0 had an incomplete tree and was omitted from two export bundles; available original objects and metadata are retained in raw Git stores. This is not proof of full historical recovery.

The SDK ZIP is a reconstruction retaining all member names, with only 13 embedded PEM key payloads redacted in eight sample/test files. Three parser/writer files containing delimiter constants remain exact. The original ZIP is unchanged locally. The reconstruction is 309,787,813 bytes, SHA256 cf53391acd7c9bafbf690c93fae1d9ba59f2053343f4d57ee3a1b4737fcaa0c0; member-level provenance is in the archive inventory. The flagged keys were samples/fixtures, not established live account credentials.

Excluded from the payload are the requested SuperJ6 IPA/AU installer, rebuildable caches/build directories, internal authentication/runtime configuration and redundant backup outputs. No original was deleted. Screening covered nested compressed content and recognizable secrets/private-runtime markers; bounded pattern checks are not a guarantee against all conceivable secrets. No product work, device operation or shell Git push was performed.
