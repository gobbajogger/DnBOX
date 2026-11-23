# perf lane (memory and performance)

Reference: /workspace/dnbox-handoff/BOT-RULES.md (finisher bots, 2026-11-23). One branch, `lane/perf`, one PR into `integration/dnbox-consolidated`.

## Items
1. All engines fit stock Elite and Pro (no PSRAM): gate budget XIP free >= 1024 B, pool margin >= 512 B; per-engine memory report as a test.
2. 10 simulated minutes of all-engine heavy playback: dropouts=0, zero late blocks, every block under 11,609 us, both models.
3. The user's 5 BREAK samples in stock firmware, without overwriting user slots or substituting samples.
4. Combined Elite+Pro builds, flashmap, host suite 0 FAIL / 0 NOT_RUN.

## Done
- (this PR) `tools/engine_memory_report.py` + `tests/host/engine_memory_report_test.py`: per-engine flash / RAM / pool attribution of each linked model; the gate's build step prints it for Elite and Pro and fails on the stock budget.
- (this PR) `FL_FDATA` factory synth data region (user approval 2026-11-23): the top `FL_FDATA_SIZE` bytes of FL_SMP, cut from the end of sample slot 2. Slots 1/2/3 keep their start offsets; slots 1 and 3 keep their sizes. Stock value 0 (no change). `tools/gate_flashmap.py`, `tools/fm1pkg_make.py` (package slot map) and `slots.c` follow the one define; `tests/host/flashmap_fdata_test.py` covers 0 and nonzero sizes.

## Measured at 395579bd (Elite / Pro)
- App image 551,220 / 551,212 B of 552,672: XIP free 1,452 / 1,460 B (428 / 436 B above the 1,024 B minimum).
- Pool 335,040 of 344,064 B: margin 832 B (320 B above 512). Ordinary RAM .data+.bss 97,188 of 98,304 B (1,116 B free).

## Left
- Item 1: full Synth One (694 programs, FX) and SuperJ6 (640 presets) need far more than the free XIP; packed tables go to FL_FDATA (size set with the synth lane), and code room comes from moving read-only tables out of the app image.
- Activating a nonzero FL_FDATA needs the nightly installer's slot-map allow-list entry and a slot-2 migration (back up slot 2 when its data runs past the new end).
- Items 2-4.

## Risks
1. A nonzero FL_FDATA shortens slot 2; a sample longer than the new slot 2 fails its header check (fail-closed) until it is re-uploaded.
2. Engine attribution is a lower bound (inlined code stays in callers).
3. Code room for the synth engines (25-31 KB) is not available in the app image without moving data out.

## Row 2: 10-minute heavy playback
`tests/host/heavy_playback_10min_test.sh` (suite entry `heavy-playback`, class G) runs 51,680 blocks
(600.0 s) for Elite and Pro, engine pairs Chicago+Miami (factory 18, Chicago FX 6) and Synth One+SuperJ6,
all drums, breaks, bus FX, reverb, delay and master on. Each block is timed in thread CPU time against
11,609 us; it fails on any late block or dropout. Runs at nice 19.

## Code room for the Synth One reader
`tools/build.py` OZ_UNITS builds the control-rate units (seq/sequencer, seq/tb3po, seq/pattern,
seq/motion, dsp/x0x_param) at -Oz instead of -O2. XIP free: Elite 1,452 -> 4,072 B, Pro 1,460 -> 4,080 B
(+2,620 B each); pool and RAM unchanged. Host heavy stress (4,096 blocks, Elite) -O2 vs -Oz for these
units: Chicago+Miami mean 536.3 vs 525.9 us, Synth One+SuperJ6 481.2 vs 490.9 us (noise), late 0.

## Row 3: the owner's 5 BREAK zones as default slot contents (owner choice 18:29: "2 is better")
`assets/default-breaks/` holds the 5 original encoded zones (pinned sha256, same bytes as DnBOX-nightly
web/product-site/assets/starting-breaks). `tools/default_breaks.py` builds the default slot images:
slot 1 (0xC2000, 80 KiB) "BR1" = BR1 zones 1-3, 60,237 B (512 header + 59,725 ADPCM), 21,683 B left;
slot 2 (0xD6000, 60 KiB with FL_FDATA 0xF000) "BR2" = BR2 zones 1-2, 50,061 B (512 + 49,549), 11,379 B left;
slot 3 (0xB3000, 32 KiB) empty. 22,050 Hz, no re-encode, decoded PCM is the pinned PCM sample for sample.
They are defaults, not a reserved area, written on a FRESH install only: `tools/fm1_install.py
--fresh-install` refuses before writing if the keyboard has any sample (or no readable layout), and
after the install `default_breaks.fill_fresh` writes them only if every slot is still blank. An
ordinary install never writes them, so a default the user cleared stays cleared. Tests:
`default-breaks` (pins, bit-exact decode, fit for FL_FDATA 0..0xF000) and `scenario-default_breaks`
(the firmware's slots.c parses both images, 5 loops listed, BREAK plays them).
