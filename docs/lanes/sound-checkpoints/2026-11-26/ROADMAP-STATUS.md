# Independent roadmap checks

Snapshot 2026-11-26. No product publication, real-device flash, or merges
performed in this check. The firmware sound/fit/dropout hard gate remains
open; see [current delivery](CURRENT-DELIVERY.md).

## Remote

[DnBOX-nightly #206](https://github.com/gobbajogger/DnBOX-nightly/pull/206)
was open against integration base 93fbcdea3c12630a449195c01af923955ee6c535,
head ac9001e01566fedebb0cc301f3a2090d079c3a06.
[Exact app and real firmware digital pad ingress host replay](remote-digital-pad-replay/README.md):
physical SMK queue pad index 8 produced firmware Voice part 2, note 41,
velocity 107. The actual PR JavaScript highlighted the key and cleared it
on release. Separate Position updates advanced and stopped the playhead;
the pad did not advance it. Node tests: 217 pass, 8 skipped, no failures.
Browser with real platform MIDI and native end-to-end transport remains
unverified. Existing Chromium wrapper has no executable. Do not conflate
an exact JavaScript DOM adapter with browser qualification.

The [separate STOP regression](remote-stop-lost-position/README.md)
drops an actual stop POSITION while delivering confirmed stopped
TRANSPORT. The pinned app leaves stale lane markers; the guarded fix
clears them without synthesizing POSITION or authoring any edits.
Disconnect also clears a held VOICE highlight in the same fixture.
This patch is preserved as source evidence, not pushed to the owner
lane or deployed. Full native/browser qualification remains open.

## Installer

[DnBOX-nightly #208](https://github.com/gobbajogger/DnBOX-nightly/pull/208)
was open, head fc2ca0489f22de00410037df46390712473493f2. Candidate installer
blob 80675789… is not on live gh-pages tip 83781eb0375340ada03b227f759c4d809d4e86f5
(active installer blob 71929d12…). Exact official v0.0.26 stable assets were
fully downloaded and compared byte-for-byte to the pinned Pages tree and
candidate catalog:

| Model | Bytes | SHA256 |
| --- | ---: | --- |
| Elite | 651732 | 95b3195f4767ef5df33b365197b01203873e5eec3d7fa497c73e5e509484a4a6 |
| Pro | 651730 | ce67d560eebd03cb8b9087f7723b1e0b71fb7cca256184cdfdd55a8c27c708b8 |

This verifies stable asset bytes, not deployment or real browser guard
behavior. The original backup preserved ADPCM zone identity, not WAV
filenames. The unverified labels are BR1 zone 1, BR1 zone 2, and BR2 zone 1.
The previously correlated labels Hot Pants (BR1 zone 3) and Tasty (BR2
zone 2) do not identify them. Keep the three positional names until
a trusted mapping exists; do not infer from waveform similarity.

## Firmware drafts

PRs #1010, #1012, #1013, #1014, #1016 are closed in GitHub and not marked
merged. Their main changes are present (with edits) on the directly checked
integration 73749f55d17a. No closure or re-opening required from that alone.
Do not count exact host pack conversion of 640 SuperJ6 presets as a runtime
all-bank playback, selector, or shipping claim. Current-style UI
reference requalification and strict timing remain open.
