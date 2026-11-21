# SMK-37 sound engine qualification

PR [#985](https://github.com/gobbajogger/DnBOX/pull/985) starts at sound PR [#973](https://github.com/gobbajogger/DnBOX/pull/973), commit `63278e3322aefa89b25f129ca2fccc35ef3b3b3d`. The ordinary firmware exposes two drum kits, a BREAK sampler and four selectable synth engines. Either bass part can own any of the four synths.

## Per-engine results

| Shipping engine | Reference | Result and coverage |
| --- | --- | --- |
| 808 | Original [8W8](https://github.com/athousanddetails/schwung-8W8/tree/94aa2716d4c2e5851fc7c5dbb4c17adfd133c997) | The production release path failed 42 of 436 comparisons before the change. All 436 now pass at relative RMS error below 0.001. The worst error is 0.000614. The open-hat choke and retrigger case improved from 1.034 to 0.0000643. The comparison now compiles the shipping release behavior. |
| 909 | Original [9W9](https://github.com/athousanddetails/schwung-9W9/tree/10cbe5c1e1c9e65b0818bb34644ab1a13092082c) and its original WAVs | The matched-source render bench passed on the first re-encode. The independently decoded shipping and historical streams passed their playback and retrigger checks. Against the untouched original WAVs, all 108 controlled hi-hat, open-hat, crash and ride renders meet the 1 dB level and 3 dB spectral limits. The largest errors are 0.347 dB and 2.478 dB. A 32-path encoder and a ride-specific error weight improve original-sample fidelity at the same 55,275-byte footprint. The separate, tighter sample-by-sample test reported 133 failures on the first re-encode with the original WAVs. The final ride's full pointwise count is unverified. [Original-WAV results and replay](../../experiments/sound-hardware-v1/909-cymbals/README.md) |
| BREAK | Original [BB Gen v2](https://github.com/mestela/schwung-breakbeat/tree/c3ea85da9025a3694c167e096cb09d0efb60420c) | Straight playback passed on all 40 slices of five loops. After the 88-frame onset ramp and accounting for the known 0–4 frame difference in slice boundaries, 694,381 PCM16 words matched. All 16 slices with source WAVs met the 1 dB level and 3 dB spectral limits. The other three loops have authenticated decoded PCM. The ordinary image starts with an empty bank. Loading a user sample makes it playable. An intact 35,335-byte CRAMEN bank alone exceeds the available application tail. [Stimulus and per-loop results](../../experiments/sound-hardware-v1/break-original/README.md) |
| Chicago | Korg Chicago AU 3.1.6 | All 81 factory patches produced finite audio on both channels. Correcting the oscillator gain curve removed a roughly 2.7 dB level error. The production overlap fix reduced strong-harmonic error from 2.211605 to 0.000409 dB and steady-envelope error from 0.755424 to 0.000017 dB. The previous-note anchor held across all 64 tested onset offsets. The 28 stationary configurations pass. The moving glide still lacks a valid pitch estimate from this fixture, and the original cold timer duration remains unknown. [Measurements and historical fixture audit](chicago-miami-reference-result.json) |
| Miami | Korg Miami AU 3.1.6 | All 30 factory patches produced finite audio on both channels. Seven original AU configurations met the published pitch, level and harmonic limits. The worst errors were 0.00484 cents, 0.379 dB and 0.048 dB, respectively. The factory audibility count covers more patches than the original-render comparison. [Measurements](chicago-miami-reference-result.json) |
| Synth One | Pinned original Synth One dry scheduler and captures | The ordinary six-voice INIT renderer passes all 180 static notes, 72 modulation notes and eight note/chord scenes. Previously a release and reattack at the same render boundary produced no new attack. It now reproduces the original retrigger at MIDI 36, 60 and 84. Original factory-bank comparisons use a separate host graph. The ordinary firmware now exposes the original Synth One rate reducer independently on both parts. Factory-bank playback and the rest of its graph remain outside the shipping INIT path. [Original PCM digests and retrigger measurements](../../experiments/sound-hardware-v1/stock-dry/README.md) |
| Juno / SuperJ6 | Pinned original-derived CoreSampler and analytic oscillator, authenticated PWM String Pad controls | All four shipping patches render finite stereo. Repeated pitches during release or sustain now reuse the ringing voice on all four. The optional factory path also clears inherited PWM and passes 298 oscillator probes plus eight authentic layer comparisons. The original build-9 release banks define all 14 selections. Complete stereo pairs measured from the AU and firmware remain at 0/14. The shipping voice and chorus optimizations reproduced the pre-change host PCM and final state at one, two and six voices. [Shipping and component measurements](../../experiments/sound-hardware-v1/sj6-factory/retrigger/README.md) |

## Effects on the shipping signal path

| Engine | Original effects and available evidence | Open comparison |
| --- | --- | --- |
| 808 | The 436-case 8W8 comparison covers voice processing, including its distortion types and sends. Worst relative RMS error was 0.000614. The bench mutes the original wet returns. | Compare 8W8 reverb, delay and master processing with the shared firmware bus using the same input. |
| 909 | The pinned 9W9 host comparison exercised voice drives, reverb controls, delay divisions, master distortion and compression, and simultaneous returns. A kit hit with effects enabled had 0.000357 relative RMS error. The four-bar wet demo measured 0.000314. | These measurements align the decoded cymbal inputs. The separate original-WAV cymbal results are reported above. |
| BREAK | The original BB Gen bench covers 40 straight slices. | Measure rhythmic retrigger with the original effect enabled. |
| Chicago | The original offers chorus/flanger, phaser, ring modulator, compressor, decimator, EQ, ensemble, short delay, BPM delay, hall and gated reverb. | The 29 AU configurations do not cover the full insert bank. Compare each insert at matching controls and level. |
| Miami | The implemented original-output route selects decimator, bypasses the second insert and runs the master. | Isolate decimation, wobble and stereo controls in effects-on original renders. |
| Synth One | The shipping rate reducer matched the original stage at all 128 front-panel positions, including live changes, with zero stereo sample differences over 1,916,928 frames of common dry input. [Source comparison and result](../../experiments/sound-hardware-v1/stock-dry/bitcrush-result.json) | Compare the shipping firmware against all 694 original factory programs with their programmed effects enabled. The original also uses tremolo, autopan, phaser, filtered ping-pong delay, reverb dynamics, master dynamics and widening. The full host graph has separate evidence. |
| Juno / SuperJ6 | The shipping Juno has stereo chorus. The optional factory path accepts decoded controls. | Render all 14 full presets through the original AU, including its post-engine processing. Oscillator agreement covers one component. |

## Populated-scene qualification in progress

At source `3bad8d0e6acf639ce8cad1a8b7e82f6aa0b37387`, the split 808 uses whichever bass workspace is free under the selected job order. This lets bass A render while the other core finishes the metals. The Juno voice and chorus loops also avoid repeated work. The strict timing runner now requires 11 triggered rows on each kit and the expected full-rate user loop. The following historical peaks remain the available complete four-scene baseline until the new counters return.

## Timing and on-device signal in the emulator

Both ordinary JieLi builds fit. The measured applications occupied 546,468 bytes on Elite and 546,460 bytes on Pro. Both retained 9,024 free pool bytes, 832 bytes above the reserve. The USB reset now completes before audio DMA starts. The guest late-transfer and xrun counters include startup.

These measurements identify the combined source `29a2694ea9c64c7726ad686a218866dfb66654fa`. Both images include the 808, Chicago, Synth One and Juno corrections and the first 909 re-encode. The final ride correction at `b8384eb0b06739409be635ee2527fa9a1413574f` changes package bytes. Both ordinary models linked with that correction, retaining 6,204 and 6,212 free XIP bytes. Its native heavy-scene reruns did not return a verified result before the build environment became unavailable. Each heavy shot uses the same calibrated 922 physical input events over 8,000 ms with `FM1_STRICT=1` and `fm1-live --mhz 360`. The unshifted ordinary scene completed 647 audio halves on each model. The lane's additional 700 µs target is tighter than the requested 11,609 µs deadline.

| Initial project and synth owners | Elite peak µs | Elite raw late / xruns | Pro peak µs | Pro raw late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Ordinary calibrated scene | 10,963 | 0 / 0 | 11,213 | 0 / 0 |
| Populated, Chicago and Miami | 13,587 | 72 / 72 | 15,460 | 100 / 100 |
| Populated, Juno and Synth One | 12,097 | 6 / 6 | 13,408 | 78 / 78 |

The populated projects retain the same scene and run real patterns on both kits, both bass parts and a loaded 909FL loop. On Elite the synth owners were Chicago/Miami and Juno/Synth One. Pro reversed each pair. All five guest part meters registered and all 22 kit tracks fired in each populated shot. In separate short shots, each isolated kit triggered all 11 of its tracks with the other four meters at zero. The [native per-part receipts](../../experiments/sound-hardware-v1/selected-native/results.json) also identify the tested sample, project, firmware, ELF and captured DAC stream.

| SHA-256 | Elite | Pro |
| --- | --- | --- |
| FWSC | `0893e0bbc0cc83cdb276e4ae3baa3d459dc9f9ebec99cdfefb8a05c9e1c91fab` | `6a4e38a656c2a1114ca296fd892201bde57865896d194734fd769ff4ade3589a` |
| ELF | `cd862677d32feb755e7f74adc2397fbb703684cd58ae1a69570d0f5df47ceba9` | `93ce01e75c458d67cb781a71c49e9ba2f3fc88fcb258e31ff20e761ea7c506f4` |

The emulator was built from `ba83317aa55cbcdb2830c5da97d5f64b763499e2`, executable SHA-256 `f52a3f6fb18ebe22a4c59281d441aa16b3ce5c93b008c53b538bd2383b296c57`. The [ordinary scene driver](../../experiments/sound-hardware-v1/strict-timing/replay.py) verifies the protected bytes in its warmed seed and collects the guest's actual counters. For the populated test, use the [project staging instructions](../../experiments/sound-hardware-v1/selected-native/README.md) and its sibling `heavy.py` with the two staged owner IDs.

The separate 2,800 ms physical-PLAY shot on Pro with Miami on A and Chicago on B peaked at 17,416 µs with 28 raw late transfers and 28 xruns. Setting only the saved BREAK part to off measured 17,423 µs and the same counts. Turning off the 808 measured 11,205 µs and zero late. Turning off Miami A measured 11,242 µs and zero late. A local change that let the 808 and Miami A use separate workspaces reduced the all-on case to 13,146 µs with 16 late transfers and 16 xruns. The change still missed the deadline and is not in the shipping diff. The [populated seed builder and physical playback test](../../experiments/sound-hardware-v1/selected-native/README.md) reproduce the ensemble.

The aligned space available to install a sample bank in the measured package is 6,192 bytes on Elite and 6,208 bytes on Pro. The complete designated CRAMEN loop needs 35,335 bytes including metadata. The existing user-slot route can play a full-rate loop on both models. A blank install therefore still requires a sample load for BREAK.

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

The Chicago/Miami command returns a failure while a required pitch, envelope or harmonic measurement fails or cannot be scored. For complete Chicago factory coverage, the project also defines a separate 81-program original capture set through `CHICAGO_AU_REFS`. For S4, the [release-bank selector](../../experiments/sound-hardware-v1/sj6-factory/release_matrix.py) reads the 14 selections from the original build-9 app. Its embedded AU supplies the reference instrument. Use the same selections, notes and timing for each firmware render. The complete stereo comparison still has 0/14 measured pairs.
