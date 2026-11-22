# Factory oscillator register

The release control `neonoscRange` (ID 86) selects 16′, 8′ or 4′ for both oscillator layers. The three normalized values 0, 0.5 and 1 correspond to −12, 0 and +12 semitones. A subsequent pitch bend adds to that interval. The foreground loader stages the first control word together with the waveforms and envelope. A missing control selects 8′.

The archived factory metrics identify 640 distinct bank/index pairs across 12 banks. Of these, 192 use 16′, 374 use 8′ and 74 use 4′. The CSV SHA-256 is `14fd2646763d3b718c307e9b8cd59e4876351b56f162bbf74f439e2c263259c8`. These are register settings only. Every row's original `bench_render` field is `NOT_RUN`.

```sh
git fetch origin refs/pull/952/head
python3 experiments/sound-hardware-v1/sj6-factory/register/replay.py
```

The replay reads the pinned Git object and checks its SHA-256. It passes one archived word at a time through the firmware factory loader on each of two owners and inspects both layers. The other controls in those test rows use fixtures. A separate audio test compares the register at MIDI 60 against transposed MIDI input with the neutral register. Each arm plays eight 128-frame stereo blocks and a performance bend begins on block four. The 18,432 stereo frames matched exactly. All 96 nonneutral blocks differed from the unshifted control. Maximum pitch error across the nine original-derived CoreSampler increments was zero binary32 units in the last place.

The note increments at 44.1 kHz come from the independent `sampler_reference.cpp` in the sampler harness at commit `2fe5930c161a30c907a4b91991ab3c9724380b10`. Keys 46/48/50, 58/60/62 and 70/72/74 cover the three registers and −2/0/+2 semitone bends. The reference oscillator voice has a neutral envelope, modulation and filter. Full stereo program comparisons still require release program records and original instrument output.
