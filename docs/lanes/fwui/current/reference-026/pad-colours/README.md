# Developer Pad Colours and SMC pickup

Use the opt-in SMK-37 Pad Colours build. Disable Clip mode and connect SMC. Open GLOBE → SYSTEM → PAD COLOURS, press PLAY to confirm top-left orientation, then OCT+ to advance. Move SMC F1 below its pickup threshold and stop. The test feeds that movement through the BLE MIDI queue and captures the first expiry frame at 1,200 ms and another frame 240 ms later.

| Source | Motion | Differing body rows during pickup | Differing at expiry / +240 ms |
| --- | --- | ---: | ---: |
| 0.0.26 `94197fc839b00aff47b98ca02f3c6396e4a6eabf` | [Pickup GIF](shipping-026/padcal-smc-route.gif) · [Eight pages](shipping-026/padcal-pages.gif) | 0 | 0 / 0 |
| Resumed `b995fb7374275e83a345fee45b7afc22d8206970` | [Pickup GIF](before/padcal-smc-route.gif) | 138 | 138 / 138 |
| Corrected painter | [Pickup GIF](after/padcal-smc-route.gif) · [Eight pages](after/padcal-pages.gif) | 138 | 0 / 0 |

Shipping uses the page-peek header over the retained body. The resumed build uses its full SMC pickup grid. The fix preserves that grid and all calibration data. SMC expiry already invalidates the LCD band, and the Pad Colours painter now respects that invalidation even if its calibration signature is unchanged. The full 240×240 before/pickup frames are identical between the resumed build and the fix. The restored frame is pixel-identical to the corrected pre-pickup still. The [receipt](receipt.json) records each still's decoded RGB hash and exact painter and dispatcher digests. GIF timing illustrates the sampled route.

Run `sh tests/host/ui_padcal_render_test.sh` for the production renderer and dispatchers with RAM-only board adapters. The regression exits 1 with the unchanged painter and exits 0 with the fix, with 34,839 checks across 513 states. The developer flag, view and wiring tests also passed. For the 0.0.26 comparison, the harness accepts the shipping page-peek state and records the pickup body count as an observation. It keeps the expiry equality checks and exercises 34,838 checks across 513 states. [Existing verified PR #987 defect](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7065109373).
