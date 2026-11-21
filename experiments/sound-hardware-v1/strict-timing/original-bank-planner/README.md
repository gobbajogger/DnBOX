# Original-bank startup with load-gated Chicago planning

These measurements use ordinary Elite and Pro builds of sound-lane source `63278e3322aefa89b25f129ca2fccc35ef3b3b3d`. The experimental patch services the first nonzero block in its original DMA bank, preserves its following silent spacer and restores the foreground startup interval. Its planner keeps the active Chicago cost estimate across rests during play when the existing smoothed CPU load reaches 65%. The image shares the IMA correction table and calculates two gain ramps without changing the shipping optimization of the Chicago peaking EQ or chorus.

The matched stock controls reproduce the archived model ELFs and FWSC packages. The candidate uses 32 fewer application bytes on each board and preserves both model-specific loader bytes and package lengths. Both retain 8,972 bytes free in the pool.

`measurements.json` records the image identities, individual strict measurements, whole-stream and project hashes, and any completed raw comparisons. The saved Clip fixture exercises a first block with nonzero physical output. The 8-second scene delivers the calibrated 922 real-input events through the emulator. Each candidate uses its own protected flash region and stock-equivalent persistent data. The +1 ms raw comparison counts all 333,312 stereo frames and all 36,420 project bytes.

## Reproduction

1. Build ordinary stock from the source above in a clean checkout. In a second checkout at the same commit, apply the experimental patch with `gzip -dc candidate.patch.gz | git apply -` from the evidence directory or its absolute path. For each model, run `BOARD=smk37 MODEL=elite sh build.sh` or `BOARD=smk37 MODEL=pro sh build.sh`, preserving the matched ELF and package before switching models.
2. Extract `experiments/sound-hardware-v1/strict-timing/direct-dma-startup.tar.gz`. Run `python direct-dma-startup/check_clock.py --latched <candidate-checkout>` to exercise both DMA banks, masked intervals and 12/24 ms overruns. The tested source passed all 258 schedules.
3. Build `fm1-live` from `ba83317aa55cbcdb2830c5da97d5f64b763499e2` using Rust 1.85.1 and `cargo build --locked --release -p fm1-live`.
4. Use the source checkout's `experiments/sound-hardware-v1/strict-timing/replay.py` against each matched ELF and FWSC in separate fresh directories. Use `--offsets 1` for the focused comparison or the original six offsets for the sweep. Use `experiments/sound-hardware-v1/sj6-factory/native/score_scene.py` with the stock seed and same-offset stock reference to compare captures. These scripts validate the protected seed against the chosen image.
5. The extracted packet's `qualify.py` supplies the separate nonzero Clip startup comparison, including the 486 ms pre-deadline DMA bank and the complete 1,150 ms physical recording.

The experimental source remains unsuitable for production. The live physical audio and final project already differ from stock in the Elite +1 ms run.

At 3,181 ms in the Elite +1 ms scene, the scanner and UI key latch both read 0x400000 in each build. Both had written command 263, word `0x80009fff`, requesting a full-velocity 909 track 9 hit. Stock had consumed it (read/write 264/264), and the recorded hit time was 3,171 ms. The candidate still had it queued (263/264), and that track had no recorded hit. The two short captures each reproduced their 8-second run's physical prefix. The first differing live stereo frame is 120,896. Each of the preceding nine tracks had the same last-hit timestamp in both snapshots.

Use `contact.py --strict-dir <checkout>/experiments/sound-hardware-v1/strict-timing --emulator <fm1-live> --elf <model-elf> --fwsc <model-fwsc> --full-run <eight-second-run> --output <fresh-directory>` once for each Elite image to repeat the 3,181 ms snapshot.

## Six original input offsets

| Model | 0 ms | +1 ms | +2 ms | +3 ms | +4 ms | +5 ms | Worst margin |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Elite peak in µs | 10,103 | 10,329 | 10,127 | 10,389 | 10,431 | 10,464 | 1,145 µs |
| Pro peak in µs | 10,491 | 10,354 | 10,507 | 10,315 | 10,462 | 10,512 | 1,097 µs |

All twelve runs completed 648 halves with zero late transfers and zero xruns. All twelve live project and stereo hashes differ from their own same-model, same-offset archived references. Both saved Clip startups matched every project byte and all 30,208 stereo frames, including ten nonzero channel words on physical edge 4.

Two further foreground polling trials reproduced the original queued hit in a focused snapshot. The first changed 131,972 full-scene stereo frames and moved the first difference earlier. The second changed 129,551 frames and 13 project bytes. Both remain experimental. [Patches, contact trace and full-scene measurements](foreground-trials.tar.gz)
