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
