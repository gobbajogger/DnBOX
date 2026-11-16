# Lane: fwui
Owner: Dot firmware UI lane. Branch: lane/fwui-r22. Base: e742be7bd0d01323c899de6e156f40b85ee451db. Target: integration/dnbox-consolidated.

## Checklist
All C1-C6 and F1-F7 remain FAIL (not yet qualified at this lane head).

## Now / next
- Now: F3 code-space fit and F2 real-render geometry, reusing existing consolidated UI/resource changes.
- Next: source-pinned baseline measurement, smallest safe correction, independent test-and-merge review.

## Blockers and routes
- Repo-prep v2 is not yet merged: consume attached v2 AGENTS/RULES/CHECKLISTS and design inputs without modifying prep-owned files; merge consolidated when available.
- Local Git store has missing ancestor objects despite a complete e742be7b checkout: verify current tree and obtain ancestry through connector; do not invent history.
- Current first-slice detector requires explicit --render-dir and --output; a bare invocation is not all-screen evidence. Capture the actual exact-source renderer closure before evaluating.

## Top 3 risks
1. Existing UI does not fit native code space with all engines.
2. First-slice detector does not cover all screen domains.
3. Shared ui.c changes require direct coordination with sound/integration.

## Deletes
None.

## Conflicts
Historical briefs are evidence only. RULES.md v2 from owner-provided dnbox-repo-prep.zip governs this lane; no inherited historical approval is treated as current instruction.

## Verified partial progress (not checklist acceptance)
- Pan formatter changed from L64/C/R63 to L50/C/R50; engine values and audio panning unchanged. Production host regression checks all 128 values across MIX and 22 drum voices, 8925 assertions. Before: endpoint assertion fails; after: PASS. Battery icon and remaining F4 checks remain unqualified.
- Exact e742 drum first-slice attempt failed before rendering: checked-in instrument.py requires absent firmware/src/app/ui_drum_voice.inc, while current renderer is inline draw_drum in ui.c. No107-defect count transferred from historical source. Next route: extend matching current production host fixtures; do not reconstruct candidate source merely to satisfy stale instrumentation.
- Native resource baseline underway under independent test-and-merge ownership. Full gate, hardware, independent review and all-screen geometry NOT_RUN at this head.

## Ordinary-page baseline inventory
- Source4bacd2e7: actual production host rendered135 ordinary page states at240x240. Harness, SHA256 manifest and all135 PNGs committed. Upload authorization rejection resolved by verifying exact files and citing owner rule13; no all-screen qualification claimed. Alternate engine selections, popups, transients and geometry remain NOT_RUN.
- First inventory compile used nonexistent ui_force; corrected by invalidating existing blit_hash/cv_tile_valid caches. Corrected run exit0,135screens.
- Independent pan regression rerun PASS8925. Native baseline e742 Elite/Pro XIP free13464/13472B; pool margin172B versus512B requirement. Integ owns measured pool correction. No full Local PASS at lane head.

## Selective R22 candidate
- Restored shared R22 header and22 drum voice layouts/glyphs from preserved freeze9 overlay, excluding historical GLOBE deletions and compressed-font accessor regression. Generic synth pages and other domains remain unfinished; no design acceptance claimed.
- Actual45-screen drum detector: checks1-4 PASS, overlaps0. Other20 checks NOT_RUN; F2 not complete.20detector unit tests PASS, including current/historical radius anchors and fail-closed malformed cases.
- Native initial overflow1060B fixed by packing1650 authored millipixel coordinates into16-bit storage.1651 float-bit checks PASS; serial135-state rendered pixels identical before/after. Elite2220B/Pro2232B XIP free; BSS96560, pool margin172 still below512 until independently verified integ correction composes. Full Local not passed.
- Build/test generation must use isolated worktrees: concurrent host/native generators changed BREAK bank and produced5 false raster differences; failed result retained and serial135/135 replay verified. This was first occurrence, not a repeated error.
- Evidence: docs/evidence/fwui/r22-candidate/summary.json and logs. Next: independent exact-head review, pool-fix composition, battery/pad regression tests, then remaining all-domain geometry and real SynthOne/SuperJ6 bindings.

