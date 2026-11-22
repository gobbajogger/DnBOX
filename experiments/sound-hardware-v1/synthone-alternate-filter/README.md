# Synth One authored filter routes

Original filter type ID 115 selects lowpass, bandpass or highpass. The INIT and legacy editor state select lowpass. The two new routes use the authored Butterworth response in a topology-preserving state-variable realization and the ordinary firmware's existing cutoff envelope and resonance control. The coefficient cache and two delay words reuse the existing voice state. The adjunct retains its seven-byte size and both the previous master and sub layouts. Changing routes resets only filter history because the histories use different coefficients and state definitions. Filter cutoff changes reuse the selected route's state.

The 39-control host replay rendered 694 stereo programs against the pinned original, verified all original PCM digests, and read back 27,066 control targets. It preserved all 598 lowpass firmware digests, including all seven prior stereo passes. It gained one previously failing program. JEC Solina Strings Bright (352) passed at +0.078/+0.226 dB level and 2.858/2.616 dB spectral error. Level and spectral limits were 1 and 3 dB. The result is 8 of 694 programs. The complete program comparison still includes other missing original graph elements.

The production-path host tests exercised all 128 selector positions, both owners, the legacy and sub layouts, rejection of single-bit marker corruption, project save and reload, and restoration across sequencer note locks. A 5,512,500-sample regression switches repeatedly between 64 Hz and 22,050 Hz and toggles full filter envelope depth at five resonance settings. The largest sample magnitude was 0.724287 for bandpass and 1.333182 for highpass. This check caught an unstable direct-form predecessor before admission.

Generate the verified original fixture described in [the factory fixture README](../../../tests/fixtures/synthone-factory/README.md), then run the following from the repository root with Python, NumPy, GCC and G++.

```sh
python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT --crush-lfo --voice --sub --jobs 5
sh tests/host/synthone_patch_test.sh
```

The archived host matrix identifies the implementation patch against `e3559ad3`. It does not measure the two board deadline or flash margin.
