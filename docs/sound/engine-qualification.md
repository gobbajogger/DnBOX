# SMK-37 sound engine qualification

PR [#985](https://github.com/gobbajogger/DnBOX/pull/985) starts at sound PR [#973](https://github.com/gobbajogger/DnBOX/pull/973), commit `63278e3322aefa89b25f129ca2fccc35ef3b3b3d`. The ordinary firmware exposes two drum kits, a BREAK sampler and four selectable synth engines. Either bass part can own any of the four synths.

## Per-engine results

| Shipping engine | Reference | Result and coverage |
| --- | --- | --- |
| 808 | Original [8W8](https://github.com/athousanddetails/schwung-8W8/tree/94aa2716d4c2e5851fc7c5dbb4c17adfd133c997) | The production release path failed 42 of 436 comparisons before the change. All 436 now pass at relative RMS error below 0.001. The worst error is 0.000614. The open-hat choke and retrigger case improved from 1.034 to 0.0000643. The comparison now compiles the shipping release behavior. |
| 909 | Original [9W9](https://github.com/athousanddetails/schwung-9W9/tree/10cbe5c1e1c9e65b0818bb34644ab1a13092082c) and its original WAVs | The matched-source render bench and exact ADPCM streaming checks pass. Against the untouched original WAVs, all 108 controlled hi-hat, open-hat, crash and ride renders meet the 1 dB level and 3 dB spectral limits. The largest errors are 0.347 dB and 2.478 dB. A 32-path encoder and a ride-specific error weight improve original-sample fidelity at the same 55,275-byte footprint. The separate, tighter sample-by-sample test detects lossy differences. [Original-WAV results and replay](../../experiments/sound-hardware-v1/909-cymbals/README.md) |
| BREAK | Original [BB Gen v2](https://github.com/mestela/schwung-breakbeat/tree/c3ea85da9025a3694c167e096cb09d0efb60420c) | Straight playback passed on all 40 slices of five loops. After the 88-frame onset ramp and accounting for the known 0–4 frame difference in slice boundaries, 694,381 PCM16 words matched. All 16 slices with source WAVs met the 1 dB level and 3 dB spectral limits. The other three loops have authenticated decoded PCM. The ordinary image starts with an empty bank. Loading a user sample makes it playable. An intact 35,335-byte CRAMEN bank alone exceeds the available application tail. [Stimulus and per-loop results](../../experiments/sound-hardware-v1/break-original/README.md) |
| Chicago | Korg Chicago AU 3.1.6 | All 81 factory patches produced finite audio on both channels. Correcting the oscillator gain curve removed a roughly 2.7 dB level error. Of 29 original configurations, 28 meet the pitch, envelope and strong-harmonic limits. Overlapping-note glide remains open at 2.212 dB harmonic error. The original pitch fit reaches its search boundary and the available transition does not identify the original anchor timer. [Measurements and historical fixture audit](chicago-miami-reference-result.json) |
| Miami | Korg Miami AU 3.1.6 | All 30 factory patches produced finite audio on both channels. Seven original AU configurations met the published pitch, level and harmonic limits. The worst errors were 0.00484 cents, 0.379 dB and 0.048 dB, respectively. The factory audibility count covers more patches than the original-render comparison. [Measurements](chicago-miami-reference-result.json) |
| Synth One | Pinned original Synth One dry scheduler and captures | The ordinary six-voice INIT renderer passes all 180 static notes, 72 modulation notes and eight note/chord scenes. Previously a release and reattack at the same render boundary produced no new attack. It now reproduces the original retrigger at MIDI 36, 60 and 84. Original factory-bank comparisons use a separate host graph. The 694-program graph and its effects are outside the shipping INIT path. [Original PCM digests and retrigger measurements](../../experiments/sound-hardware-v1/stock-dry/README.md) |
| Juno / SuperJ6 | Pinned original-derived CoreSampler and analytic oscillator, authenticated PWM String Pad controls | All four shipping patches render finite stereo. Repeated pitches during release or sustain now reuse the ringing voice on all four. The optional factory path also clears inherited PWM and passes 298 oscillator probes plus eight authentic layer comparisons. The complete stereo comparison has 0 of 14 original preset pairs available. [Shipping and component measurements](../../experiments/sound-hardware-v1/sj6-factory/retrigger/README.md) |

## Reproducing the comparisons

Run the 808 and 909 host benches against the pinned checkouts above. Use a separate working directory per native build because both models generate headers. The native smoke tests and the heavy scene use disposable emulator flash. No physical keyboard was tested.

```sh
EIGHTW8_DIR=/path/to/schwung-8W8 sh tests/host/run_drum808.sh
NINEW9_DIR=/path/to/schwung-9W9 sh tests/host/run_drum909.sh
python3 tests/host/run_a3_dsp_equivalence.py drum808
OPENBLAS_NUM_THREADS=1 python3 tests/host/bass_reference_qualification.py --out /tmp/bass-qualification
sh tests/host/synthone_init_audio.sh
sh tests/host/juno6_runtime_test.sh
sh tests/host/superj6_factory_load_test.sh
```

The Chicago/Miami command returns a failure while any original configuration misses a measured goal. For complete Chicago factory coverage, the project also defines a separate 81-program original capture set through `CHICAGO_AU_REFS`. For complete SuperJ6 comparisons the missing inputs are `J6presets.zip` and the original stereo renders for the 14-program S4 schedule. The [SuperJ6 bench](../../experiments/sound-hardware-v1/sj6-factory/README.md) records that schedule and the source requirements.
