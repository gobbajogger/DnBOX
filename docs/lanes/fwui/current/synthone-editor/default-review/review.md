# Default Elite and Pro Synth One editor review

Production `199f0444` gives both bass parts six matrix pages and a physical list for the 13 INIT parameters plus CRUSH RATE. The prior ordinary build opened the engine, preset and status selector. The 14 archived selector captures show that earlier three-entry layout. The current explicit patch-disabled build offers EFFECT and SOUND. The archive includes 96 matrix, 30 list and six live or simultaneous-editor captures. [Native PNG archive](routes.tar.xz) · [File SHA-256 manifest](routes.json)

## Native review

At 240×240, the oscillator balances, filter values and envelope shapes remain legible with either bass part selected. On EFFECT, the label is at the upper left, the trace occupies the lower left and the Hz value is right-aligned. A K5 move from 48000 to 43658 Hz changes 138 pixels on either part, including 20 in the trace and 118 in the readout. The adjacent envelopes and SOUND row retain their pixels. In the scrolled list, CRUSH RATE is visible and selected on the ninth line. The owner colors, SOUND fields and busy banner remain distinct. [Before the EFFECT spacing fix](effect-before-layout.png) · [Edit bounds and panel geometry](geometry.json)

The earlier centered EFFECT row filled 50.844% of its panel using the bounding rectangle around meaningful ink. Ten frames failed the 70% minimum. The revised ink bounds span 218×42 pixels of each 232×47 interior, or 83.969%. Across 132 current frames, all 708 panel checks pass. The smallest measured content bounds are 78.081%. All 270 visible list rows pass, with a minimum of 80.130%. Each of the 30 list windows has one highlighted line. The catalog replay also passed all 1,590 panel checks across 296 current and historical frames and rejected the earlier centered EFFECT image. [Per-frame measurements](panel-measurements.json.gz) · [Geometry checker](../../../../../../tests/host/check_synthone_layout.py)

The archive from the optional editor contained 122 frames. Of those, 92 have identical decoded RGB on the default editor and 30 differ. Ten additional frames record the rate control, the extra list entry and SOUND at its new page number. [Historical optional editor](../routes.json) · [Per-frame comparison](comparison.json)

## Emulator viewport review

Each of the 132 current captures and 14 earlier ordinary selector captures was placed at its original size in each of the three archived Qt WebKit shell rasters. The LCD crop from each shell matched its indexed native screen before replacement. All 438 resulting LCD crops reproduced their 240×240 source pixels. Independent visual inspection covered the earlier selector, matrix, selected list line and final SOUND page. The complete highlighted line and readout fit on the portrait screen, and the landscape LCD remains visible beside the shell controls. The desktop LCD retains the native spacing between the graph and value.

| Viewport | LCD origin | Reviewed default frame |
|---|---|---|
| 1440×900 | 263, 142 | [Bass A EFFECT after K5](1440x900.png) |
| 390×844 | 75, 203 | [Bass B scrolled to CRUSH RATE](390x844.png) |
| 844×390 | 43, 142 | [Bass B SOUND](844x390.png) |

The independent visual score is 10/10 for the reviewed default editor at native size and the three measured viewports. The viewport comparison reuses measured browser output and checks the complete LCD raster at each size. Live browser input, fresh browser layout and physical LCD behavior are outside this comparison. [Recorded shell measurements](../../panel-routes/viewport-replay.json)
