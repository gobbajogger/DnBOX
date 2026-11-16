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
