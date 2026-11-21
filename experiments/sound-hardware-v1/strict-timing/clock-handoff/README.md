# SMK startup spacer and strict replay

Both candidates move startup handling out of the recurring audio interrupt. The original-bank first block is rendered during the USB reset interval. The initialized opposite bank supplies the following silent hardware transfer. The regular IRQ acknowledges the active release before rendering so that a second release stays pending even after two bank transitions. The playback planner retains the measured active Chicago estimate across a rest only above 65% average load. The two integer gain ramps follow the preceding edge-settle candidate.

`foreground.patch` masks ALNK0 while the foreground waits up to one half for the spacer. `vector.patch` installs a one-use ALNK0 handler after the early render. That handler waits for an actual hardware release and reinstalls the ordinary vector before returning. TIMER5 and the foreground continue independently.

| Link | Elite app bytes | Pro app bytes | Elite / Pro package bytes | Free pool per board |
| --- | ---: | ---: | ---: | ---: |
| Stock | 546,516 | 546,508 | 651,732 / 651,730 | 8,972 |
| Foreground mask | 546,564 | 546,556 | 651,732 / 651,730 | 8,972 |
| One-shot vector | 546,612 | 546,604 | 651,732 / 651,730 | 8,972 |

The linked loaders match stock byte for byte in both variants. The foreground build exceeds stock application use by 48 bytes and the vector build by 96 bytes. These increases disqualify both under the unchanged-flash requirement.

## Physical input and the first differing sample

| +1 ms physical scene | Peak µs | Halves | Raw / scene late | Changed project bytes | First different stereo frame |
| --- | ---: | ---: | ---: | ---: | ---: |
| Foreground mask, Elite | 10,387 | 648 | 0 / 0 | 2 | 97,856 |
| One-shot vector, Elite | 10,535 | 648 | 0 / 0 | 10 | 97,856 |
| One-shot vector, Pro | 10,339 | 648 | 0 / 0 | 4 | 97,856 |

Each comparison above used the same 922 physical events. All three candidates reproduced all 30,208 frames of the saved Clip startup, including the first nonzero transfer on edge 4. Each complete live stream diverged. [Complete native and build receipts](result.json)

The rebuilt public observer reproduced the original stock +1 ms result on both boards. The Elite and Pro peak times were 11,323 and 11,318 µs. Each stock run completed 648 halves with one raw boot late. Each board's 333,312 stereo frames and 36,420 project bytes matched its own historical reference byte for byte.

On Elite the foreground mask measured 10,387 µs with zero raw and scene late and 648 complete halves. It changed two project bytes. At offset 2,801 the 909 mask changed from 255 to 127. At 3,352 the bass B pattern length changed from 35 to 32. The vector candidate reached 10,535 µs, zero raw and scene late, and 648 complete halves. Ten sound-setting bytes differed. Each candidate matched the complete saved Clip startup, including the original first generated bank before deadline. At the first differing live stereo frame, 97,856, stock emitted (−28, −28) and both candidates emitted (−667, −667).

[A separate 2,650 ms capture](contact-2650.json) locates the Elite vector failure ahead of the synthesizer. Both scanners showed the same physical note bit, 0x400. Stock's UI had already latched the bit and its 909 track 4 showed velocity 255 with its last hit at 2,649 ms. The vector UI still held zero and track 4 had no hit. Both shorter wire captures exactly matched the corresponding prefixes of their full runs. For this source layout `fm1_in.notes` starts at offset zero and `ui.keys` at byte 112. The stock observation was independently located in the [earlier native observer trace](../direct-dma-startup.tar.gz).

To reproduce the snapshot from a completed full run, use `python3 contact.py --packet "$PACKET" --emulator "$FM1_LIVE" --elf "$ELF" --fwsc "$FWSC" --full-run "$RUN/stock" --output "$NEW_DIR"`. Use the corresponding candidate files and `"$RUN/candidate"` for the other arm.

### Testing an additional input poll

A third Elite build checks for a changed physical key after each of the header and main draw sections when the sequencer is stopped. It calls the existing rate-limited foreground input dispatcher. Apply [the UI change](poll-during-draw.patch) to the one-shot vector variant. Its app is 546,660 bytes, with 8,972 free pool bytes and the stock package length. The fresh, warmed 2,710 ms replay matched all 99,840 stereo frames. [Measured replay](poll-result.json)

The subsequent 8,000 ms replay peaked at 10,471 µs with 648 halves, 0 raw late and 12 changed project bytes. In that capture its first DAC difference moved to stereo frame 102,464.

## Stopped-clock control

The test compiles the actual candidate boot code, audio interrupt and foreground handoff into a C harness. It exercises both initial banks, zero, one and two releases during USB settling, the initialized spacer and overruns lasting 12 and 24 ms. A separate check pauses the DMA clock after the first block and resumes it after 50 ms. The foreground timeout writes block 2 into the bank for the silent spacer. The one-shot vector keeps that bank zero, then delivers block 2 at the following release. Both bank polarities reproduce the distinction. Each candidate was exercised on 260 schedules. This control models complete bank transitions and pending flags. Native output comparisons use the strict emulator below.

## Reproduction

Use firmware commit `dea023ce632538621b59bb307ebcaf440805b032` for stock. Apply a single patch to another clean checkout at that commit and build each model separately with the project's pinned JieLi toolchain and AC79 1.2.14 SDK. From this directory the controlled-clock command is `python3 check_clock.py /path/to/patched/checkout`. The native outputs come from `BOARD=smk37 MODEL=elite sh build.sh` and `BOARD=smk37 MODEL=pro sh build.sh` in both checkouts.

The archived reference runner has SHA256 `04c795b1961298cbd10d4bd9881ab97fde5eef7ff179f49a6b953dab78d43dc8`. The rebuilt observer runner uses the archived dropout-observer tag, commit `ba83317aa55cbcdb2830c5da97d5f64b763499e2` in the same repository. With Rust 1.85.1, the checked-in Cargo.lock, ALSA, X11 and Wayland development libraries, run `cargo build --locked --release -p fm1-live` there. The native executable used here has SHA256 `8dca639e2f8b0a992f7e4581563d3497e664c8331ffa86a2e70961e1922076ac`. Each available stock calibration in [result.json](result.json) compares the output of this binary to the original measured timing, scene, initialized flash, original ELF and package, complete RAM project and physical PCM.

Extract [the original qualification driver](../direct-dma-startup.tar.gz). Use a fresh output directory and matching board-specific ELF and package files.

```sh
python3 "$PACKET/qualify.py" --emulator "$FM1_LIVE" \
    --stock-elf "$STOCK_ELF" --stock-fwsc "$STOCK_FWSC" \
    --candidate-elf "$CANDIDATE_ELF" --candidate-fwsc "$CANDIDATE_FWSC" \
    --output "$RUN" --offsets 1
```

The 8,000 ms scene shifts all 922 physical events by +1 ms. The driver captures a fresh factory flash per image, warmed boot counters, the saved Clip startup and live stereo/project comparisons. The runner uses strict dual-core simulation at 360 MHz. Every reported time is a target timer measurement.

The 2,710 ms follow-up uses the same 922-event input file and a fresh warm flash from the candidate's own 2,000 ms factory run. Only inputs before the recording deadline are consumed. The check compares each available output frame against the same prefix of the saved 8,000 ms stock capture. A passing prefix starts a full 8,000 ms run from a second copy of the same warm flash.
