# FWUI reference and qualification index

The [coverage Page](https://chatgpt.com/space/page_800ecfd4f2a08191884949bf6bbee920) and [canonical map](canonical-input-index.json) cover 353 screen and renderer groups and 1,944 mapped roles. The [production catalog check](catalog-reuse-index.json) verifies 1,968 native 240×240 RGB captures. The full-page archive retains that catalog, the selected reference animations and paired motion evidence for distinct routes. Catalog GIFs are lossless one-frame copies, and motion GIFs retain the order of actual renderer output at the stated capture cadence.

Shipping behavior comes from `94197fc839b00aff47b98ca02f3c6396e4a6eabf` (0.0.26). The mapped production source is `199f0444a4cf27374112a5fdf48f82005b070bbc`. The assigned baseline is `b995fb7374275e83a345fee45b7afc22d8206970`. Unchanged captures retain their original source pins. The branch changes BREAK cues and an optional developer Pad Colours compositor handoff.

| Area | Evidence |
| --- | --- |
| BREAK SEQ rest cue | [Shipping, baseline and corrected stills and motion](../break-seq/README.md). All four corrected lanes change at 15 of 15 consecutive playhead transitions. |
| BREAK SEQ slice assignments and locks | [Full-page captures](../break-seq/metadata). The corrected populated-step replay changes 36 assignment pixels and six lock pixels. The independent 24-state replay checks each step and two-digit assignments across pages, lanes and themes. [Finding on #987](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7072473672) |
| Pad Colours | [Paired 0.0.26, baseline and corrected renders](../pad-colours/README.md). All 138 body rows restore on timeout and stay restored at +240 ms. |
| Battery and SMC pickup | Battery has four levels and a verified exact match to the historical icon footprint in all 120 sampled icon states. SMC comparison covers pickup both ways, locked owner/peek, HOME and mixer banks, Settings statuses, Clip and Fill. HOME Live did not exist in 0.0.26. The Clip host body is a stub, with separate native evidence. Step 1 proves queued Fill motion. |
| 909/808 | Exact-source SOUND/KIT, SEQ, MIX and separate LIST witnesses retain physical primary and alternate entry. The two 909 AMP pages inherit Decay semantics from 0.0.26. Some LIST roles use production or host renders as their only visual witness. |
| Shared, bass, startup and native routes | Paired HOME, GLOBE, SONG, PERF, FX, overlays, both bass slots and their engine pages. Native startup has 42 paired mocked-board scenarios, plus dedicated normal-loop proofs. External-clock waits and dropout have separate LCD and LED receipts. |
| Remaining R22 MIX layout | Both current and shipping use the legacy mixer picture. The selected owner reference uses voice glyphs, mute strikes, thin faders and numeric channel levels. The full detached candidate passes 11,624 host checks across both kits and palettes. Its best fit leaves 940 B on Elite and 948 B on Pro, which miss the 1,024 B floor by 84 B and 76 B. [Candidate and stock receipts](mix-blocker/). See the [guideline audit](extended-guideline-audit.md) and [finding on #987](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7072487906). |

Identical source and archived RGB reuse establishes the corresponding visual witness. A historical candidate, static witness or version-only route is recorded as such. Some role-matched visual references are absent in the authority set, including portions of native startup and LIST. Those entries retain the closest verified route evidence and their original coverage status.

## F1–F7 cross-check

| Check | Result and evidence |
| --- | --- |
| F1 screens and shared R22 treatment | **Open.** The canonical index covers all mapped roles. The stock drum MIX design discrepancy and remaining reference gaps are recorded above. |
| F2 geometry and visible text | **Pass in the exercised domains.** The [reached renderer audit](geometry-reached-baseline.json) found zero geometry flags in 1,538 states, 44,082 text audits, zero missing of 19,643 presence checks and zero missing or damaged of 249 glyph-pixel checks. The [independent final BREAK delta review](break-native-review.md) adds 5,602 checks and [72 exact viewport crops](break-viewport-review.json). [Pad Colours independent review](pad-colours-independent-review.json) adds 45 exact viewport crops. |
| F3 stock fit | **Pass.** [Elite and Pro packages, hashes and build receipts](stock-builds/) retain 1,100 B and 1,108 B of XIP free, with 832 B of pool margin each. |
| F4 Pan and battery | **Pass in the mapped replay.** The scoped suite checks the L50/C/R50 formatter and battery layouts. The paired battery replay records four levels, deferred Settings states and exact icon pixels on normal and SMC top bars. |
| F5 pads, LEDs and hold-GLOBE | **Pass in host and stock-emulator coverage.** Drum route/held-step receipts, the scoped hold-GLOBE suite and the [stock BREAK playhead GIFs and input receipts](stock-break-motion/) cover the mapped operation and feedback. |
| F6 continuous strip and mode selection | **Pass.** The paired [BREAK lane motion](../break-seq/README.md), drum captures and lane edit/transaction suites verify Velocity, Chance, Pitch and Pan and the physical lane selection path. |
| F7 live controls and synth routes | **Pass in the exercised domains.** The scoped synth integrations and owner-unit/shared-glyph tests cover both bass slots and the SuperJ6 and Synth One routes. The reached text scan flagged zero known instructional strings. |

The [scoped F1–F7 host log](f1-f7.txt) and the exact build artifacts identify the tested source. The [shipping erased-sector recovery disposition](shipping-recovery-disposition.md) records the pre-frame 0.0.26 crash and the independently verified current repair already present in the assigned baseline.

The scoped runner completed 23 named checks with zero failures. The runner also emitted its separate package-harness skip because its host work directory lacked SDK inputs. Both stock packages were built and verified in the isolated stock tree. The complete Project Sources archive contains 2,502 240×240 GIFs including the 1,968 lossless production catalog frames. Archive SHA-256 is `2cfd31090db10b582326756c5ede6c4fb10d139f5ef37383c86383179ec1cf2a`.
