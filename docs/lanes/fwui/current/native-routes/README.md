# Startup, recovery and calibration on the host

The production LCD driver sends its normal command and RGB565 pixel streams to
a 240×240 RAM display. Production fonts and painters draw every frame. The
fixture supplies register values, clock ticks, button edges, encoder turns,
RAM linker regions and reset endpoints. It makes no device connection.

| Firmware entry | Exercised path | Hardware boundary |
| --- | --- | --- |
| `fm1_cstart` | Boots with a pending previous attempt, clears three RAM regions, copies three initialized regions, passes boot status through the clear, selects normal startup or safe mode, and requests the chip updater before LCD initialization after the fourth failed attempt. | Actual addresses, startup assembly, second-core halt and hardware protection registers. |
| `fm1_main` | Its unchanged startup prefix sees an unrecognized flash ID, takes the RAM-only path, initializes the LCD, draws the factory splash and both post-crash notices, and enters the real safe-mode function on the guarded route. | This first fixture stops before `x0x_dbg.boots++`. A second fixture below traverses the complete ordinary loop with a recognized flash ID. |
| `lcd_init` | Runs the SMK initialization sequence and transfers the complete black clear before DISPON. The test checks every write window and the full pixel count. | Electrical reset, DMA completion, panel timing and color on glass. |
| `fm1_fault` | Draws six diagnostic rows, polls the production four-second deadline against a test clock and calls reset. | Hardware exception entry, physical delay and reboot. |
| `safe_main`, `enter_uboot` | Checks PLAY before and after the 500-ms guard, both core readouts, the retry overlay with and without a crash record, USB request, detach, delays, bootguard and bootloader/reset endpoints. | Physical key, USB, watchdog and reset. A separate test also exercises a stalled updater and the return to recovery. |
| `panel_setup` | Tests all 14 buttons and seven encoders, alternating encoder direction, timeout at initial release, second button and second encoder, and success. All three timeouts restore the old map. Success saves the checked new map once into RAM. | Physical scanning, actual timing and persistent calibration. |

The replay compares every decoded pixel in 31 frames against their published
images in the main screen index. Seven additional states have full-frame
oracles. They use the driver clear, the theme background, the title above an
empty calibration screen, or the retry message over the correct predecessor.
Each additional PNG and all 38 RGB digests are in this directory. A one-pixel
vertical displacement of the retry message fails the frame comparison.

The [supplemental screen index](screen-index.tsv) maps the 40 recovery, calibration and waveform case IDs to their PNGs and decoded RGB digests.

The primary inventory retains its original 98/118 count. Seven additional complete function bodies execute across the two host startup fixtures, including `fm1_main`. Physical startup remains unmeasured.

## Returning from a failed update

A host that stops sending data during a safe-mode install can recover without
power cycling. After three unanswered 5-second package reads, the firmware
shows the failed result for 1.5 seconds, then repaints the recovery controls
and saved crash registers. PLAY shows the retry caption and requests a reset.
The scanner, USB and bootguard timer keep their state.

The compiled host replay compares all 57,600 pixels at each of the package,
failure, restored safe-mode and retry checkpoints. Before the change,
the first restored frame still showed FAILED. Pressing PLAY painted a retry
caption over that screen. Removing the redraw reproduced both bad frames.
[Restored controls](safe-after-failed-update.png) ·
[Retry before](safe-retry-after-failed-update-before.png) ·
[Retry after](safe-retry-after-failed-update.png) ·
[Pixel differences](recovery-delta.json) · [Native receipt](report.json)

## Recognized flash and the ordinary application loop

Seven private SMK host runs execute the complete production `fm1_main`. They
drive a RAM flash device with a recognized JEDEC ID, the production storage-owner
and splash-selection functions, a simulated 1-ms clock and both octave buttons.
The full application frames come from the regular engine, project and UI code.
The 16 indexed states include three splashes that match their archived decoded
RGB digests and 13 additional 240×240 PNGs. Seven caption masks check 5,272
ink samples against the finished header, including antialiased edges. All checked
samples fit in x3–213 and y0–18. [Frame index](normal-loop/screen-index.tsv) ·
[Case results and input hashes](normal-loop/normal-loop.json)

