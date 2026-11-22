# Synth One noise route diagnostic

This host comparison uses the oscillator 2 candidate at the experiment's prescribed `e3559ad3` preimage. It scores the six threshold crossings against the pinned factory source at 1 dB level and 3 dB spectral error. The reference and mode-0 candidate digests were checked against the 42-control matrix for all six programs.

The observer records the original context's end-of-frame RNG state, smoothed volume, selected inverse LFO gain, generator velocity amplitude, and pitch tracking. All six use mono and zero pitch tracking. Every observed RNG change was exactly one step of the verified Soundpipe linear congruential generator. The observer leaves the original PCM bit-identical.

- Mode 0 is the unchanged raw oscillator 2 candidate.
- Mode 1 adds the experimental last-note mono and source glide implementation.
- Mode 8 adds the source noise gain and LFO route before the stock firmware amplitude envelope and filter. It maps the original frame-major LCG to the candidate voice-major spans using the previously verified affine stride.
- Mode 9 combines modes 8 and 1.
- Mode 25 uses the original observed RNG draw and activity instead of the candidate's stride, retaining the candidate velocity gain.
- Mode 57 also uses the exact original generator velocity amplitude. The excitation component now uses original scheduling, draw, gain and LFO while the rest of the voice and FX remain the firmware approximation.

The independent noise stream diverges at frame zero on four presets because the candidate clocks the initial key before the original voice starts. For the other two presets it diverges at frame 22,050 in the stock overlapping-note path and at frame 66,790 in the approximate mono path. None of the noise variants restores rows 86, 130 or 213. Mode 57 retains gained passes on rows 195 and 581 and loses row 19 on the right at 3.0063 dB. Mode 1 alone passes row 130 at 2.370/2.366 dB spectral error, while mode 57 scores 4.812/4.806 dB. The measured route does not justify adopting the OSC2 candidate or adding production noise.

After reproducing the 42-control matrix, run from the repository root with the original source build, the 42-control output directory, and a separate output directory.

```sh
python experiments/sound-hardware-v1/synthone-osc2/noise-diagnostic/run.py FACTORY_BUILD MATRIX_OUTPUT NOISE_OUTPUT
```

The fixture's source build and measured noise/gain streams are external comparison inputs.
