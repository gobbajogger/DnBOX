# BREAK SEQ rest playhead — 0.0.26/reference checkpoint (unmatched)

**Status:** WORKING; this is an evidenced first-area checkpoint, **not** the final selected GIF set, a fresh host reproduction, a firmware fix, or a full-page visual PASS.

## Source identity and selection

- Source branch: `lane/fwui-verified-regressions-resume` at `f1786ebec2616477163b62b029c7e77405a9c3de`; actual production firmware in merged [PR #987](https://github.com/gobbajogger/DnBOX/pull/987): `199f0444a4cf27374112a5fdf48f82005b070bbc`.
- Shipping **0.0.26 source comparator**: [`firmware/src/app/ui.c` lines 5836–5883](https://github.com/gobbajogger/DnBOX/blob/94197fc839b00aff47b98ca02f3c6396e4a6eabf/firmware/src/app/ui.c#L5836-L5883), exact file blob `eb911f58db83e417c67dd71576331803fa6c829e`. This is source evidence, **not** a freshly rendered 0.0.26 image.
- Reference authority: [FWUI implementation and Step 1 route map](https://chatgpt.com/space/page_800ecfd4f2a08191884949bf6bbee920) and [UI reference owner’s full-page comparisons](https://chatgpt.com/space/page_0b5b928f86108191a193b316a3f4b1d9). The Step 1 complete screen-and-ingress map must be reused, not repeated. The owner’s reference package/Step 2 binary is not inspectable in this checkout.
- **First unmatched route:** BREAK → SEQ → empty 16-hit pattern → playback at 1/16 → no overlay → observe rest position on each of Velocity, Chance, Pitch and Pan. **Full-page reference GIF selection remains UNMATCHED** because the authenticated original 0.0.26 and chosen R22 full-page motion pixels could not be opened for a defensible comparison.

## Verified source difference (not a new binary run)

In 0.0.26, `ph = seq.playing ? eng_step[TRK_BRK] : -1` and each of the sixteen BREAK cells is painted with `c == ph` **even when `on` is false**; a rest at the active position is distinguished from the other rests. In the current [BREAK layout footer](https://github.com/gobbajogger/DnBOX/blob/f1786ebec2616477163b62b029c7e77405a9c3de/firmware/src/app/ui_break_layout_main.h#L90-L110) the footer explicitly skips the phase-sensitive bottom row in SEQ (`if(ui.view==V_SEQ)continue`). The replacement [step-lane renderer](https://github.com/gobbajogger/DnBOX/blob/f1786ebec2616477163b62b029c7e77405a9c3de/firmware/src/app/ui_step_lane.inc#L138-L171) paints values and `notes` but reads no `eng_step` transport phase. It therefore cannot reproduce a phase-highlighted **empty rest**. This is consistent with the already posted [63-check host defect report](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7065118335) on prior input `ac63b270`; **do not report it again as a new defect**. The exact resumed branch has not been newly executed.

The desired patch must preserve the R22 four-lane value strip, sixteen real steps, slice assignments, locks, and its existing space. Do not simply restore the older incompatible footer or overwrite lock pixels. An exact-head red/green framebuffer test must demonstrate moving rest-phase contrast for all four lanes without breaking selected values or static layout.

## Historical GIF candidates, not approved revision references

The separately indexed archive `DnBOX-975-all-GIFs.zip` is **14,222,261 bytes**, SHA-256 `2c35210da618125088494bf6aac1c1fd21b855970d7cb0fc337df7af4e6e8e2f`; ZIP member groups total **1,880** readable 240×240 GIFs. Its reported capture commit is `61827a71`, not original 0.0.26 and not production `199f0444`. Measurements are in [archive-candidates.json](archive-candidates.json).

Keep these visual mechanisms separate in the eventual single canonical set:

| Independent renderer | Actual older archive candidate | Verified measurement | Revision-matched selection |
|---|---|---|---|
| Battery header | `06-renderer-motion/checklist-battery.gif` and four `battery-level-*.gif` stills | Five animation frames; four one-frame levels | **Unmatched** |
| Playhead / non-BREAK | `06-renderer-motion/checklist-play--2.gif` | Eleven frames; bass A, **not** BREAK rest cue | **Unmatched** |
| SMC pickup / SEQ | `07-smc-pickup/seq.gif` | Seventeen frames; independent overlay family | **Unmatched** |
| BREAK SEQ rests | No positively identified matching BREAK phase-motion GIF in the historical renderer-motion group | 0 qualifying original/full-page source matches | **Unmatched** |

These are candidates for role selection, not accepted reference-versus-current comparisons. The final GIF set and canonical coverage/findings index must be published **together**, only after all distinct routes and sources are qualified.

## Reuse and remaining gates

[Pad Colours after SMC expiry](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7065109373) is a **second already-posted** verified defect and not evidence for this BREAK GIF. No duplicate PR comments were added. This checkpoint made **no firmware changes**, no claimed `.fwsc` or `.img` rebuild, and no release or device writes.

Archived ZIP SHA, group counts, candidate GIF dimensions/frame counts and original/current source text were checked. **NOT_RUN:** current exact-head host RED/GREEN, 0.0.26 native render, current 199f full-page render, Elite/Pro builds, four-size independent visual review. Build and current source/SDK environment are not present on this worker; the source checkout and revision-specific reference payload must be restored before code qualification. The checksum note for AC79 SDK is not the SDK archive.

**Next:** recover original/reference inputs and current build tree, capture 0.0.26 and production BREAK rest-phase sequences under the same physical route, add a regression and minimal verified fix, run focused host tests and stock Elite/Pro builds, then continue full-page role matching.