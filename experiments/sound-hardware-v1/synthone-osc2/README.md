# Experimental Synth One oscillator 2

This experiment is based on DnBOX `e3559ad37c354c02d49f8b99c68fa1982b2ec4b9`. The archived `raw-osc2.patch` is an opt-in implementation candidate for an isolated checkout. It implements four raw waveform knots, continuous interpolation and an independent phase, with static tune and detune. The four new cell controls are off by default. Its V3 adjunct fits inside the existing 32-byte cell and accepts V1 and V2. The candidate omits oscillator bandlimiting and detune LFO and does not qualify as a full factory implementation.

The 42-control matrix read back 29,148 controls and reproduced all 694 pinned original stereo PCM hashes. It rendered 132,300 frames per program and scored each stereo channel at 1 dB level and 3 dB spectral error. Seven programs pass both channels before and after oscillator 2. Rows 19, 195, and 581 gain a pass. Rows 86, 130, and 213 lose one. Three additional programs become audible and all 66 zero-oscillator-2-level firmware renders are bit-identical to the 38-control matrix. The complete scores and PCM digests are in `factory-42.jsonl.gz`. The result receipt includes the per-channel metrics and threshold crossings.

The independently authored raw waveform evaluator matched 3,145,728 original samples bit for bit, including wraps and interpolation. The reference table SHA-256 is `0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5`. Focused host tests cover all controls and pot positions, legacy cells, save/load and both owner note-lock paths. The raw implementation patch SHA-256 is `2154ba93f3d534a285e0f98b2aac855b17c87a30b889cad94355373048fb8fe4`.

## Reproduce

Use the verified original build from [the factory fixture](../../../tests/fixtures/synthone-factory/README.md). Apply `raw-osc2.patch` to a separate checkout of the stated base, with this directory available. From the repository root run the standalone harness.

```sh
git apply experiments/sound-hardware-v1/synthone-osc2/raw-osc2.patch
python experiments/sound-hardware-v1/synthone-osc2/replay.py FACTORY_BUILD OUTPUT --crush-lfo --voice --sub --osc2 --jobs 5
cc -O2 -std=c11 -ffp-contract=off -Ifirmware/src/dsp tests/host/synthone_osc2_reference.c -lm -o osc2-reference
./osc2-reference PUBLIC_MORPHS_F32
```

The archived matrix came from a scratch copy of the same projection harness and previously built original/control executables. The scratch producer selected the same 20 voice controls via the old `S1_PROJ_SUB` macro. The standalone harness selects these with `S1_PROJ_OSC2`.

## Diagnostics

Both diagnostic suites use rows 19, 86, 130, 195, 213 and 581. The candidate zero mode reproduced each 42-control firmware digest. The untouched original arm reproduced the pinned original digests. Score thresholds, stimulus and measurement are identical to the full matrix.

- `diagnostics/crossing-study.json` compares raw OSC2, experimental last-note mono with the source glide formula, authored raw OSC1 waveform and tune, and both changes. Mono alone restores row 130. Raw OSC1 alone restores row 213. Neither restores row 86. Combining both restores none.
- `diagnostics/exact-poly-study.json` changes only the original reference's `isMono` using the original control adapter. The 42-control firmware matches row 86 in this comparison. Rows 130 and 213 still fail and all three gained passes remain.
- `filter-diagnostic/filter-study.json` injects sample-rate LFO gains observed directly from the original into the compact firmware filter for cutoff and envelope depth. Mode 4 is this injection alone. Modes 5, 6 and 7 combine it with mono, authored OSC1 or both. The injection alone restores row 130, does not change row 86, worsens row 213, loses the gains on rows 19 and 195, and retains the gain on row 581. It does not justify a production filter/ADSR change.
- `diagnostics/noise-stride.c` compares a voice-major affine RNG stride with the pinned original Soundpipe `base.c` and `noise.c`, which initialize the context seed to zero and consume noise even at zero gain. The stride matches the serial original bit for bit at each of 3,698,820 sample positions in 8,192 variable blocks. This holds when the active set and order match the oracle. Firmware scheduling, noise modulation and full factory parity remain open.

Diagnostic replacement sources are retained for comparison only. Their firmware host adapters replace the two same-named app/DSP files in the host source list used by `replay.py`. The original observers replace the fixture's `preset_audio.cpp` input as in `compile_producers`. The original oracle source is separate from the firmware implementation.
