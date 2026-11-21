# Synth One original factory music against the ordinary instrument

On ordinary firmware source `42ae56e1d83127fd9855fb8e86f8907af216b63a`, all 694 original programs and their effects produced measurable stereo audio. Loading the closest available settings on the shipping six-voice INIT instrument passed **0 of 694** at the 1 dB level and 3 dB spectral limits. The median spectral errors were 16.04 dB left and 15.58 dB right. Even “Artificial Unintelligence,” with spectral errors of 2.69 and 2.74 dB, missed level by 2.83 and 1.69 dB.

The original arm loads all 144 ordered frontend commands into the pinned AudioKit Synth One voice engine, runs its sequencer and executes the original effects callback at the original sample phase. The stock arm uses the ordinary firmware sources compiled on the host. It loads INIT, writes the 21 available effects controls through the project setter, checks the physical value read back after all edits, and cold-loads the saved sound on a real firmware runtime owner. These controls cover base crush rate, width, autopan, tempo-adjusted LFO rates and waves, tremolo, phaser and master dynamics. Each control uses the closest reachable physical setting. Autopan uses the original equal-power path even for zero depth. Both arms receive identical note events at 44.1 kHz, with three notes, an overlap, two releases and time for tails. Each produces 132,300 stereo frames per program, or 91,816,200 per arm.

No audio normalization or time alignment is applied. Each channel must contain signal and meet both limits. Spectral error is the RMS dB difference of frame-averaged amplitude spectra, using 512-sample Hann windows, 128-sample hops and a floor 80 dB below the original peak bin. The complete factory commands, voice, delays and reverbs remain outside the shipping instrument. Differences here include all of those contributions. The separate original-host replay and individual effect tests answer different questions.

## Replay

Generate the verified source closure and factory command pack following [the original factory fixture](../../../tests/fixtures/synthone-factory/README.md). Use `tests/host/run_synthone_factory.py --source-build SOURCE --output FACTORY_BUILD --build-only` for the verified build. From the repository root run the following with Python, NumPy, GCC and G++.

```sh
OPENBLAS_NUM_THREADS=1 python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT
```

[Summary and input digests](summary.json) · [694 per-program records, compressed JSON Lines](rows.jsonl.gz). Each record includes the original bank and index, the submitted panel positions, both stereo PCM SHA-256 hashes, and the separate level and spectral results for each channel.