| Host case | Display and endpoint |
| --- | --- |
| Foreign owner, stale OTA staging | The owner reset removes the custom splash, restores the factory startup and shows `OLD DATA CLEARED`. Five mapped OTA sectors clear. The notice expires and the ordinary header returns. |
| Matching owner, text or picture | Each stored splash matches its archived counterpart. Both runs reach the first ordinary frame without the clear notice. |
| Release during countdown | The ordinary header displays `UPDATE MODE IN 3..`, `2..` and `1..`. Releasing both octave buttons requests `UPDATE MODE CANCELLED`. In this part view, ordinary octave input then replaces it with `STEPS 17-32` before drawing. Its expiry is also captured. |
| Hold through exactly 5,000 ms | When a frame is due on this tick it displays `UPDATE MODE IN 0..`. Continuing to hold for 1 ms calls `enter_uboot("UBOOT")`. The earlier recovery fixture covers that endpoint’s visible output. |
| Release immediately after 5,000 ms | The countdown has already set its state to zero. Releasing both buttons at 5,001 ms skips the cancellation request and the next frame displays `STEPS 17-32`. |

The last two runs start with a valid sample shadow. The recovery case restores
all 4,096 bytes in 16 ordered writes before LCD initialization. The control
retains a valid live sector and discards a different stale shadow without
writing the live sector. Each run sees the user zone and both BREAK bank
revisions after project load. Both first ordinary screens match the matching-owner
control at all 57,600 pixels. Reverting the guard makes the seven-case test
stop during recovery at the unbound pattern table. [Recovery frames and case results](normal-loop/normal-loop.json)

The patch-enabled host replay at `199f0444` reproduced all 16 native frame hashes and all seven case-to-frame mappings. [Current ordinary startup replay](normal-loop/replay-199f0444.json)

## Stock Elite and Pro startup at `11bc3f3a`

Twelve independent `fm1-live` shots ran the two linked stock packages at
360 MHz with strict alignment, timed LCD transfers and the native IRQ and
audio paths. The emulator starts at the application entry after the modeled
SPL handoff. The seven-case C replay above records the first completed UI
frame. The stock replay checks the splash at 448 ms and the ordinary screen
at 1,500 ms against those full-frame references. All 57,600 RGB565 values
match in every comparison. [Stock splash](stock-splash.png) ·
[Stock ordinary screen](stock-ordinary.png) · [Per-model hashes and logs](stock-boot.json)

| Stock input | Flash operations | Result on each model |
| --- | ---: | --- |
| Valid shadow, erased live slot | 18 | Recovered all 4,096 bytes and erased the shadow. |
| Valid live slot, different stale shadow | 1 | Kept all 4,096 live bytes and erased the shadow. |
| Fresh process booting the recovered flash | 0 | Read the saved sample and reached the same ordinary screen. |

Each model's three final 1 MiB flash images match byte for byte. Only the
expected live and shadow sectors changed during recovery. All six ordinary
checkpoints reported one boot, zero crashes, one stored zone, one BREAK loop
and 89 audio halves. Physical power-loss persistence, color on glass and
electrical timing require a board.

Run each model with its own matching FWSC and ELF. The output directory
must be new.

```sh
python3 tests/ui_inventory/stock_boot_fixture.py --model elite --emulator /path/to/fm1-live \
  --fwsc /path/to/x0x-smk37-elite.fwsc --elf /path/to/elite/x0x.elf /tmp/elite-recovery
```

## Stock panel calibration and updater entry at `11bc3f3a`

The stock Elite and Pro packages also consumed scripted SMK matrix keys and
quadrature encoders through the timed scanner. All 14 button prompts, all
seven encoder prompts and the title matched their archived full-frame
references. A rotated button mapping with alternating encoder directions
changed only the panel-object sector. A fresh emulator process read the
exact 32 taught bytes without another flash operation.

