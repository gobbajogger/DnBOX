# Startup, recovery and calibration on the host

The production LCD driver sends its normal command and RGB565 pixel streams to
a 240×240 RAM display. Production fonts and painters draw every frame. The
fixture supplies register values, clock ticks, button edges, encoder turns,
RAM linker regions and reset endpoints. It makes no device connection.

| Firmware entry | Exercised path | Hardware boundary |
| --- | --- | --- |
| `fm1_cstart` | Boots with a pending previous attempt, clears three RAM regions, copies three initialized regions, passes boot status through the clear, selects normal startup or safe mode, and requests the chip updater before LCD initialization after the fourth failed attempt. | Actual addresses, startup assembly, second-core halt and hardware protection registers. |
| `fm1_main` | Its unchanged startup prefix sees an unrecognized flash ID, takes the RAM-only path, initializes the LCD, draws the factory splash and both post-crash notices, and enters the real safe-mode function on the guarded route. | On the normal path the fixture stops immediately before `x0x_dbg.boots++`. Mounted flash, later engine/audio startup, interrupts and the application loop remain untested here. |
| `lcd_init` | Runs the SMK initialization sequence and transfers the complete black clear before DISPON. The test checks every write window and the full pixel count. | Electrical reset, DMA completion, panel timing and color on glass. |
| `fm1_fault` | Draws six diagnostic rows, polls the production four-second deadline against a test clock and calls reset. | Hardware exception entry, physical delay and reboot. |
| `safe_main`, `enter_uboot` | Checks PLAY before and after the 500-ms guard, both core readouts, the retry overlay with and without a crash record, USB request, detach, delays, bootguard and bootloader/reset endpoints. | Physical key, USB, watchdog and reset. The OTA service is idle. |
| `panel_setup` | Tests all 14 buttons and seven encoders, alternating encoder direction, timeout at initial release, second button and second encoder, and success. All three timeouts restore the old map. Success saves the checked new map once into RAM. | Physical scanning, actual timing and persistent calibration. |

The replay compares every decoded pixel in 31 frames against their published
images in the main screen index. Seven additional states have full-frame
oracles. They use the driver clear, the theme background, the title above an
empty calibration screen, or the retry message over the correct predecessor.
Each additional PNG and all 38 RGB digests are in this directory. A one-pixel
vertical displacement of the retry message fails the frame comparison.

The [supplemental screen index](screen-index.tsv) maps all 40 case IDs to their PNGs and decoded RGB digests.

The primary inventory retains its original 98/118 count. Six additional
whole function bodies now execute at host endpoints. The partial `fm1_main`
prefix is recorded separately. Physical startup remains unmeasured.

## Alternate renderer entry conditions

| Renderer(s) | Current production dispatch and evidence |
| --- | --- |
| `strip_value_fit`, `dg_tail` | `draw_part_sound` calls the generic matrix widget only for bass on the normal part view. Drum part dispatch calls `draw_drum_voice` in both main bands and the knob strip. In other views the generic knob strip calls `drum_strip_glyph` only for `DG_LEVEL`. The tail specifically requires `DG_DECAY` or `DG_NTAIL`. These paths have no admitted route in that dispatch. |
| `original_cell_value` | Its sole call is in the generic footer for a bass engine or bank parameter. `describe_pages` emits those targets on the bass part view. The part view returns after `draw_part_sound(2)` before reaching the generic footer. The sequencer preset page uses the distinct `R_BPATSND` parameter. |
| `draw_wave_glyph` | A supported oscillator shape must survive `wave_shape` while its original waveform projection fails. The host scan covers 111 Chicago/Miami factories and 124,032 single-control previews with zero rejections. Every deliberately out-of-range preview supplied to the projector is rejected. Multiple simultaneous edits are outside this scan. |
| `ui_position_badge`, `header_pattern_cue`, `matrix_row_hint`, `matrix_next_hint`, `lane`, `cv_blit_from` | These static functions have no caller in the registered firmware sources in the inventory. |
| `nav_arrow`, `header_pattern_badge`, `row_picture` | Only the uncalled helpers in the preceding row reference them. |

## Reproduce

From the repository root run the focused host tests. Each takes its own
temporary directory for compilation and writes the resulting evidence to the
directory supplied on the command line.

```sh
python3 tests/ui_inventory/native_routes.py /tmp/dnbox-native-routes
python3 tests/ui_inventory/wave_fallback_frame.py /tmp/dnbox-wave-fallback
```

The Chicago fault-input probe also renders the full first oscillator page with
a valid WAVE value. A normal held cutoff lock at 127 keeps the original wave.
Changing that sibling lock to 128 in host RAM makes the unmodified projector
reject it and selects `draw_wave_glyph`. Suppressing only this painter removes
43 pixels at x9–37, y52–60. The two captures are included beside
[`wave-frame.json`](wave-frame.json). The full-frame comparison changes 90 pixels in the oscillator plot and 36 in the
cutoff readout, with zero changes elsewhere. These two cases supplement the
38 native startup/recovery captures. The malformed value is a robustness
fixture. The project importer rejects a held-step value above its parameter
maximum in `project_validate_for`.

The stock Elite and Pro symbol tables retain separate symbols for
`enter_uboot` (98 B), `fm1_cstart` (6,784 B), `draw_wave_glyph` (542 B),
`strip_value_fit` (276 B) and `dg_tail` (512 B). The admission decisions
above follow actual dispatch. Inlining and elimination limit what these
symbol tables can show.
