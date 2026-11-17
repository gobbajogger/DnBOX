# DnBOX consolidated state

Updated: 2026-11-16 09:47 UTC. Accountable owner: Integration & Release. This is the single living integration reference. Lane handoffs are immutable evidence, not alternative completion claims. **Inventory/candidate preparation only: no integrated composition, qualified release, merge, branch deletion, repository rename or deployment has occurred in this consolidation.**

## Current decision and next checkpoint

- Owner19:43 requests `lead/consolidation-20261116` in both repositories. Initial firmware WIP already exists at `integration/dnbox-consolidated` commit `e30f9a3175a839f2bcacf0034328877383fc17fc`; preserve that ref and create the requested lead branch without force or overwriting any writer. First public-safe WIP push target: **09:55 UTC**, subject to exact source/readback and public-content review; report a concrete reason if missed. This is not a promise of passing whole-product qualification.
- Firmware candidate source recommendation: retained testmerge `8a0ef30ca3e60e9c2c2721f1c3b7b303e12b229b`, tree `0bff565aab165b1750535a6e1cf9cbd805e1f20d`. The live development target remains #869 `94197fc839b00aff47b98ca02f3c6396e4a6eabf` until independently verified integration. This source recommendation is not permission to reset any target.
- Website candidate base is verified #132 `3f22e000ad17456318cde997067c7fd45dd7e3b3`, #131 is1ancestor commit from main and #132 retains it with17further commits; no equal-tree claim.
- All five lanes resumed separate implementation under owner19:27. Consume frozen snapshots below; never overwrite a live lane worktree. No active branch is eligible for cleanup merely because its work was copied.
- Top risks: (1) stock-memory combined native fit and audio/timing are not qualified; (2) incomplete new-engine/storage and all-page UI integration; (3) denied browser checks and unavailable connector administrative mutations. PSRAM, force/main writes, Actions, network Git and device writes remain prohibited.

## Live repository inventory

Authenticated GitHub API pagination, matching-refs crosscheck and open-PR terminal pages agree. Published per-ref/per-PR CSVs are in `docs/integration/20261116/`; complete raw pages remain in the paired preservation archive. Counts below are BEFORE any consolidation operation, not after-cleanup counts.

| Repository | Branches | Open PRs | Live source/target heads |
|---|---:|---:|---|
| gobbajogger/X0X37-for-Loopy-Pro | 357 | 73 | main `035026fe70059af35e25c7c121b40f318b732fe3`; nightly `2dd06b46ff037f858765ddf40275d8da02a9066c`; gh-pages `cd345c949da096ee093dfd70bfadc6b185f6e2ed` |
| gobbajogger/x0x37-for-Loopy-Pro_nightly | 131 | 9 | main `8af48f83c21e76bc7ebac2ddbdb7c5e3ff0a81ea`; gh-pages `241196130538b5e6da7738f0c2ebbbdc57fce02e` |

- Preserve #951 `37c780cdf4b18e824c863d7cc2982792882f61cb`, #869 `94197fc839b00aff47b98ca02f3c6396e4a6eabf`, #944/#947/#948/#887, website #132/#131, protected/recovery refs and all unique work.
- Firmware #898 PR head reports `eac5504cc2f4c81b3e4932759b1d0d9a86643cd4`, while both live-head inventories report `qa/chicago-ui-acceptance-20261113` at `8954bc4c58a6d2ab02990dd070ea1b3e815892d8`: HOLD pending reconciliation. Do not delete or silently select one.
- 18 firmware and 6 website PR cached base SHAs differ from live target heads. CSV records both. `coordination/merge-queue` is absent and direct GET404; do not recreate or activate stale coordination.
- Repository metadata reports admin/maintain/push, but that is not connector administration authority. Protection GET returns403; readable rulesets prohibit bypass/force/delete on protected refs. Exposed API tools support branch/tree/commit/PR writes, but no rename, tag-create or ref-delete mutation is exposed. Rename/tag/delete capability remains unestablished; no unsupported endpoint or credentials route will be invented.
- Qualified release source and actual deployed-site provenance are **NOT ESTABLISHED** by a branch head. Firmware nightly, default main and Pages source are distinct.

## Frozen lane inputs and actual state