A MIDI Device Inquiry returned a device reply over the emulated USB cable.
The updater request drew the USB updater screen, detached and reset the
chip. Holding both octave buttons for five seconds exercised the other updater
screen and reset. Neither path changed flash. The simulator restarts the
application on PR_PWR reset. The board's mask ROM lies outside this
measurement. [Per-model frame, storage and USB records](stock-panel.json)

```sh
python3 tests/ui_inventory/stock_panel_routes.py --model pro --emulator /path/to/fm1-live \
  --fwsc /path/to/x0x-smk37.fwsc --elf /path/to/pro/x0x.elf /tmp/pro-panel
```

## Final stock rerun after the Synth One UI change

At `61827a7`, both linked packages repeated the startup, cold restart,
calibration and updater checks above. The 22 target runs retain the
application counters, LCD comparisons, USB events and flash
hashes by model. Each recovery, control and cold restart reached one boot,
31 UI frames, 89 audio halves and zero recorded crashes. Both models detached
at 2,104 ms and reset at 2,133 ms on USB request, and reset at 6,637 ms on
the octave hold. The original package results remain pinned to `11bc3f3a`.
[Final package and ELF hashes, case results and raw logs](stock-final.json)

The final `ui.c` has no production caller of `strip_value_fit`. Its one
call to `drum_strip_glyph` passes `DG_LEVEL` directly. Neither stock ELF
contains a separate symbol for `strip_value_fit` or `dg_tail`. Both retain
`drum_strip_glyph` (260 B), `draw_wave_glyph` (542 B), `enter_uboot` (98 B)
and `fm1_cstart` (6,784 B). These source and link results leave the original
frame inventory counts unchanged. [Source and symbol dispositions](../native-gaps.json)

## Linked exception, safe mode and PLAY retry

At `61827a7`, a test-only emulator fixture delivers two modeled guard exceptions
to each stock firmware. The unchanged CPU interrupt handler records the actual
PC, vector, return address and diagnostic registers. Both packages show their
first full diagnostic at 1,312 ms and their second at 9,312 ms. The firmware
requests PR_PWR reset after each four-second hold. On the next boot,
`fm1_cstart` selects safe mode. Pressing PLAY on the modeled SMK matrix
shows the retry overlay at 15,216 ms, resets at 15,238 ms and shows the
post-crash screen at 15,664 ms. The boot counter reaches three.

| Linked target | First reset | Second reset | Safe frame | Captured target frames |
| --- | ---: | ---: | ---: | --- |
| Elite | 5,307 ms | 13,307 ms | 13,840 ms | [Diagnostic](guard/elite-first-fault.png) · [Safe mode](guard/elite-safe-play-safe.png) · [Retry](guard/elite-safe-play-retry.png) |
| Pro | 5,307 ms | 13,308 ms | 13,840 ms | [Diagnostic](guard/pro-first-fault.png) · [Safe mode](guard/pro-safe-play-safe.png) · [Retry](guard/pro-safe-play-retry.png) |

Each of the five model-specific target frames matches all 57,600 pixels of
an independent native rendering with that model's saved crash record. The
emulator fixture changes only the timing of the modeled peripheral exception.
Physical fault origin, electrical timing and watchdog behavior require a
board. [Logs, crash records, complete frame hashes and image paths](stock-guard.json)

For this replay, apply `tests/ui_inventory/stock_guard_injection.patch` to
the external emulator checkout at `gobbajogger/DnBOX@6b60618c` and
build `fm1-live` in release mode. Then run the firmware checkout's fixture
once for each matching package and ELF.

```sh
python3 tests/ui_inventory/stock_guard_routes.py --model elite --emulator /path/to/patched/fm1-live \
  --fwsc /path/to/x0x-smk37-elite.fwsc --elf /path/to/elite/x0x.elf /tmp/elite-guard
```

## Alternate renderer entry conditions

