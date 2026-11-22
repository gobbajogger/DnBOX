# BREAK SEQ full-page comparator

The physical entry is HOME → turn the part encoder to BREAK → SEQ. The host fixture clears the 16-hit pattern, sets the rate to 1/16 and uses PLAY. It advances the production audio engine, checks the active and previous pad LEDs and captures the 240×240 LCD across all sixteen positions. The capture is a host replay and its GIF uses a fixed 125 ms per sample.

| Input | Stopped still | Moving sample | LCD cue transitions |
| --- | --- | --- | --- |
| Shipping 0.0.26 source `94197fc839b00aff47b98ca02f3c6396e4a6eabf` | [PNG](shipping-026-lane-0-stopped.png) | [GIF](shipping-026-lane-0.gif) | 15/15 |
| Assigned branch `b995fb7374275e83a345fee45b7afc22d8206970`, Velocity | [PNG](branch-before-lane-0-stopped.png) | [GIF](branch-before-lane-0.gif) | 0/15 |
| Assigned branch, Chance | [PNG](branch-before-lane-1-stopped.png) | [GIF](branch-before-lane-1.gif) | 0/15 |
| Assigned branch, Pitch | [PNG](branch-before-lane-2-stopped.png) | [GIF](branch-before-lane-2.gif) | 0/15 |
| Assigned branch, Pan | [PNG](branch-before-lane-3-stopped.png) | [GIF](branch-before-lane-3.gif) | 0/15 |

[R22 candidate GIF](r22-candidate.gif) is the existing BREAK SEQ composition candidate keyed as `sequence.break.sequence` in the shared selection. It supplies layout evidence. Transport behavior comes from the shipping replay. The receipt preserves raw sampled phase counts and encoded GIF frame counts because the encoder may collapse duplicate adjacent frames.

The shipping comparator passed 81 fixture checks and the branch baseline passed 324 transport and input checks while failing the LCD movement requirement in all four lanes. This confirms the [existing PR #987 finding](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7065118335). The remaining route map and final stock builds are open.

Run the capture with `SMC_UI_TEST_FILE=tests/ui_inventory/break_seq_phase.c SMC_UI_OUT=build/host/break-seq sh tests/host/smc_ui_test.sh`. For the shipping checkout use the same fixture with `X0X_HOST_DEFS=-DSHIPPING_026`.