| Piece / owner | Exact durable input | Actual qualification / remaining work |
|---|---|---|
| Firmware UI | `consolidation-ui-20261116/MANIFEST.json`:43,265 references/5,200 unique objects; live UI5,378files closure `24f133feb0db2fe23e26c232c64c681f739f67ecb2c5243dd9e0d33ed19289e4`; integration5,353files closure `e83414d19fcb5c9ded6e943fada7dba86b44daacbaa1531fbf4cb9d99503eb88` | Immutable source/evidence archive hash `0fdd3896b17b7a0e37f2f4dc7d2f7299eaffcc11190e02ca1b6639721815b674`. Latest global composer is newer than candidate9, unfinished and not full-render accepted. Live UI writer resumed independently. |
| Candidate9 drum slice | exact producer closure `54525101c3a13dc467fcd560351228268fee720b9bd043112d9d8db9df47128b`;25overlays on941; profile `722c6f4cf22cb6eab0b7f871774a6762883e85fee052d6bddcd747384be3f05d` | Independently checked45constructed drum states,135panels, unchanged70%/12% thresholds, actual progression107/27/4/0. Hook recognition radius4/9 only; detectors unchanged.18unchanged mechanics tests PASS. Other domains NOT_RUN. Not all-page, units, device, native/audio or 10/10 approval. |
| Public gate #951 | `37c780cdf4b18e824c863d7cc2982792882f61cb`, tree `565d6725e8da95f745975eccddf7b22c7acc9659`,21test/docfiles | Independently corrected source/doc scope accepted;41mechanics PASS on that source. No product source or whole-UI qualification. Initial failures/tamper tripwires preserved. |
| Resource source | accepted bounded freeze `5070f079238683340c58a74ea8361caf4898e350188aaab5557717a9b14c42b5` | Four Elite/Pro PADCAL0/1 configurations pass recorded resource floor; pool only172B above8192floor. Newer combined UI diagnostic overflows XIP~4.5KiB before floor. Not a fit waiver or current composition build. |
| Sound engines/models | `FILES-AT-CONSOLIDATION.private.json`:23,387files/1,168,439,510logicalbytes,0read errors, hash `283436da65657348b85a3ad7573ad5f5d6c4e336f739a2ef9980df6122559bdf` | Corrective4367file freeze `ebb5962248b72d2d459c44edb804512bef7fd43b34e2b5b7a1582ed8c31e3375`; follow-on4369file `e402a06bbd5123cfbb78e3a83375e20daa6bacd476a8302c32889d7e5fa0714c`; private194file model freeze `72b55c462402bedc464b90912c7705a9948371350d902be7ace4183618ff798f`. Source/host-scoped acceptance only; original-loop clock/gain P2 remains OPEN; full graph64bindings/preset apply/RT/native fit not complete. |
| SynthOne recovery | exact source `56079bbdcb753c37d65aedd17ce49ba71c261797`; parent `26f300b04e7fe91df1aadf88167bc01f532a7358`; tree `baf1d1068dae85d3e1666a2df8b02e5437295d2c`;4276blobs/399trees authenticated | Existing hook measurement-only; missing later takeover export not recovered. New renderer/A-B implementation open. Historical slot-2 id 0x4F445231 loads as Synth One init; Juno save IDs keep their meaning; genuine new-engine admission is not a rename. |
| Remote | `pause-freeze-20261116T0920Z/remote-only-freeze.json`:69files/2,875,243B closure `e52ac5129481041eea00bedced3e762cdc2493687a3778c0a13372ffe8fa1a37`; independently rehashed all69 from frozenZIP with0failures | Public #132 `3f22e000ad17456318cde997067c7fd45dd7e3b3`; local broad R22 source not pushed/accepted. Typed controls/edit mechanics partial; FunkyBeats exact809e1e78 intake21pure tests but no port yet. Browser launchEPERM remains NOT_RUN, never rerouted. |
| Remote preservation | all-files4102files/754,127,199B closure `f50448e72f15b4aeee494a198ee04b8b107c2b39c0077745c06d2419cc6d5174`;8Library archives recovery-verified | Complete public3f22tree900blobs; comparison836unchanged/64changed-local/285local-only/0publicabsent. Local-only requires classification, not blanket commit. Current local Git is unborn intake fixture, not clone/HEAD. |
| Website | `website-pause-consolidation-20261116/manifest.json`:37owned public files+52tests/fixtures+8observed shared dependencies;97frozen hashes independently verified; owned-tree `8b82955084a85ff3c2b6da63a8d0c5bbae4333f5c0f5062e6a84c5538245cd08` |19routes+redirects, broad local UI not accepted.859evidence entries. Site writer resumes only owned scope on separate branch; shared tokens/Remote excluded. |
| Locked owner references | PDF `db826c9cbeb84971f2d030c847043a21749ad4172e30739773494df0c9acca17`; notes `1f6b0743bc8ba955a718d2a41421ec410a19cbb5c75ec3b9978b7a533304ed72`; canonical source-of-truth2891B hash `868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6` | Original PDF/notes recovered;112-original R19SVG source pack provenance not established.26captured source screens are not proof of112originals. Approved909+808common system and improved glyph references remain constraints. |

## Integration outcome coverage