| Renderer(s) | Production dispatch at `11bc3f3a` and evidence |
| --- | --- |
| `strip_value_fit`, `dg_tail` | `draw_part_sound` calls the generic matrix widget only for bass on the normal part view. Drum part dispatch calls `draw_drum_voice` in both main bands and the knob strip. In other views the generic knob strip calls `drum_strip_glyph` only for `DG_LEVEL`. The tail specifically requires `DG_DECAY` or `DG_NTAIL`. These paths have no admitted route in that dispatch. |
| `original_cell_value` | Its sole call is in the generic footer for a bass engine or bank parameter. `describe_pages` emits those targets on the bass part view. The part view returns after `draw_part_sound(2)` before reaching the generic footer. The sequencer preset page uses the distinct `R_BPATSND` parameter. |
| `draw_wave_glyph` | A supported oscillator shape must survive `wave_shape` while its original waveform projection fails. The host scan covers 111 Chicago/Miami factories and 124,032 single-control previews with zero rejections. The [field ownership and range proof](#simultaneous-wave-previews) also covers valid simultaneous previews. The malformed-lock fixture exercises the fallback painter. |
| `ui_position_badge`, `header_pattern_cue`, `matrix_row_hint`, `matrix_next_hint`, `lane`, `cv_blit_from` | These static functions have no caller in the registered firmware sources in the inventory. |
| `nav_arrow`, `header_pattern_badge`, `row_picture` | Only the uncalled helpers in the preceding row reference them. |

## Simultaneous wave previews

At #975 commit `4c05722d`, the wave projector rejects invalid cells, masks or
control values before drawing. For an admitted Chicago or Miami cell, the
bounds of its native fields also hold under *any combination* of stored edits
and preview values. This is a source-level result for the normal oscillator
pages. The compiled single-control sweep and malformed-lock framebuffer test
remain separate evidence.

| Projector reads | Which controls can change them | Bounds after simultaneous edits |
| --- | --- | --- |
| Chicago shape IDs 1/9, width 22 | All three come from the fixed compact base. | Shape indices 1 and 3, width 0.5. |
| Chicago gain IDs 3/11 | Only macro 0 writes these two targets. Its two affine maps and their final quantizers stay in [0,1]. | Each rounded gain lies in 0..32768. Edits to other controls cannot change it. |
| Miami shape IDs 1/5/43 | Macros 0/6/11 each select a separate lookup table. | The carrier yields 0..2 and the two other shapes yield 0..3, giving 48 supported shape triples. |
| Miami width 22 | Fixed compact base. | Stored binary32 value 0.9010000228881836. |

The [Chicago setter](../../../../../experiments/original_synth/chicago_program.c)
and [Miami setter](../../../../../experiments/original_synth/miami_program.c)
assign only the target indices between the selected macro's consecutive table
offsets. Native readback selects that field or the fixed base. Factory loading,
[patch restoration](../../../../../firmware/src/app/original_sound.c) and
[wave projection](../../../../../firmware/src/app/ui_original_wave.h) use those
same setters. An admitted preview can replace its own earlier value, and cannot
change another macro's targets. Both setters accept every finite normalized
value in [0,1]; admitted control bytes have that conversion. The shape and
gain rejection tests inspect individual fields. Drawing the wave after those
checks introduces no additional rejection. This is why the disjoint field
bounds cover combined edits without enumerating the Cartesian product.

Run `node tests/ui_inventory/wave_preview_domain.cjs` from the repository
root to read both generated mapping tables and the compact binary32 dictionary.
Two in-memory negative controls were rejected: assigning Chicago shape 1 to
a mutable macro, and adding index 4 to a Miami shape lookup.

## Reproduce

From the repository root run the focused host tests. Each takes its own
temporary directory for compilation and writes the resulting evidence to the
directory supplied on the command line.

```sh
python3 tests/ui_inventory/native_routes.py /tmp/dnbox-native-routes
python3 tests/ui_inventory/wave_fallback_frame.py /tmp/dnbox-wave-fallback
python3 tests/ui_inventory/startup_loop.py /tmp/dnbox-normal-loop
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

At `11bc3f3a` the stock Elite and Pro symbol tables retain separate symbols for
`enter_uboot` (98 B), `fm1_cstart` (6,784 B), `draw_wave_glyph` (542 B),
`strip_value_fit` (276 B) and `dg_tail` (512 B). The admission decisions
above follow actual dispatch. Inlining and elimination limit what these
symbol tables can show.
