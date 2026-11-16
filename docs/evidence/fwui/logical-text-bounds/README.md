# Logical full-screen text ink bounds

Run `python3 tests/ui_defects/text_bounds/run.py --source . --output /new/absolute/output`.

The observer operates inside the actual font decompressor, after opacity decode and before the unchanged clipping/painting statements. It maps the exact source strip origins20/138/82 high to screen y0/20/158, includes current cv_oy, and fails changed layout definitions or unknown canvas dimensions. Explicit non-full-width clip calls are excluded and counted. Existing ordinary/global and deep-list fixture coverage assertions remain; generated snapshots are disabled in the reproducible runner. Source copies, source-input manifest and post-instrumentation hashes bind the exact input set. This is a sampled source closure, not qualification of every file at a Git SHA.

Final producer observations:172 ordinary/global records,10467 text calls,1427082 decoded ink samples,0 samples outside logical240x240;500 deep-list records,17564 calls,2486373 decoded samples,0 outside. Explicit clip calls235/557 are excluded. Setup/interstitial frames also contribute calls; band replays duplicate ink, so these are samples, not unique pixels or only672 final frames. Six edge/band-coordinate arithmetic assertions and nonzero call/ink assertions run in each producer. Unit tests reject missing/duplicate graphics anchors and changed/duplicate layout definitions.

The first observer incorrectly treated each canvas band as the whole screen and reported fully off-band calls as clipping. Those exploratory logs are preserved, not product defects. Corrected exploratory runs preserve all172+500 PNG bytes exactly, while logical outside calls are zero. They remain local captures; this checkpoint publishes no PNG/raw buffers and does not bypass any denied artifact upload.

LIMITS: no proof of ink union/visibility across band passes, intentional explicit clipping, text/glyph overlaps, content-area bounds, every state, complete inventory, R22 style or visual10/10. F1/F2 remain unqualified. The first counters' false-positive interpretation was repaired, not hidden by relabeling band findings as product PASS.
