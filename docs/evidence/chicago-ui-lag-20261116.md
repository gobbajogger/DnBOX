# Chicago UI lag measurement, 2026-11-16

On the host, Chicago pages AMPLIFIER, ARPEGGIATOR, EFFECT and SOUND called `original_sound_restore_chicago` and `original_chicago_program_init` **386 times on every UI frame** at `94197fc8` (no fix). At `02121895` (fix `4b663ba2` plus the PR #897 test commits) a steady frame on those same pages did it **0 times**. The cold frame fell from 386 to 1. The 256-value glyph oracle matched **256/256**. Elite, Pro and emu360 did not run.

This file is a measurement record. It does not change firmware behavior. A player on those four Chicago pages was paying for 386 program restores on every redraw before the fix, including frames that did not change the screen. After the fix, a steady redraw does not restore the program.

## Identities

| Role | SHA | What it is |
| --- | --- | --- |
| Base | `94197fc839b00aff47b98ca02f3c6396e4a6eabf` | Lineage #869. Parent of the fix. No `og_states` cache. |
| Fix | `4b663ba299bec435a7f6f09e5a19870dc2f9e17f` | `ui: stop Chicago AMP/ARP restoring the program 386 times a frame` on `lane/fmb-chicago-ui-lag`. |
| Head | `02121895e268e08d8809e55f9e881dc1baaf8fe6` | PR #897 oracle head. The source that was measured. Parent of this evidence commit. |

Ancestry check, run on the detached checkout before this evidence commit, with `HEAD` equal to the head SHA above:

```
git merge-base --is-ancestor 4b663ba299bec435a7f6f09e5a19870dc2f9e17f HEAD
```

Exit code **0**. The same command for the base SHA against the fix SHA also exited 0. `git rev-parse 4b663ba299bec435a7f6f09e5a19870dc2f9e17f^` is the base SHA.

`git diff --stat 94197fc839b00aff47b98ca02f3c6396e4a6eabf 02121895e268e08d8809e55f9e881dc1baaf8fe6 -- firmware` is one file: `firmware/src/app/ui.c`, 10 insertions and 4 deletions. That hunk is the whole firmware difference. The three commits after the fix change only `tests/host/chicago_og_states_test.c`.

Machine: Linux 6.12.113+ x86_64. Host compiler: Ubuntu clang 18.1.3, target `x86_64-pc-linux-gnu`. Times below are UTC on 2026-11-16.

## 1. Host oracle

Command, from the repository root, at the head SHA:

```
sh tests/host/chicago_og_states_test.sh
```

Exit code **0**. The program prints `CHICAGO OG_STATES PASS` only when its failure counter is 0. No `FAIL` line was printed. One pre-existing compiler warning was printed (`ui.c` sign compare at the `ORIGINAL_SOUND_JUNO` name check). The script still exited 0.

Log time: 2026-11-16 00:34:53 UTC.

| Check | Count |
| --- | --- |
| Program failure counter | 0 |
| Exit code | 0 |
| `GLYPH_PIXEL_EQUAL` | 256/256 |
| `PIXEL_ORACLE_NEGATIVE` | 2/2 (`c=11` and `c=15`, both `PASS`) |
| Control `c=11` states | 2 |
| Control `c=15` states | 3 |
| Legacy 0..127 scan, each control | 128 restores and 128 inits |
| Chicago steady frames, 8 pages × 8 frames | 0 restores and 0 inits |
| Miami steady frames | no `FAIL Miami` line |

The legacy 128 is one walk of pot values 0..127 inside the test's `legacy_state_table`. It is the old `og_states` loop, run once. It is not a full UI frame. The frame count is in section 4.

Quoted program output:

```
WRAP_PROBE restore=2 init=2
PIXEL_ORACLE_NEGATIVE c=11 PASS
CONTROL c=11 legacy_scan_restore=128 legacy_scan_init=128 states=2
PIXEL_ORACLE_NEGATIVE c=15 PASS
CONTROL c=15 legacy_scan_restore=128 legacy_scan_init=128 states=3
GLYPH_PIXEL_EQUAL 256/256 cases
CHICAGO p=0 'OSCILLATOR' cold_restore=2 cold_init=2 steady8_restore=0 steady8_init=0 frame_crc32=8f38ce9c/8f38ce9c
CHICAGO p=1 'FILTER 1' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=d26168ed/d26168ed
CHICAGO p=2 'FILTER 2' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=d25710e5/d25710e5
CHICAGO p=3 'ENVELOPE' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=3c15e8d3/3c15e8d3
CHICAGO p=4 'AMPLIFIER' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=4fbdcd5e/4fbdcd5e
CHICAGO p=5 'ARPEGGIATOR' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=ba9676e0/ba9676e0
CHICAGO p=6 'EFFECT' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=10476287/10476287
CHICAGO p=7 'SOUND' cold_restore=1 cold_init=1 steady8_restore=0 steady8_init=0 frame_crc32=2bb8e186/2bb8e186
CHICAGO OG_STATES PASS
```

`WRAP_PROBE restore=2` is two `pref_value_of` calls, used to prove the linker wrap is live. A zero counter would have been `WRAP_INACTIVE` and a failed run.

## 2. Pixel oracle

The 256/256 result is the same process as section 1. There is no second command. `check_states_and_pixels` compares the cached glyph's RGB565 pixels with the legacy drawing for controls 11 and 15, values 0..127. That is 256 cases. The program printed `GLYPH_PIXEL_EQUAL 256/256 cases` and then `CHICAGO OG_STATES PASS`.

The negative check forces a wrong glyph index and requires the pixels to differ. Both controls printed `PIXEL_ORACLE_NEGATIVE` `PASS`. A blank framebuffer would have printed `PIXEL_ORACLE_INACTIVE` and failed.

This oracle ran on the fixed tree only. It checks that the cached glyph still draws the same pixels. It does not measure lag.

## 3. Elite and Pro native builds

**NOT RUN.** The commands exited because the pinned SDK file is absent. That is a missing input, not a firmware pass and not a firmware failure.

Commands, development configuration, no `--release`, from each source tree:

```
python3 tools/build.py --board smk37 --model elite
python3 tools/build.py --board smk37 --model pro
```

| Tree | Model | Exit | stderr |
| --- | --- | --- | --- |
| Head `02121895` | elite | 1 | `fm1pkg_make: cpu/wl82/tools/uboot.boot: No such file or directory. Set AC79_SDK to a JieLi AC79 SDK checkout.` |
| Head `02121895` | pro | 1 | same stderr |
| Base `94197fc8` | elite | 1 | same stderr |
| Base `94197fc8` | pro | 1 | same stderr |

Stdout on each attempt, before the SDK read: `check_locked_settings: 20 settings ok`. No object file, ELF, flash figure, pool figure or RAM figure was produced.

Missing inputs on this machine:

- `AC79_SDK` and `SDK_LOCAL` are unset. `tools/build.py` pins `AC79NN_SDK_V1.2.14_2026-10-13` and the three files `uboot.boot`, `cfg_tool.bin` and `cfg/eq_cfg_hw.bin` under `cpu/wl82/tools`. None of those files were opened.
- `JIELI_TOOLCHAIN` and `X0X_TOOLCHAIN` are unset. `~/.jieli` is absent. No pi32 compiler is on `PATH`. `tools/get_toolchain.sh` was not run.

Flash free, pool free, RAM and the deltas against `94197fc8` are **NOT RUN**. The 8,192 B pool floor was not evaluated on these bytes.

Earlier evidence, not remeasured here: #869 at `94197fc8` recorded Elite 8,440 / Pro 8,448 flash free and pool 8,340. Those numbers are a citation of that earlier record. This run does not confirm them and does not report a delta from them.

## 4. Per-frame restores and host time

The in-repo host profiler `X0X_PROFILE` in `host/x0x_host.c` times `engine_render` while the sequencer is playing. `ui_frame` runs later in `run_ms`, outside that timer, so that profiler does not count Chicago restores. It was **NOT RUN**. The measurement that matches this bug is a host-only harness that calls `ui_frame` and counts the wrapped restore and init.

Harness source, committed with this record and compiled as the file that produced the numbers: `docs/evidence/chicago-ui-lag-20261116-measure.c`. SHA-256 `c10aa22731d55e2f02a1cffda2703a12de9b78abbde4545cd301bb04b8bda4bc`. It was compiled from `/tmp/chicago-lag-logs/chicago_frame_measure.c`, which is the same bytes.

It does not edit firmware. It includes one tree's `host/x0x_host.c`, boots, adopts Miami on voice 0 and Chicago on voice 1, then uses the same `settle` sequence as `tests/host/chicago_og_states_test.c`. Chicago is `PART_303B` (part 3). Miami is `PART_303A` (part 2). Page titles in the table are the titles the oracle printed for the same page index.

Each tree was generated with `sh host/build_host.sh --generate-only` and compiled with the oracle's flags: `-O2 -std=c99`, `tests/host/original_build.sh` `ORIGINAL_FLAGS`, the same `-I` paths, and `--wrap` of `original_sound_restore_chicago` and `original_chicago_program_init`. `MEASURE_LABEL` was `with-fix` or `without-fix`. `X0X_HOST_C` was that tree's `host/x0x_host.c`.

| Tree | SHA | Harness exit |
| --- | --- | --- |
| with-fix | `02121895e268e08d8809e55f9e881dc1baaf8fe6` | 0, at 2026-11-16 00:37:40 UTC |
| without-fix | `94197fc839b00aff47b98ca02f3c6396e4a6eabf` | 0, at 2026-11-16 00:37:55 UTC |

`WRAP_PROBE` printed `restore=1 init=1` on both, from one `pref_value_of` call. The without-fix tree still contains the scanning `og_states` (`for (v = 0; v <= 127; v++)`).

Both runs printed 212 `FRAME` lines. `init` equalled `restore` on every line (0 mismatches).

Chicago steady frames are 24 samples (3 repeats of 8). The median of an even count is the mean of the 12th and 13th sorted values, which is why some medians end in 0.5 ns. Cold is one sample. Host time is `CLOCK_MONOTONIC` around that single `ui_frame` call.

### Restore counts

| Page | Cold before | Cold after | Steady before (24/24) | Steady after (24/24) |
| --- | --- | ---: | ---: | ---: | ---: |
| 0 OSCILLATOR | 2 | 2 | 0 | 0 |
| 1 FILTER 1 | 1 | 1 | 0 | 0 |
| 2 FILTER 2 | 1 | 1 | 0 | 0 |
| 3 ENVELOPE | 1 | 1 | 0 | 0 |
| 4 AMPLIFIER | 386 | 1 | 386 | 0 |
| 5 ARPEGGIATOR | 386 | 1 | 386 | 0 |
| 6 EFFECT | 386 | 1 | 386 | 0 |
| 7 SOUND | 386 | 1 | 386 | 0 |

Miami pages 0..5: one cold frame and one steady frame each, restore 0 and init 0 on both trees.

The with-fix steady counts match the oracle's `steady8_restore=0` on all eight Chicago pages. The with-fix cold counts match the oracle's `cold_restore` column, including AMPLIFIER cold 1.

### Host `ui_frame` time

These are host wall-clock times on this clang `-O2` build. They are not device times, not emu360 times, and not hardware evidence.

Steady frames, nanoseconds, min / median / max:

| Page | Before ns, min / median / max | After ns, min / median / max | Median delta |
| --- | --- | --- | ---: |
| 0 OSCILLATOR | 106825 / 112628 / 131683 | 101498 / 108641 / 135221 | -3987 ns |
| 1 FILTER 1 | 107402 / 112554 / 141344 | 101695 / 102217.5 / 106694 | -10336.5 ns |
| 2 FILTER 2 | 112972 / 119071 / 167087 | 101583 / 102160.5 / 114731 | -16910.5 ns |
| 3 ENVELOPE | 121308 / 125293.5 / 156784 | 100205 / 101013.5 / 106646 | -24280 ns |
| 4 AMPLIFIER | 913875 / 984182.5 / 1246672 | 135279 / 139615.5 / 182757 | -844567 ns |
| 5 ARPEGGIATOR | 853550 / 965994.5 / 1065995 | 135769 / 136325.5 / 140677 | -829669 ns |
| 6 EFFECT | 854661 / 926495.5 / 1450407 | 135964 / 136565.5 / 168413 | -789930 ns |
| 7 SOUND | 911889 / 978317 / 1249330 | 135774 / 136596 / 174981 | -841721 ns |

On pages 4–7 the slowest fixed frame is still faster than the fastest unfixed frame: before-min is above after-max in each of those four rows. Median drop on those pages is 790 µs to 845 µs of host time per steady frame (789930 ns to 844567 ns).

Pages 0–3 already restored 0 times on a steady frame before the fix, and they still do. Their median host times differ by 4 µs to 24 µs. OSCILLATOR and FILTER 2 ranges overlap. FILTER 1's ranges miss by 708 ns (before min 107402, after max 106694). ENVELOPE's miss by 14662 ns. Those gaps are far below the 789930 ns to 844567 ns median drop on the pages that went from 386 restores to 0. The restore count on pages 0–3 did not change, so this record does not treat that small host-time movement as the lag fix.

Cold host time, one sample, nanoseconds, before → after: OSCILLATOR 227013 → 211489, FILTER 1 150767 → 135848, FILTER 2 141461 → 128278, ENVELOPE 191154 → 124356, AMPLIFIER 1194923 → 228543, ARPEGGIATOR 974302 → 162859, EFFECT 973796 → 164005, SOUND 1082861 → 177327. The four pages that dropped from 386 restores to 1 restore dropped by 810 µs to 966 µs on that single sample.

## 5. emu360

**NOT RUN.**

The gate is not in this checkout. On `origin/emu/fm1-emu-360mhz` at `6b60618c8bf344d65a166905fb746f46ab708fcb`, the script is `dnbox/emu360/bin/gate.sh`. Its header says:

```
gate.sh --fwsc PACKAGE.fwsc --elf APP.elf [--shot MS] [--bin fm1-live] [--out DIR]
```

That script's pass test is `x0x_dbg.max_us < 11609` and `x0x_dbg.late == 0`. The historical path named in the script, `/workspace/zdprobe/dnbox360/gate.sh`, is not on this machine. `/workspace/emu360-test` is not on this machine. `fm1-live` is not on `PATH`. No `.fwsc` and no `x0x.elf` were produced, because section 3 did not compile.

`cargo` and `rustc` are installed. They were not used to build a substitute emulator. An emu360 shot still needs the firmware package and ELF for each tree. An emulator result would be emulator evidence. It would not be a hardware test.

## Files this measurement touched

Committed on this branch:

- `docs/evidence/chicago-ui-lag-20261116.md` (this record)
- `docs/evidence/chicago-ui-lag-20261116-measure.c` (host harness glue)

Not committed, and not firmware edits:

- `/tmp/chicago-lag-logs/chicago_frame_measure.c` (the compiled copy; same SHA-256 as the committed harness)
- `/tmp/chicago-lag-logs/measure-with-fix` and `measure-without-fix` (host binaries)
- `build/gen/` and `build/host/chicago-og-states` in the head tree, from the oracle script and `host/build_host.sh --generate-only` (`build/` is gitignored)
- A detached worktree at `/tmp/chicago-baseline` checked out at the base SHA, used only to compile the without-fix harness. No branch was created for it. Its `build/gen/` came from the same generate command. No file in that worktree was edited.

`firmware/`, the fix commit, and the product sources were not modified.

## Risks

- The restore counts are exact across every sampled frame. The host nanoseconds are one run on one clang build. They will not match a device or a later host run.
- Pages that did not change their restore count still moved by a few microseconds to a few tens of microseconds. That is the noise floor of this host timer. The page 4–7 claim rests on the exact counts and on before/after ranges that do not overlap.
- Cold time is one sample per page.
- Native flash, pool and RAM were not measured, so this record does not show the 8,192 B pool floor for these bytes. The earlier #869 figures are not a delta.
- emu360 and `X0X_PROFILE` did not run. Nothing here is a device test, an install, or a qualification pass.
- The pixel oracle shows the cached glyph matches the legacy drawing for 256 values on the fixed tree. It does not time the frame.