## Battery and preserved GLOBE coverage
- Production shared-header battery/cache regression:2892 assertions PASS across eight views and cacheable drum header, transitions0/1/2/3 and unknown/USB clipping. Simulator defaults stay full; test fixtures can vary host battery input. No firmware runtime code changed in this checkpoint.
- Existing home_globe_part regression PASS on selective R22 candidate. This is a preserved-subset test, not the full F5 pad/LED matrix.
- Pan production8925 assertions and battery2892 assertions support F4 content; final same-head independent replay/full lane gate remains pending, so no lane acceptance requested.

## Parameter-page sequencer repair
- Fixed F5 discrepancy:909/808 parameter-page white pads showed sequencer LEDs but only auditioned. Both parameter andSEQ pages now call the same drum_step path; black-key voice selection/audition and existing release ownership remain unchanged. BREAK already used its sequencer path.
- Expanded production pads/settings cases across all22drum voices, emitted parameter pages,16pads, BREAK parameter pages and closed/list/help contexts. Before:2160 toggle failures. After:7096 assertions PASS.
- Held-step-release regression PASS144 (both kits/voices and all four banks); existing HOME/GLOBE clear/copy regression PASS. Full host suite and all F5 playback/LED combinations remain pending; no fullF5 acceptance claimed.
- Exact logs:docs/evidence/fwui/parameter-pads/. No device operation occurred.

## Remote formatter handoff
- Exported exact current Chicago/Miami pref_value_of readouts at every descriptor raw value, pinned to production b6be480c and source-file SHA256. Per-engine JSON under700KB at docs/evidence/fwui/formatter/. Capture describes boot factory preset state only, explicitly NOT_ADMITTED as runtime qualification. SuperJ6/SynthOne generic-percent fallback is not exported.

## Gate fixture repair
- Independent baseline found clip-chord display fixture omitted font_bitmap_width while extracted production caps() now requires it. Fixture now extracts the unchanged production helper rather than reverting compressed-font behavior. Corrected actual run:7049 checks/7050 captured chord draws PASS. Evidence:r22-candidate/clip-chord-fixed.log. Production code unchanged by this repair.

## Independent review and thin-lane preview
- Independent reviewer replayed battery2892, coordinate1651, parameter pads7096 and clip-chord7049 checks successfully. Independent saved-trace evaluation confirms45drum states/checks1-4/zero overlaps, not a fresh whole-UI run or Review10/10.
- Final b6 native runtime rerun in isolated tree:Elite2444B/Pro2456B XIP free; BSS96560; pool margin172 before integ fix. These native inputs unchanged through62e71.
- Isolated F6 read-only adapter/renderer preview:1295 assertions PASS,20native240x240 captures. Uses existing step/motion values, preserves four knob controls by replacing only footer strip. NOT connected to production input/render path; no F6 delivery claim. BREAK velocity explicitly unavailable, not mislabeled Level.
- Owner decision pending for SEL+SELECT lane switch; no mapping added before answer. Integ owns authentic BREAK velocity support. Preview code and captures under docs/evidence/fwui/step-lane-preview/.

## Mixer formatter handoff
- Exported exact target5 LEVEL/REV/DLY/PAN raw0..127 values for Remote from pref_value_of, avoiding JavaScript approximations of fm_log2f rounding. Existing CUT descriptor is also retained as source data, not a newly invented UI control. All five part voices share this formatter. docs/evidence/fwui/formatter/mixer.json pins production65a240 and source SHA256; no runtime admission claim.

## Reachable inventory correction
- Corrected inventory harness:bare V_GLO is not a reachable global screen; replaced5 invalid bare setups with42 real GLOBAL index/section row states via production entry/list routing. Captured172 ordinary/global native states. Historical135 capture is retained, not relabeled.
- New source-derived inventory schema/pinned test blob at docs/evidence/fwui/inventory-routes/manifest.json. Four render archives retain all172 PNGs. coverageComplete=false:alternate engine pages, other overlays/transients and hardware clip/PADCAL remain NOT_RUN. No F1/F2 acceptance claimed.

## Global header correction
- Actual global inventory exposed old header path on O_LIST:missing common R22 battery/style. Expanded production battery test first failed on GLOBAL. R22 header now also handles list overlays; corrected2905 assertions PASS. Other modal/help and full all-screen visual checks remain open.
- Fresh172-state replay confirms only42 GLOBAL images changed;130 ordinary states pixel-identical. Both native builds pass XIP reserve:Elite2412/Pro2424B free; BSS96560; pool remains172-margin until integ fix composition. No full Local/Review acceptance.

