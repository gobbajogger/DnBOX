Native Synth One Phaser, compile-only checkpoint. Four parameters share the original DSP receiver and retain exact Init values via zero-code per control:

| ID | Owner byte | Control | Original range | Init |
|---|---|---|---|---|
| 116 | 48 | Mix | 0–1 | 0 |
| 117 | 49 | Rate | 1–300 cycles/min | 12 |
| 118 | 50 | Feedback | 0–0.8 | 0 |
| 119 | 51 | Width | 100–1000 Hz | 800 |

`apply.py <composed-donor>` layers on the native typed carrier with strict anchors. It uses one four-knob Phaser page and does not replace an existing control. All four edits update the existing s1c receiver while the engine is held; no delay-arena reinit/clear or extra persistent RAM. UI page-cap check still needs the composed donor link. The standalone real JieLi PI32v2 engine seam compiled at 1,142 B text/const for both Elite and Pro, 0 B data/BSS (non-LTO object size, not linked growth). No playback tests or firmware/device writes.
