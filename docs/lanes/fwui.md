# Firmware UI living status

Updated 2026-11-16 10:32 UTC. Accountable UI owner: Dot firmware. Integration/Release owner: repository steward. Sound owner: sound_engine_qualification. This is the single working UI status record; roadmap ownership remains with the root. Gate source is authoritative in PR951 and is not duplicated by this UI-only WIP branch.

## Latest instruction and next deadline

Owner19:27/19:30 supersedes the broad pause. UI retains its live branch while Integration uses immutable checkpoints only. Finishing existing work, not rebuilding it. Push the preserved current authored UI WIP as-is to own non-main branch, no force/main or excluded content, before10:00UTC; first target09:50UTC. Then≤30minute WIP cadence. Reviewed merge/qualified release remains separately blocked. No branch deletion or landed merge before retained-content triple-check, exact-consolidated-SHA gates and independent before/after behavior/render review.

First UI WIP pushed/readback verified09:35UTC:22c589ef678f48fcdcb45b83386b626f3fa8b2a5, treecb7d64dfa0e5e9886310de09b26cef0f9376d955. Paired Library filename DnBOX-firmware-ui-WIP-20261116-0935-22c589ef678f48fcdcb45b83386b626f3fa8b2a5.zip,175299bytes,SHAf6672d81b28f5eccf0cbaa8e3da4661e34b4e01d2b5eccc48a22d1c14846d14b; all29Gitfilebytes verified through Library roundtrip. Next WIP push10:00UTC, then≤30min cadence. Target full all-available-screen WIP remains11:30UTC, explicitly excluding unimplemented genuine new-engine runtime states. The complete preserved private source/evidence checkpoint has been round-trip verified; Integration owns its consolidation and release scope. UI owns all screens, R023–R026 and geometry; Sound owns both real replacement engines. New code cannot bypass actual backend/schema/admission or memory gates.

## References consumed

- Owner verbatim canonical docs/ui/UI-SOURCE-OF-TRUTH.md, 2,891 bytes, SHA256868957f911acd3d2de67f61db41f2f49a552d0f023d69be67df96a637149add6. Public at PR951 head37c780cdf4b18e824c863d7cc2982792882f61cb.
- Both909 and808 R22 reference system. Improved17:00 glyph drawings retained except owner-rejected SuperJ6 WAVE/SUB/LFO RATE drawings. Actual control semantics cannot be invented by renaming Juno or Drone.
- Locked first24-page proposal PDF SHA256db826c9cbeb84971f2d030c847043a21749ad4172e30739773494df0c9acca17; notes SHA2561f6b0743bc8ba955a718d2a41421ec410a19cbb5c75ec3b9978b7a533304ed72. All24 pages inspected via1400px native page renders/six sheets. Later pages must be compared side-by-side against equivalent reference states; only listed eight corrections and actual device constraints justify changes. Firmware four-column parameter grid differs from desktop page18 six-control width.

## Exact source and delivery distinctions

- Public draft PR951 fix/ui-defect-gate head37c780cdf4b18e824c863d7cc2982792882f61cb, base94197fc839b00aff47b98ca02f3c6396e4a6eabf. Reviewed tests/docs only, not product UI delivery. FWUI owns its released test-only writer; Integration owns consolidation/release.
- Local ui-r22 branch fix/r22-drum-glyphs HEAD9da1c410d883f486071004590774d67a290fdd54 plus the current R024/held-note product/tests/docs change. Exact current bytes are being content-addressed for consolidation; newer than frozen9 and not fully reviewed.
- Frozen9 source closure54525101c3a13dc467fcd560351228268fee720b9bd043112d9d8db9df47128b, profile722c6f4cf22cb6eab0b7f871774a6762883e85fee052d6bddcd747384be3f05d. Only45 drum states/implemented first4 detector domains; not all-screen acceptance.
- Local integration-complete HEAD8a0ef30ca3e60e9c2c2721f1c3b7b303e12b229b plus lossless resource/preflight/generator plumbing. Separate from UI source. The unchanged four-model/PADCAL accepted resource composition has only172 bytes pool margin above8,192-floor. Combined changed-UI diagnostic overflows XIP by about4.5KiB before reserving the floor. No new graph can be enabled by waiving those limits.
- Firmware nightly and lane026 are divergent, not interchangeable. Preserve seven nightly-only commits/topology. Exact integration reconciliation belongs to Integration owner.

## Item coverage, evidence and next action