## Source-derived F7 navigation research
- Global-header893 independently reviewed/replayed: all25 delta blobs verified, one-line runtime predicate change preserves pickup/help/clip exclusions, actual GLOBAL battery2905 checks PASS. Integ92bdd7e20fde6ad3fa161d85dcd6deb641e251c9 retains893 via normal second parent; combined native qualification pending, no consolidated merge.
- Source-backed category proposal retained at docs/evidence/fwui/f7-proposal/. Machine check covers every SynthOne ID0..149 once in25 functional categories; max4 four-control pages/category. All48 sequencer fields explicitly retained (relative offsets, octave-boost and note-enable). SuperJ6 eight audited eight-control groups split into two pages each. Existing SOUND engine/preset and deep-list value editing must remain reachable. No page/input design implemented; owner yes/no pending.
- Sound independently corrected global detuning and envelope keyboard-tracking grouping. Unit audit preserves upstream mistakes rather than propagating them: position136 repeats master makeup identifier but denotes wet makeup; delay cutoff tracking is a ratio despite native Hertz tag. These findings are metadata/provenance, not qualified formatter outputs. Remaining unit laws, live DSP binding, complete renders and full lane checks remain open.

## Deep-list inventory extension
- Actual SEL entry and list_move traversal capture500 selectable states across57 lists. Independent review requested upward rewind and complete row accounting; corrected harness asserts first selectable row, unique rows and expected count. Rerun PASS; all500 render bytes identical before/after coverage correction. Adds scoped source inventory, not complete F1/F2 qualification.
- Eight archives publish400 native240x240 renders. Archive09 was denied twice as allegedly exceeding700KB despite measured fullJSON447932bytes; no further retry or workaround. Archive10 not attempted. Both retained locally; manifest explicitly marks publication incomplete and distinguishes all image statuses. No secrets or personal screenshots.
- Help/confirmation/transients/alternate engines/hardware modes remain NOT_RUN. Audit also confirms old fallback topbar paths without four-level battery; targeted modal tests next, no F4 acceptance.

## Help and confirmation battery repair
- Actual held-SEL help route failed battery pixel checks on7ea37 unchanged893 runtime. Minimal common-header predicate now also permits help and confirmation while retaining pickup and hardware clip exclusions. Cached PART header rules and modal input behavior unchanged.
- Expanded production battery suite PASS2941 checks: all prior views and GLOBAL, genuine held help/release, ask/cancel, four battery levels. Logs/hashes at docs/evidence/fwui/modal-header/. Exact combined native and independent replay pending; no fullF4 or lane PASS. Pickup/clip/message coverage remains open.

## Pickup and clip status header repair
- Independent6fa2 replay PASS2941. Subsequent actual pickup status failed the same battery check; unified status renderer now retains PICK text, LP identity, clip INT/EXT from smk_ck_ext and metro phase, with common four-level R22 battery. Removed unreachable old header fallback, not its controls. Clip body/clock are test fixtures, never device claims.
- Expanded battery suite PASS5258, including all four battery levels, internal/external clock distinction and stable LP title pixels. Existing SMC restore/clip/wait/overlay suite PASS8238. Native own-source Elite/Pro XIP free4288/4300 B, BSS96544, unchanged pre-integ pool margin172. Exact combined native and independent source review pending.
- Five240x240 production-header captures plus logs/source hashes retained at docs/evidence/fwui/status-header/. SMC-specific popup/peek and transient notification bars still need battery coverage; no fullF4 acceptance. No F6/F7 design change.

