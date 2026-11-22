# FWUI reference and qualification index

The [coverage Page](https://chatgpt.com/space/page_800ecfd4f2a08191884949bf6bbee920) and [canonical map](canonical-input-index.json) cover 353 screen and renderer groups and 1,944 mapped roles. The [production catalog check](catalog-reuse-index.json) verifies 1,968 native 240×240 RGB captures. The full-page archive retains that catalog, the selected reference animations and paired motion evidence for distinct routes. Catalog GIFs are lossless one-frame copies, and motion GIFs retain the order of actual renderer output at the stated capture cadence.

Shipping behavior comes from `94197fc839b00aff47b98ca02f3c6396e4a6eabf` (0.0.26). The mapped production source is `199f0444a4cf27374112a5fdf48f82005b070bbc`. The assigned baseline is `b995fb7374275e83a345fee45b7afc22d8206970`. Unchanged captures retain their original source pins. The branch changes BREAK cues, the optional developer Pad Colours compositor handoff and the targeted MIX and FX readouts documented below.

| Area | Evidence |
| --- | --- |
| BREAK SEQ rest cue | [Shipping, baseline and corrected stills and motion](../break-seq/README.md). All four corrected lanes change at 15 of 15 consecutive playhead transitions. |
| BREAK SEQ slice assignments and locks | [Full-page captures](../break-seq/metadata). The corrected populated-step replay changes 36 assignment pixels and six lock pixels. The independent 24-state replay checks each step and two-digit assignments across pages, lanes and themes. [Finding on #987](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7072473672) |
| Pad Colours | [Paired 0.0.26, baseline and corrected renders](../pad-colours/README.md). All 138 body rows restore on timeout and stay restored at +240 ms. |
| Battery and SMC pickup | Battery has four levels and a verified exact match to the historical icon footprint in all 120 sampled icon states. SMC comparison covers pickup both ways, locked owner/peek, HOME and mixer banks, Settings statuses, Clip and Fill. HOME Live did not exist in 0.0.26. The Clip host body is a stub, with separate native evidence. Step 1 proves queued Fill motion. |
| 909/808 | Exact-source SOUND/KIT, SEQ, MIX and separate LIST witnesses retain physical primary and alternate entry. The two 909 AMP pages inherit Decay semantics from 0.0.26. Some LIST roles use production or host renders as their only visual witness. |
| Shared, bass, startup and native routes | Paired HOME, GLOBE, SONG, PERF, FX, overlays, both bass slots and their engine pages. Native startup has 42 paired mocked-board scenarios, plus dedicated normal-loop proofs. External-clock waits and dropout have separate LCD and LED receipts. |
| R22 drum MIX and shared readouts | Both drum kits now use voice glyphs, thin faders, numeric channel levels, filled LEVEL columns and an amber pending-mute cue. Direct mute retains its strike. FX omits the active bank's duplicate body readouts and retains sends, delay type and the other bank's context. Master and Wave retain Wet GR and the editable Pump row. [Final changed rows and receipts](r22-mix-stock/) · [Guideline findings](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7073299621). |

Identical source and archived RGB reuse establishes the corresponding visual witness. A historical candidate, static witness or version-only route is recorded as such. Some role-matched visual references are absent in the authority set, including portions of native startup and LIST. Those entries retain the closest verified route evidence and their original coverage status.

## F1–F7 cross-check

| Check | Result and evidence |
| --- | --- |
| F1 screens and shared R22 treatment | **Open.** The canonical index covers all mapped roles. The verified MIX and FX changes are recorded above. Bass AC/SL lanes remain while the on-note cue is open, and missing exact motion witnesses are still listed in the motion review. |
| F2 geometry and visible text | **Pass in the exercised domains.** The [reached renderer audit](geometry-reached-baseline.json) found zero geometry flags in 1,538 states, 44,082 text audits, zero missing of 19,643 presence checks and zero missing or damaged of 249 glyph-pixel checks. The [independent final BREAK delta review](break-native-review.md) adds 5,602 checks and [72 exact viewport crops](break-viewport-review.json). [Pad Colours independent review](pad-colours-independent-review.json) adds 45 exact viewport crops. |
| F3 stock fit | **Pass.** [Final Elite and Pro packages, hashes and build receipts](r22-mix-stock/) retain 1,116 B and 1,124 B of XIP free, with 832 B of pool margin each. |
| F4 Pan and battery | **Pass in the mapped replay.** The scoped suite checks the L50/C/R50 formatter and battery layouts. The paired battery replay records four levels, deferred Settings states and exact icon pixels on normal and SMC top bars. |
| F5 pads, LEDs and hold-GLOBE | **Pass in host and stock-emulator coverage.** Drum route/held-step receipts, the scoped hold-GLOBE suite and the [stock BREAK playhead GIFs and input receipts](stock-break-motion/) cover the mapped operation and feedback. |
| F6 continuous strip and mode selection | **Pass.** The paired [BREAK lane motion](../break-seq/README.md), drum captures and lane edit/transaction suites verify Velocity, Chance, Pitch and Pan and the physical lane selection path. |
| F7 live controls and synth routes | **Pass in the exercised domains.** The scoped synth integrations and owner-unit/shared-glyph tests cover both bass slots and the SuperJ6 and Synth One routes. The reached text scan flagged zero known instructional strings. |

The [scoped F1–F7 host log](f1-f7.txt) and the exact build artifacts identify the tested source. The [shipping erased-sector recovery disposition](shipping-recovery-disposition.md) records the pre-frame 0.0.26 crash and the independently verified current repair already present in the assigned baseline.

The scoped runner completed 23 named checks with zero failures. The runner also emitted its separate package-harness skip because its host work directory lacked SDK inputs. Both stock packages were built and verified in the isolated stock tree. The complete Project Sources archive contains 2,502 240×240 GIFs including the 1,968 lossless production catalog frames. Archive SHA-256 is `2cfd31090db10b582326756c5ede6c4fb10d139f5ef37383c86383179ec1cf2a`.

## Exact R22 MIX and readout update

The production renderer blob is `f7104b3f8a7a87d7f03daaa7845d7f513f9213f4`, based on `22a34d6ecef7b31e62ea6655fddc4920ab30f8e1`. The full archive carries 2,635 full-page GIFs and preserves the original 2,502 members that are GIFs. Its refreshed index names 90 affected mapped and range-edge states, 20 event snapshots, eight kit and palette witnesses, and 15 physical-route frames. The archive SHA-256 is `a7215fd637ce783d1fc3905afcf0f0552c827aedd208917444ecd3df6d4493c3`.

[Host results](r22-mix-stock/host-results.txt) include 11,624 mixer checks and the focused FX regressions. The timing replay passed 24 checks and [matches shipping for all 20 recorded event rows](r22-mix-stock/mute-events.txt). Both accepted and applied mute masks are recorded at the original engine sample indices. The renderer-local size change preserved all 28 mixer replay frames pixel for pixel. Stock links and package CRCs passed for both models. [Integration measurement](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7073368435).

Bass SEQ and GEN retain AC/SL so a held accent and a slide in visible column 15 remain readable. Shipping and the candidate share the held fill and final-column connector rules. Removing those lanes requires a verified replacement cue on the notes.