| Requirement | Current verified state | Next action / owner |
|---|---|---|
| #951/#869 combined integration | source pins and distinct worktrees inventoried; not composed or landed | Integration: retained-content/ancestry map then isolated candidate, no wholesale divergent tree adoption |
| R012 combined Elite/Pro build | isolated earlier native resource results exist, not combined source | Integration: exact source +SDK/toolchain/model/PADCAL manifests, all4native builds and floors |
| R013 zero dropout/Chicago-Miami lag | no fresh combined audio/RT proof | Sound+Integration: reconcile original requests and exact engine timing baseline; native/device gaps explicit |
| R014 Chicago cache/Miami options | preservation required, not assumed from latest UI | Integration: source-diff attribution and focused no-regression tests before composition |
| R020/R046 fiveBREAK defaults/preview | exact five owner choices and current shipped preview path not yet reconciled | Integration+Sound: recover selections/history/source; do not invent defaults |
| R041 originals | accessible supplied originals retained; no verified missing known Library filename/version requiring reattachment | Integration: missing historical takeover export has no known Library identity; complete112SVGpack unestablished; request only exact genuinely missing originals if identified |
| R038/#887 canonical roadmap | existing Working/Pending/Complete and index remain authoritative; not yet reconciled to composition | Integration coordinates root sole roadmap writer; no premature Complete |
| R045/#120 voting | runtime proposal/design/active source not verified; older feature-intake namespace must not be confused with runtime voting | Integration: read exact issue120/history/branches; no stale observer activation or assumed vote approval |
| R040 cleanup/R039 rename | explicit names approved DnBOX/DnBOX-nightly; exhaustive refs inventoried, all HOLD | Integration: retained-content/dependency checks, applicable gates, independent before/after review, exact-tip archive tags and durable content, then eligible deletion/closure+rename only through supported authorized capability |
| R029/R037 independent review/emulator | narrow source reviews exist; required all-page10/10/emulator/device proof not established | Lane owners+Integration: exact final composition checks, preserve denied checks as NOT_RUN |

## Required triple-check before merge/deletion

1. Exact retained-content diff proving all commits/files recoverable from the consolidated branch or exact-tip archive tag; named durable destination and active writer/PR/automation dependencies verified. Tag alone is not integration.
2. Full applicable host, geometry/UI-defect, native resource, audio/timing local gates at the exact consolidated source, retaining every first failure, actual exit/log and no prior-pass regression. NOT_RUN/blocked does not satisfy a required gate.
3. Independent equivalent-state before/after render and behavior comparison against locked baseline. Full logs for all three per batch. Any missing proof/regression blocks merge/deletion. No force/main/protection/Actions/device operations to evade this.

## Preservation and coverage boundaries

Owner19:35–19:36 supersedes earlier asset exclusions: coefficients, captures and binaries are authorized for Git as well as Library; only secrets/tokens remain excluded. Owner identifies SDK libraries as MIT-licensed: retain exact license notices, provenance and byte identity. Use a supported large-file route for large artifacts; do not substitute an unresolved LFS pointer for retained bytes. Do not execute unrecognized software without applicable confirmation. Git blob creation supportsbase64 binary; no LFS transfer tool is exposed. Large artifact upload remains a specific capability check, not an exclusion. Each eligible public artifact requires paired exact GitSHA +same-byte Library archive named lane/date-time/SHA; all nonsecret originals and binaries remain subject to the owner’s latest Git+Library preservation instruction, using a supported size/transfer route. Existing source/evidence checkpoints are durable but do not yet constitute the new paired consolidated artifact.

Repository branch/PR inventory is exhaustive at capture time. Local inventory covers submitted cloud lane roots; files and evidence counts overlap and must not be summed as unique work. The user Mac-mini is offline at fresh environment listing; inaccessible desktop files, separate historic chats, sample selection history and every old proposal are not exhaustively recovered. Sound/website/Remote/UI owners supplied actual snapshots, not full historical completion. No new attachments requested merely because an old name mentions private. All open/failed/missing proof remains open above.

## First candidate composition and checks

Firmware candidate1 is a single-parent WIP composition on8a: all5,353base blobs individually Git-hash verified, exact14resource5070overlays and21byte-identical #95137cfiles. These copied changes are not landed merges or ancestry-retention claims. No GitHub workflow files exist. Latest liveUI/global/Sound changes are not included yet. Focused first-run41mechanics PASS; fontpack and exp-curve tests exit0; compact-program test performs796checks then exits1 because LeakSanitizer reports fatal traced-runtime incompatibility. Original log/exit retained, no sanitizer suppression or expectation change. Full host/native/UI/audio/timing qualification NOT_RUN for this composition. FirstWIP is intentionally unqualified.

## First WIP corrective record

Firmware first public candidate commit `e30f9a3175a839f2bcacf0034328877383fc17fc` was read back at09:45UTC on integration/dnbox-consolidated. Independent source review confirms all14resource overlays and21gate files match intended bytes and no new secret/internal-agent markers; exact-composition acceptance is pending. Four inventory CSVs were unintentionally newline-normalized CRLF→LF by text upload: rows were retained but the first remote tree did not match the local byte manifest. This corrective update restores original CSV bytes and corrects two documentation references; no product source, tests, thresholds or failure expectations are changed. The first commit and original failed logs are retained. Source code acceptance is not native/UI/audio acceptance.