## Pickup status regression correction and SMC battery
- Independent review found unmerged0ed44 removed pickup CHN/long-step-range and save text. Corrected those semantics: production helper supplies CHN or each16-step range; original save label/compact !/Q/* fallback restored for pickup/clip. Worst P16+49-64 width initially failed unchanged header spacing; pickup-only indicator positions now reserve45px for complete context without reducing font or clipping. LP/INT/EXT and record/metro status retained.
- SMC physical overlay and peek also failed battery checks. Both now reuse the same R22 battery renderer; context text is bounded and existing physical-control glyph/number moves left20px, with both peek status lines retained. Remote confirmed no firmware writer conflict.
- Producer battery/status suite PASS5306, including real fader-created peek/overlay, CHN/ranges, worst-font-width bound, save failure and four-level transitions. Existing full SMC restore/clip suite PASS8238. Actual native Elite/Pro free3676/3684B, BSS96544, pool172 before integ fix. Independent review/exact combined qualification pending; no F4/lane PASS.
- Historical deep-list manifest now explicitly records harnessCommit7ea37 separately from runtime893; it does not satisfy final one-SHA capture qualification. Denied archive09 remains unmodified/unretried; archive10 remains unattempted.

## Readable kick depth label
- Remote source audit identified still-cryptic P.Dpth/P.DEPTH presentation, prohibited by RULE18. Presentation now reads Depth/DEPTH consistently in generic lists/knobs and all three R22 caption cases. Native target0/voice0/slot4 descriptor P.Dpth, stored value and DG_BDSWEEP pitch-sweep glyph remain unchanged.
- Production fixture failed before alias and PASS11 checks after; captures all three909kick pages and selected deep-list row. Four240x240 renders plus exact mapping/source hashes at docs/evidence/fwui/readable-depth/. Independent review and native changed composition pending; no fullF1/lane PASS.

## Strengthened authentic caption gate
- Added explicit P.DEPTH rejection alongside P.DPTH; old detector failed the new negative case, corrected detector passes all20 unit tests and accepts readable DEPTH. This strengthens, never relaxes, RULE18 coverage.
- Re-evaluated unchanged genuine historical45screen traces with the stricter detector: checks1–3 PASS, check4 FAIL in3screens, overlaps0. Prior old-detector PASS evidence retained as historical, not silently relabeled.
- Fresh first-attempt b341 production runtime capture:45screens, checks1–4 PASS, overlaps0; checks5–24 remain NOT_RUN. Full trace/report/render log/source closure and45nativePNG retained in four small archives at docs/evidence/fwui/strict-caption/. No fullF1/F2/Review acceptance.

## Exact per-voice drum formatter export
- Added source-backed909/808 exports:24 voice/KIT tables,197 typed controls,21924 complete raw-domain readouts. Each file is under700KB and pins productionb341/source hashes plus boot state vector. Native and display names remain separate, including P.Dpth→Depth.
- Producer PASS421 nonmutation/descriptor assertions; parser verifies complete contiguous domains and unique typed IDs. Existing Chicago/Miami/Mixer control tables are unchanged. Empty source units stay empty/raw, never fabricated percentages. Current drum formatter code is identity/raw-based; alternate-mode audio behavior and physical-device equivalence are not qualified.
- Data/index/logs at docs/evidence/fwui/drum-formatters/ for Remote typed adapter. No runtime production change in this checkpoint; no full lane acceptance.

## Exact native maps and preserved input regressions
- Fresh b341 native Elite/Pro builds: XIP free3544/3552B, BSS96544; pool margin172 remains below512 until integ fix composition. All312 tracked firmware/tools/assets/build.sh files match exact remote tree. External compiler/linker/plugin/SDK boot hashes recorded.
- Actual linker maps committed with receipts proving same ELF bytes as normal builds after adding only -Map to effective SMK link. First wrapper attempt missed build.sh board default; its wrong-target maps/receipts were detected, preserved INVALID, never admitted. Corrected wrapper forces board/model and rejects unexpected output before link; new negative regression fails old wrapper and PASS3 corrected checks.
- Current same-runtime pan8925, held-step144, pads7096 and HOME/GLOBE regressions pass. Evidence docs/evidence/fwui/native-b341/. F3 candidate evidence awaits independent review; no full Local, combined source, device or lane acceptance.

## Exhaustive pan identity regression
- Expanded the production pan formatter test from one mixer voice to all five, and enumerated all 197 declared 909/808 voice/KIT descriptors rather than a guessed parameter limit. Exactly 22 drum voice pans and zero KIT pans; all 128 raw values retain L50/C/R50 endpoints and monotonic sides. Stored values remain unchanged.
- Current b341 runtime PASS10,975 checks. This is formatter-domain coverage, not every rendered screen or complete F4 battery acceptance. Runtime source unchanged; evidence docs/evidence/fwui/pan-exhaustive/.

## Repeated execution-path correction
- Before file setup or tests, verify the execution working directory and use absolute paths for cross-directory writes; run with fail-fast shell setup so a failed edit cannot fall through to a stale test.
- Two incorrect doubled worktree prefixes were caught before source writes. The first ran the unchanged old pan fixture; the second failed to compile the absent toast probe. Both attempts are retained distinctly; neither is new-product evidence. Both schedule prompts received this correction through the coordinator.

## Transient toast battery failure retained
- Genuine b341 runtime probe confirms missing R22 battery on SAVED, NEXT pattern cue, actual host ABOUT and a clearly labeled synthetic long diagnostic: all four FAIL, 36 of 36 expected battery fill pixels absent. Existing 57-character stress diagnostic has small-font width317px and clips on240px; no scrolling behavior implemented before owner design approval.
- Production and stored message text unchanged. Negative fixture returns exit1, never a passing acceptance test. Before renders/logs and invalid setup attempt are retained at docs/evidence/fwui/toast-before/.

## Source-derived renderer dispatch coverage
- Isolated copied b341 production source now has an observation-only probe for14 main-body render functions and5 exceptional dispatch branches. Existing ordinary/global construction plus actual help/ask/SMC/readout/wait/transfer paths observed17 of19 categories. Clip-grid and PADCAL actual renderer coverage remains NOT_RUN because this host fixture substitutes them. Counts are function entries, not unique screens or visual acceptance; coverageComplete=false.
- Six instrumentation tests PASS: only counters added, missing/duplicate function or branch/include anchors rejected, unrelated branches untouched. First attempt omitted original-engine experiments and failed before compilation; preserved failure and corrected full input manifests/logs are retained in docs/evidence/fwui/dispatch-coverage/attempts.zip. No production changes, screenshots, native-fit delta or complete F1/F2 claim.

## Reviewed corrections and authentic clip-grid samples
- Independent test-and-merge accepted cde93 source-loss repair: battery5306 and SMC8238 checks PASS at exact source hashes. This closes pickup/chain/range/save loss in bounded host scope, not fullF4. Independently verified all102 changed blobs6fa→8a5 and stricter caption20/20 suite; native identity/DG_BDSWEEP/value retained by Depth alias. Integ composed full8a5 atf910d92 with its explicit-origin hooks preserved; exact combined native/host remains under reviewer ownership.
- New fixture extracts unchanged actual main_fm1 clip-grid function, uses real gfx/palette/map helpers and explicit feedback state. Five native240x240 samples: empty, populated, queued, second viewport and compact offset. Final1131 assertions PASS for bounds/center semantic pixels/unchanged source state/signature-rate behavior. Initial422-check run and all unchanged image hashes retained. No device access or substituted grid claim. Clip-key subpages/PADCAL remain NOT_RUN; existing explanatory footer is not R22 acceptance. Evidence docs/evidence/fwui/actual-clip-grid/.
- Independently reviewed Remote canonical isolated glyph sweep:63 controls/7434 frames, exact dependency/idle contexts, all gzip/raw/frame hashes and computed bounds/differences agree. Only factory-depth-zero BD Pitch is constant, source-correct. Fixed-scale crop evidence is not full-layout, temporal/easing, audio or browser acceptance. Review receipt at docs/evidence/fwui/remote-glyph-review/.

## Actual RAM-only pad-calibration renderer
- Extracted unchanged smk_padcal_draw and production helpers into a hardware-free host fixture. Actual GLOBAL > SYSTEM action establishes header context; the host enter hook remains explicitly substituted and only renderer RAM is initialized. No device calibration or pad writes.
- PASS34826 assertions across513 states:8pages x16 selected pads x real/boost x marked/unmarked plus orientation. Checks actual palette/marker/frame pixels, bounds, state nonmutation and redraw caching. Nine sample captures, not every state render or full R22/visual acceptance. Initial generic HOME-context body run retained as body-only; all nine body rasters identical after header-context correction. Evidence docs/evidence/fwui/padcal-render/.
- Reviewer measured exact integ f910 composition: Elite3064/Pro3072B XIP free, RAM1096B free, pool margin836B; battery5306, scalar3970 and events23 PASS. This is that composition only, not full Local or lane acceptance.

## Actual clip-key graphics and review correction
- Replaced only isolated test drawing stubs with production gfx/fonts/rounded-shape helpers and unchanged CK render functions.17 SCALE/CHORD/ARP selected-field states plus actual bottom strip rendered, with state nonmutation/nonempty ink/selection-change assertions. These are240x138 body and240x82 strip, never mislabeled full240x240. All18 raster buffers identical on reproducible and corrected runs. No device I/O, full visual acceptance or input routing claim.
- Reviewer requested fail-closed extraction anchors and broader provenance. Added unique-anchor checks; eight duplicate-anchor negatives reject before generated host source. Provenance binds generator/prep/render fixture/extractor/fake USB and generated font/includes. First missing blend helper compile failure retained; fixed with actual unchanged helper.
- Separate text observer found268 rendered text calls with0 opaque glyph pixels outside canvas and identical18 buffers; not a whole overlap/style gate. Existing explanatory text and full glyph/design conformance remain open. Evidence docs/evidence/fwui/clip-keys-render/.

## Reviewed clip-key instruction-text removal
- Independent source review accepted corrected clip-key fixture publication atbd11827; prior clip/PADCAL fixtures independently replayed1131/34826 checks onf910.
- Removed only two explanatory text_c calls from ck_page_draw under RULE21. Fields/values/selection/keybed/input code unchanged. New source test FAIL before/PASS after; all17 actual body rasters differ only in instruction rows64-73/124-135, bottom strip identical. Knob/USB/BT input regression PASS0fail; chord display7049 checks/7050 draws PASS. Independent reviewer accepted exact two-call delta; combined native/full visual qualification remains pending. Evidence docs/evidence/fwui/clip-text/.
- Keep generated Python expressions spaced and validate helper syntax before long test/artifact pipelines; a parse failure is not product evidence. Two whitespace-related helper parse errors occurred before actions; corrected runs are the evidence. This corrective line is also saved in both schedule prompts.

## Startup instructional footer removed
- Independent review accepted the one-line removal of startup-only HOLD ANY BUTTON explanatory text under RULE21. Null initialization/return color and all context/help/input branches remain unchanged. Actual footer regression FAIL before/PASS after; existing battery/header/help5306 checks PASS. No generic full-render or full Local claim. This joins reviewed695 clip-key text cleanup for one next combined qualification, avoiding repeated native micro-builds.

## Clip-key raw-domain text geometry
- Test-only fixed-context sweep uses actual setters/readback/rendering for all153 legal values across17 page-fields.1914 text calls show zero canvas-clipped opaque pixels, conservative label/value bounding overlap or120-pixel column overruns; injected negative geometry triggers all3 detector counters. Every raw starts from the same explicit idle CK/settings state. Drawing nonmutation holds; observer leaves18 baseline rasters byte-identical. ASan+UBSan also passed. Not Cartesian combinations, live-note states, full-screen visual acceptance or any checklist PASS.
- Review requested final instrumented-input provenance rather than original setup-only receipt. Added separate final domain receipt for observer/C/sh/generated includes/fonts and compile command; corrected rerun passed. Runtime remains ef70, frozen for integ combined qualification.

## Actual chord graphics replay
- Supplemented old no-op shape/text fixture with production gfx/fonts/rounded primitives while retaining original7049 lifecycle/oracle checks and7050 chord draws. Actual text passthrough checks opaque font pixels within strip canvas;4096 pitch-class masks and generated chord/inversion/arp cases pass. Existing hardware transport mocks preserved. ASan+UBSan replay passes;4 duplicate-anchor negatives fail before output mutation. Final instrumented provenance/compile command separately bound. No production change or full F1/F2/visual acceptance.

## Actual clip strip matrix and review stability
- Extended observer before text_c so centered stage-chip text is also checked; all18 baseline rasters remain byte-identical and7049 chord checks/7050 draws pass. New actual strip matrix1977 states covers153 individual raw values,576 scale combinations and1248 arp combinations with maximum auxiliary settings; state/transport nonmutation, nonempty raster and redraw cache pass. ASan+UBSan1977-state replay passes. Still not full combinations, live input or full visual acceptance.
- An observed-baseline compile failed on missing assert include; explicit include repaired it and successful affected rerun is retained alongside failure.
- Two review handoffs compared older receipts against evolving prep inputs. Freeze preparation/source inputs through the final capture and review; verify receipt hashes match the exact frozen files before claiming evidence applicability. Final combined sources were copied immutably and all source/generated hashes checked after rerun. Root confirmed both schedule prompts carry this correction.
- Rule6 deviation: published85052c1 anddcc9f5e subjects omitted the next-deliverable suffix. Preserve history without force/amend. Before creating a DnBOX commit, validate its subject matches <type>(<lane>): <what changed> | next: <next deliverable> and include the lane-file update in that same commit. Root confirmed this corrective line is in both schedules.

## Logical screen text-ink bounds
- Isolated actual font-decode observer on172 ordinary/global and500 deep-list records reports0 ink samples outside logical240x240. Calls10467/17564 and decoded samples1427082/2486373 include setup/band replays; not unique pixels. Explicit clip calls235/557 excluded. Existing inventory assertions remain, unknown canvases/source layout changes fail, arithmetic edge controls and source-anchor negatives pass. Reproducer generates no PNGs.
- Initial canvas-only observer flagged fully off-band replayed text. Preserved exploratory logs; corrected mapping uses actual header/main/knob origins0/20/158 and all672 exploratory PNGs remain byte-identical. This fixes harness interpretation, not a product defect. No band-union visibility/overlap/full-inventory or fullF1/F2 acceptance claimed; captures remain local and no denied artifact action was retried.

## Actual canvas origin binding
- Extended isolated bounds observer to validate actual cv_commit/cv_commit_rows/full-with-tiles coordinates and reject replacement of a text-bearing canvas before commit.172/500 affected producers reran with690/1671 origin checks and unchanged0 logical out-of-screen samples. Four unit tests include missing/duplicate actual commit-route mutations. Independent review verified frozen bf9deb55 source/report hashes and reran instrumentation tests. No production change or band-union/overlap/fullF2 acceptance.

## Native drum Attack contract6 regression
- Actual six visible909/808 typed Attack controls ×four current scale contexts ×settled min/max:48 cases,0 failures,357 checks. Native polyline rising-span/32-unit viewbox meets min≤10% and max≥40%; unchanged AA renderer generates actual pixels. Idle/state nonmutation and real page identity asserted.
- Authenticated f254 Git blob7b573423 supplies only unchanged historical Attack block into same current host;48 cases/24 minimum-span failures. Original input manifests must match; expected-only mutations and exact hook hashes verified again after execution. Reviewer requested these runner guards, then affected two-case rerun retained96 byte-identical rasters. Independent source/hash/threshold review, four extraction tests and two equivalent PNG-pair visual inspections accepted bounded evidence.
- Distinct generated64x48 regression PNGs and manifests published under docs/evidence/fwui/attack-shape/. No denied deep-list/Remote artifact retried. Not whole historical firmware, full-screen/all-engine/live/check5 or fullF2 acceptance.

## SONG normal-body instruction removal
- Removed only the explanatory suffix from EMPTY: WHITE KEYS ADD BARS, preserving EMPTY status, END marker, placement, font, color, selection and all input/transport code. Help-card design question remains reserved to the User.
- Actual production graphics fixture passes 12 fixed length/selection/band states and 86 checks: extracted renderer pixels equal actual draw_song, prior-literal composition differs only on empty state, END/nonempty and other band pixels identical, song/UI/transport state unchanged. Before source assertion correctly rejects the old explanatory literal; this is not represented as a baseline runtime failure.
- Independent test-and-merge verified frozen source/test hashes and exact literal-only diff versus ef70, accepted scoped source evidence. It did not independently rerun the full generator; producer logs are under docs/evidence/fwui/song-status-text.
- Consolidated ref rechecked e742be7bd0d01323c899de6e156f40b85ee451db, unchanged; no merge/deletion. Runtime literal changed, so prior combined native XIP3240/3252B is not a new-head PASS. Integration owns exact recomposition/native measurement. No allocation or executable statement change. All full checklist items remain unqualified.
- Contrast triage: 172-state actual opaque-pixel scan initially had 1,428 candidates; exact four decorative call-site classification accounts for 1,410, leaving 18 sampled face/value calls. Same-color actual label positive controls prove classification is not a color exclusion. Remaining frequency/value and EMPTY status contrast must be checked against final visible layout; no palette change, no check20 PASS. Exploratory source/logs retained locally, not qualifying complete inventory.
- Next: complete source-bound contrast observer and remaining actual frequency/value candidate analysis.
