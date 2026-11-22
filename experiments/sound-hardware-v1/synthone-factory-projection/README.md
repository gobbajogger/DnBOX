# Synth One original factory music against the ordinary instrument

On ordinary firmware source `42ae56e1d83127fd9855fb8e86f8907af216b63a`, all 694 original programs and their effects produced measurable stereo audio. Loading the closest available settings on the shipping six-voice INIT instrument passed **0 of 694** at the 1 dB level and 3 dB spectral limits. The median spectral errors were 16.04 dB left and 15.58 dB right. Even “Artificial Unintelligence,” with spectral errors of 2.69 and 2.74 dB, missed level by 2.83 and 1.69 dB.

The original arm loads all 144 ordered frontend commands into the pinned AudioKit Synth One voice engine, runs its sequencer and executes the original effects callback at the original sample phase. The stock arm uses the ordinary firmware sources compiled on the host. It loads INIT, writes the 21 available effects controls through the project setter, checks the physical value read back after all edits, and cold-loads the saved sound on a real firmware runtime owner. These controls cover base crush rate, width, autopan, tempo-adjusted LFO rates and waves, tremolo, phaser and master dynamics. Each control uses the closest reachable physical setting. Autopan uses the original equal-power path even for zero depth. Both arms receive identical note events at 44.1 kHz, with three notes, an overlap, two releases and time for tails. Each produces 132,300 stereo frames per program, or 91,816,200 per arm.

No audio normalization or time alignment is applied. Each channel must contain signal and meet both limits. Spectral error is the RMS dB difference of frame-averaged amplitude spectra, using 512-sample Hann windows, 128-sample hops and a floor 80 dB below the original peak bin. The complete factory commands, voice, delays and reverbs remain outside the shipping instrument. Differences here include all of those contributions. The separate original-host replay and individual effect tests answer different questions.

On firmware `7acc039a861da947a80f12a440fcc135e1ee76db`, the additional route maps original parameter 58 to either LFO or their average. All 694 original-versus-shipping stereo comparisons were evaluated at 22 available panel settings. Both thresholds together passed on 0/694. The 580 programs with that route off produced the same stock stereo hashes as the 21-control run. For the 114 routed programs, both arms were regenerated and every original digest matched the first run. [22-control summary](summary-22.json) · [694 program scores](rows-22.jsonl.gz).

## Editable voice and effects projection

On source `6e3055e52d4cfc70c5af472d0727eb82b4efa882`, the optional voice build read back all 24,290 edits across 694 factory programs. The original arm reproduced all 694 archived stereo PCM hashes. Both arms rendered 91,816,200 stereo frames, and zero programs met both the 1 dB level and 3 dB spectral limit on both channels. Median spectral error was 12.94 dB left and 11.88 dB right.

The firmware rendered 78 programs silent. Forty-four had zero oscillator 1 level, 32 selected oscillator 2 exclusively, and two did both. The production voice currently derives excitation from oscillator 1, weighted by the balance control. The full original renders for those programs contain audio. The closest stereo pair, “Who Are You?”, met each level limit and missed the spectral limit by 0.260 dB left and 0.250 dB right. [Run receipt](voice-35-result.json) · [All per-program results](rows-35.jsonl.gz).

## Replay

Generate the verified source closure and factory command pack following [the original factory fixture](../../../tests/fixtures/synthone-factory/README.md). Use `tests/host/run_synthone_factory.py --source-build SOURCE --output FACTORY_BUILD --build-only` for the verified build. From the repository root run the following with Python, NumPy, GCC and G++.

```sh
OPENBLAS_NUM_THREADS=1 python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT
OPENBLAS_NUM_THREADS=1 python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT_22 --crush-lfo
OPENBLAS_NUM_THREADS=1 python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT_VOICE --crush-lfo --voice --jobs 5
OPENBLAS_NUM_THREADS=1 python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT_SUB --crush-lfo --voice --sub --jobs 5
```

The optional voice run takes its ordered factory IDs from the compiled firmware descriptors. It chooses the closest physical edit for each original value, writes the production sound cell, cold-initializes a normal runtime owner, and checks the applied value. It uses the same 132,300-frame stimulus, original renderer, and stereo acceptance limits as the effects-only comparison. Each row lists effects positions first, followed by voice positions in the summary's `projected_voice_ids` order. The parallel worker count changes only host render scheduling. Rows are written in factory order.

[Summary and input digests](summary.json) · [694 per-program records, compressed JSON Lines](rows.jsonl.gz). Each record includes the original bank and index, the submitted panel positions, both stereo PCM SHA-256 hashes, and the separate level and spectral results for each channel.

The sub option applies the three additional persisted controls. [Sub oscillator measurements](../synthone-sub/README.md).
