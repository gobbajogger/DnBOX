# FWUI render and route audit

## BREAK SEQ loses persistent per-step metadata

Start at HOME, turn the part encoder to BREAK and press SEQ. With playback stopped and step 16 set, assign slice 3 and add a BREAK parameter lock to that step. The host comparison enters through real panel controls and seeds the controlled hit, slice and lock data. Under shipping 0.0.26 source `94197fc8`, the assignment changes 39 body pixels and the added lock changes nine. Under assigned source `b995fb73`, both changes affect zero body pixels. The action route still shows a temporary slice toast, and BREAK SOUND retains its persistent metadata footer. Neither substitutes for observing all 16 assignments and locks from SEQ. #987 source `199f0444` is identical across the participating firmware files. This is independent of the already reported rest playhead.

The selected original [162-frame GIF samples](break-r22-original-frame-samples.png) show a dedicated assignment band and an independent hit/phase strip below the step indices. Its [assignment band](break-r22-original-footer-rows.png) shows unassigned dashes in every frame. The waveform slice numbers belong to the displayed waveform window. The exact shipping populated replay proves the assigned label and p-lock pixels.

The narrow geometry candidate aligns indices, assignment labels and locks to the same 16 existing slider columns. Keep the value strip at y227–239. Use y211–217 for step indices and y218–226 for assignment labels, subject to the native font’s measured glyph bounds. Keep amber locks at y238–239 and the playhead/value separation. Retain existing assignment values and contrast. Before adoption check two-digit labels, all three BREAK SEQ pages, theme contrast and independent playhead, slider and lock changes.

- [Shipping painter](https://github.com/gobbajogger/DnBOX/blob/94197fc839b00aff47b98ca02f3c6396e4a6eabf/firmware/src/app/ui.c#L5869-L5882)
- [Current BREAK footer](https://github.com/gobbajogger/DnBOX/blob/199f0444a4cf27374112a5fdf48f82005b070bbc/firmware/src/app/ui_break_layout_main.h#L88-L104)
- [Current SEQ lane](https://github.com/gobbajogger/DnBOX/blob/199f0444a4cf27374112a5fdf48f82005b070bbc/firmware/src/app/ui_step_lane.inc#L138-L171)

## Drum mixer owner reference differs on the same route

The [909 comparison](mix-909-authority-history-shipping-current.png) shows the owner crop, the selected GIF’s opening frame, exact 0.0.26 host and exact assigned source in that order. The shared reference selection explicitly applies the owner PNG to both 909 and 808 drum MIX and retains their eleven-channel routing. The owner frame uses voice glyphs, mute strikes, thin markers and per-channel numerical dB values. The current renderer uses text voice tabs and filled track bars, and omits those per-channel numerical values. The shipping body is identical to current. This is a confirmed design-conformance gap. The current selected BD still shows a K5 LEVEL value of -3.8 dB, which alone cannot establish the named G-FINAL binding.

## Scope and reuse

The [catalog index](catalog-reuse-index.json) verifies every indexed 240×240 decoded raster against its original RGB digest. All 353 Step 1 groups have a current catalog member. The 353 number includes distinct renderers and actual route groups together with contextual overlays, conditional fixture branches and historical exclusions. It is therefore a conservative packaging key, not a count of standalone user screens. Preserve each route’s evidence kind and compile variant. Batch multiple captures in one GIF only for the same documented renderer and real ordered state transition. Repeated values, rows, selected indices and fixture endpoints can share a GIF when they remain the same route and renderer, with each witness retained in the index. A valid isolated still can supply the full-page state of a proven static renderer.

- HELP and LIST share renderer families. Hold-button and category/action ingress remain independently indexed, as do modal question producers and confirm/cancel outcomes.
- Clip host waiting and Clip grid are separate. Native startup, recovery, OTA/update safety and calibration use separate route contracts even where neighboring frames share a painter or raster.
- Pad Colours and stock calibration remain separate. Pad Colours is a developer option and must carry its variant and SMC restore evidence.
- Battery fill is one component under separate host-header compositions. SMC pickup, playhead and per-step metadata require their own temporal or state witness. Exact catalog PNGs serve as matched static evidence for the captured states.