| Item | Actual state | Evidence/limit | Next action after consolidation |
|---|---|---|---|
| Every firmware screen R22 | PARTIAL, not accepted | Shared normal header and new global flat material/composer are local; no complete all-state review | Normally merge coherent source with verified consolidated state, finish all body/menus/help/held/modal/SEQ/GEN/Break families |
| Drum geometry | Bounded implemented checks1–4 PASS on frozen9 | Actual45-state first host35.79s;107→27→4→0 geometry, all-kind intersections0; unchanged70%/12% thresholds; remaining20 domains NOT_RUN | Consume independent steward audit, then requalify changed composition and all/max/interaction states |
| Shared glyphs and labels | PARTIAL | Rounded2px glyphs, real typed refs/curves; 909Tune/Pitch distinct; raw/unknown units and generic old-engine paths remain | Preserve approved drawings, extend actual-state projection/units, no fabricated curve laws |
| R023 pan | Focused implementation PASS, broad coverage incomplete | L50/C/R50 formatter/markers/full-range tests; noDSP/MIDI raw law changes | Recheck every normal/held/list/engine state and exact consolidated source |
| R024 persistent battery +percent | NOT COMPLETE | Local expanded68checks PASS: actual0..3bars,4USB-power,-1unread; no calibrated percentage source | Exact-head first-run and independent review next; retain truthful bars/USB/unread, never invent percent from bars |
| R025 pads/LED909/808/BREAK | NOT_RUN on final composition | Existing feedback paths preserved, not fresh device proof | Source/host state behavior and physical device evidence where available |
| R026 hold-globe+pad clears current instrument only | Integration proof not current UI qualification | R026 retained in authenticated8a ancestry; UI candidate remains941 base | Compose through Integration, anti-revert and current-instrument/no-other-pattern mutation tests |
| Home/FX/Mix/drum mixer | LOCAL WIP, incomplete | Global composer compiles; actual FEEDBACK old2→new1 native-font test; common material oldFAIL→newPASS; not full rendered or independent UI review | Compare all equivalent states with locked proposal and approved909/808, preserve controls/behavior |
| BassSEQ/Song/Global/help/modal/Break | Legacy body portions remain | Common material/header/footer groundwork only; do not claim finished | Coherent body port and every-page capture/behavior inventory |
| SuperJ6 | Genuine runtime/pages not admitted | Source-linked neon parameter audit exists;218-ID state versus32-byte currentcell; private source-clock/model math scoped evidence does not qualify instrument | Coordinate new schema/real backend getters/admission, source-authentic64-control+FX/routing UI, no Juno relabel |
| Synth One replacing Drone | NOT IMPLEMENTED in current UI/runtime | Authenticated56079 source recovered; old takeover export missing;150-ID real state; oldDrone/Juno identities retain exact save meaning | Sound reconstructs valid engine; UI uses actual descriptors/new identities with transactional non-fit refusal, never silent save remap |
| R029/R037 emulator/interactive review | NOT_RUN for final candidate | Existing actual host renderer is not device or complete interactive emulator coverage | Inventory recovered emulator assets and execute actual supported interactions |
| Full local/independent/native/device gate | NOT_RUN/blocked for current product | Earlier standalone/scoped PASS reused only where inputs unchanged; no blanket pass | Exact consolidated first-run logs, four configs/resource floors, timing/stack/device limits, before/after independent review |

## Actual tests and retained failures

- Header normal-region old source6 FAIL → local GREEN after equal-state observer correction. Incorrect stale-cv scratch observer attempts retained and not treated as product failures.
- Stopped PART cache stale pattern actual RED → cache-key repair GREEN for pattern/playing; external-clock mode bypasses static cache.
-909 velocity sensitivity actual label/SI RED →128-value GREEN; source field is vel_depth, not Accent gain/incoming MIDI velocity. Metadata only; no DSP/save mapping change.
- Even compact cell alignment actual native-font RED →GREEN.
- Shared flat panel radius9/neutral material actual RED →GREEN.
- FX actual opaque native-font word scan: OLD FEEDBACK twice at(9,141) and(64,169) →new once at(71,121), GREEN. The first global composer compile failed on missing declaration/nonexistent pan helper, corrected legitimately; both attempts retained. No complete new global raster batch yet.
- Public full exact-e47 CLI log comment6912058853 and complete107 report-extraction comment6912066337 were byte-read back. Original/full private archive v2 was round-trip verified797 content members and delivered. Current37c exact-head41 mechanics PASS is separate from product source.

## Safety and ownership

