# Joined sizing and timing trial, e00f689d
This is not an installable release. Neither engine compatibility nor real-time heavy playback has passed. SuperJ6 work is parked.

## Exact edits and architecture
`worker-typed-s1-only.patch` is a full replacement for earlier incremental S1 patches, relative to e00f689d. It includes exact Room store/read, common input and held-envelope shortcuts, bounded normalized oscillator phase, static pitch work avoidance, early stationary poly admission, and reuse of a successfully retired native job slot. The typed worker lives in the existing 5,120-byte shared B workspace; compile-time check prohibits increasing it. An empty or unacknowledged peer is not a successful handoff. Jobs that do not meet the explicit qualification retain complete serial rendering. Host proof across 694 presets plus INIT means identical to the existing compact engine, not bit equality with the original app.

Native worker timing flags: `-DDNBOX_S1_WORK_SPLIT=1 -DDNBOX_S1_SPLIT_START=1 -DS1C_SPLIT_SPAN=112u`. Use normal JieLi flags, no reassociation and no contraction. Only `s1c.c` and `s1u.c` go to `-O3` (do not apply globally). See `build-joined.sh`.

Display patches prepare both native bands before the first foreground callback. Staged LZ + exact Chicago FAST32 and the three cold code savings are separate mechanical changes. The display fit and order are measured, not an all-project codec-capacity, complete UI responsiveness, or audio-performance certification.

| Actual joined link & test | Elite | Pro |
|:---|---:|---:|
| Spare native app XIP | 4,844 B | 4,852 B |
| Spare audio pool above unchanged 8,192 B reserve | 27,392 B | 27,392 B |
| Synth One B six held notes, no drum kits, peak | 11,526 µs | 11,531 µs |
| Same solo with late flags / long stale repeat spans | 0 / 0 | 0 / 0 |
| Six held notes + both sequenced drum kits, peak | 18,646 µs | 18,685 µs |

Solo still misses a 100-µs margin against the physical ~11,610 µs deadline. Both kit runs had real repeated PCM and 324 completed halves, not a zero-dropout result. The normal owner-A typed test before display measured 11,778 µs and stale repeats, so the two solo B passes do not guarantee slot A or arbitrary note timing. The strict load evidence verifies six actual Synth One gates as well as USB stimulus and no emulator skipped cycles.

## Bench
Create model-matched virtual seed using `seed.sh SOURCE BOARD RAW_PACKAGE NEW_NOR`. It carries the exact three generated build companions and the existing family marker. For dry setup, run `SYNTH_OWNER=B build-project.sh SOURCE NEW_SAVE` from a scratch checkout. For kit setup use `SYNTH_OWNER=B SYNTH_KIT=2 build-project.sh SOURCE NEW_SAVE`. Supply this file as `PROJECT` and the exact local seed as `SEED` when invoking `replay-solo.sh` (no kits) or `replay-kits.sh` (starts sequencer). For B send the six gate-on messages `4400:923058;4403:923458;4406:923758;4409:923b58;4412:923e58;4415:924358`. Use the model-matched local controller. Run `receipt.py LINKED_ELF RESULT_DIRECTORY 1`.

On the same solo B replay, selective `-O2` gave 11,876 µs peak, 380 completed and 10 stale spans. Early admission (allowing 3+ stable acquired voices rather than waiting for all six) cut the size-optimized staggered attack peak from 15,839 to 13,744 µs. At the heavy baseline, borrowing the peer only after the completed drum work acknowledged lowered peak 29,490 → 24,474 µs before tuning compiler placement. Both were still unsafe as product builds.