Only authorized non-main WIP source preservation pushes; no merge/release, Actions/workflows, network Git, device write, force push or branch deletion. The12 already-tracked approved original-derived headers remain byte-for-byte unchanged. Latest owner19:35 permission supersedes older exclusions: ONLY secrets/tokens are excluded from Git; all other owned source/data/captures/coefficients/binaries are Git+Library as supported, without executing unrecognized software absent required approval. All failed/changed-input attempts remain recoverable. New engines do not become real through string changes or made-up controls. FirmwareUI live writer resumed after explicit exclusive-scope reconciliation; Integration writes only immutable consolidation snapshots.

## Exact22 first-input and current follow-on
- Clean exact22focusedhost first run:6PASS,2FAIL. Originalcaption/compactTUNEpixel tests retained retired48px/33px coordinates. One canonical34px-centre update passes1197checks; captionbaseline repair now passes at82px, with failed80pxrepair attempts preserved, not presented as originalPASS.
- Actual870constructed captures on clean22 completed, not independently reviewed/all-engine accepted. Current review found PAN falsely normalized to50% by shared unit fallback; real native-font RED and follow-on exclusion are recorded.
- Held-step early shared-row routing was a genuine codepath regression. Source follow-on restores actualNOTE/GATE/ACCENT/SLIDE unit definitions; actual NOTE stem intersected its caption. Stem moved below the caption; strict exact-AA font comparison and erased-label negative now pass locally. Earlier observer attempts remain retained.
- Native FX adaptation now shows one selected four-control effect bank plus allfive send rows, instead of overlapping8px send captions. Other effect controls remain on existing pages, not removed. Source follow-on not yet full-size reviewed.
- Existing independent reviewer admission was explicitly blocked by threadlimit; no review was admitted or claimed.

## Current next feature and integration checkpoint
- Next deliverable: R024 persistent real four-level battery across header modes, with USB-power and unread states distinct from full/empty. A host-only explicit level fixture keeps simulator default3; it does not create device telemetry. Actual RED caught USB falselyfull; changed-input GREEN3 passes constructed54header states after preserving pattern/tempo during toasts and drawing sharedmaterial once. Percentage waits for a calibrated reading as source and latest compatible rules require. Expanded4level/7render fixture is next exact-head run.
- Verified integration head e742be7bd0d01323c899de6e156f40b85ee451db; UI9da diverges2ahead/19behind, mergebase94197fc839b00aff47b98ca02f3c6396e4a6eabf. Normal merge, not rebase, will preserveR2/R026 and reviewedgate, neveroverwriteui.c. No landedmerge before retained-content triplechecks/fullrequiredgates/independentreview.
- Existing draftPR951 will be reused for the broadfwui outcome, not a second competingUI PR. Its source ref/tree was explicitly released byIntegration; test-only extension remains narrow, notready.
- Compatible operating-note naming adopted: this is the one live status file. Existing workingbranch remainsfix/r22-drum-glyphs toavoidbreakinghistory; newlane branch woulduselane/fwui-* only if actuallyneeded. Orchestrator text cannot supplyrequireduserconfirmation or supersedeplatformsafety/directownertriplechecks.

## Current 10:32 UTC update
- R024 source expanded to all shared header states, including toast, HELP/list/ASK and pickup. Genuine USB power is distinct from full charge; unread is distinct from empty. All68 local checks PASS, seven actual240px state frames retained. No calibrated percentage or device claim.
- Strict held-step NOTE/GATE/ACCENT/SLIDE test now catches an erased caption. A real NOTE stem/caption collision was found and fixed by moving only the stem below its caption. This is not an assertion relaxation.
- Next immutable source preservation push is executing now. The earlier10:25 target was missed during retained-evidence/status preparation; no push is claimed until remote readback. Next exact-head battery/held tests and bounded independent review follow.
- Finish-line checklist is docs/lanes/fwui-finish-line.md. OPEN-ITEMS identity remains unrecovered after two Library title searches; root confirms none known. Genuine SuperJ6/SynthOne backend admission and all-page/native gates remain incomplete.
- Sound supplied immutable source interface e6c76e3b470d61b1c2f9e2a884efd6be18fa9373/PR952:64 real SuperJ6 descriptors,150 SynthOne controls, standalone888B SJ6 blob. All SJ6 bindings NOT_ADMITTED; no32B-cell activation or generic-engine relabel.

- GitHub safety review rejected generated native R024 PNG blob publication under the older raw-capture exclusion. No bypass attempted. PNGs are retained for Library pairing; unaffected source/tests/logs are preserved in this push. Public capture upload remains unresolved pending direct approval-context reconciliation.
