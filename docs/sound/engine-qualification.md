# SMK-37 sound engine qualification

PR [#985](https://github.com/gobbajogger/DnBOX/pull/985) starts at sound PR [#973](https://github.com/gobbajogger/DnBOX/pull/973), commit `63278e3322aefa89b25f129ca2fccc35ef3b3b3d`. The ordinary firmware exposes two drum kits, a BREAK sampler and four selectable synth engines. Either bass part can own any of the four synths.

## Per-engine results

| Shipping engine | Reference | Result and coverage |
| --- | --- | --- |
| 808 | Original [8W8](https://github.com/athousanddetails/schwung-8W8/tree/94aa2716d4c2e5851fc7c5dbb4c17adfd133c997) | The production release path failed 42 of 436 comparisons before the change. All 436 now pass at relative RMS error below 0.001. The worst error is 0.000614. The open-hat choke and retrigger case improved from 1.034 to 0.0000643. The comparison now compiles the shipping release behavior. |
| 909 | Original [9W9](https://github.com/athousanddetails/schwung-9W9/tree/10cbe5c1e1c9e65b0818bb34644ab1a13092082c) and its original WAVs | The matched-source render bench passed on the first re-encode. The independently decoded shipping and historical streams passed their playback and retrigger checks. Against the untouched original WAVs, all 108 controlled hi-hat, open-hat, crash and ride renders meet the 1 dB level and 3 dB spectral limits. The largest errors are 0.347 dB and 2.478 dB. A 32-path encoder and a ride-specific error weight improve original-sample fidelity at the same 55,275-byte footprint. The tighter waveform suite has 133 failures with the final ride and the original WAVs. It has zero failures on matched decoded input at the tested firmware source. [Original-WAV results and replay](../../experiments/sound-hardware-v1/909-cymbals/README.md) |
| BREAK | Original [BB Gen v2](https://github.com/mestela/schwung-breakbeat/tree/c3ea85da9025a3694c167e096cb09d0efb60420c) | Straight playback passed on all 40 slices of five loops. After the 88-frame onset ramp and accounting for the known 0–4 frame difference in slice boundaries, 694,381 PCM16 words matched. All 16 slices with source WAVs met the 1 dB level and 3 dB spectral limits. The other three loops have authenticated decoded PCM. The ordinary image starts with an empty bank. Loading a user sample makes it playable. An intact 35,335-byte CRAMEN bank alone exceeds the available application tail. [Stimulus and per-loop results](../../experiments/sound-hardware-v1/break-original/README.md) |
| Chicago | Korg Chicago AU 3.1.6 | All 81 factory patches produced finite audio on both channels. Correcting the oscillator gain curve removed a roughly 2.7 dB level error. The production overlap fix reduced strong-harmonic error from 2.211605 to 0.000409 dB and steady-envelope error from 0.755424 to 0.000017 dB. The previous-note anchor held across all 64 tested onset offsets. The 28 stationary configurations pass. The moving glide still lacks a valid pitch estimate from this fixture, and the original cold timer duration remains unknown. [Measurements and historical fixture audit](chicago-miami-reference-result.json) |
| Miami | Korg Miami AU 3.1.6 | All 30 factory patches produced finite audio on both channels. Seven original AU configurations met the published pitch, level and harmonic limits. The worst errors were 0.00484 cents, 0.379 dB and 0.048 dB, respectively. The factory audibility count covers more patches than the original-render comparison. [Measurements](chicago-miami-reference-result.json) |
| Synth One | Pinned original Synth One dry scheduler and captures | The ordinary six-voice INIT renderer passes all 180 static notes, 72 modulation notes and eight note/chord scenes. Previously a release and reattack at the same render boundary produced no new attack. It now reproduces the original retrigger at MIDI 36, 60 and 84. Original factory-bank comparisons use a separate host graph. Both parts now expose independent Synth One rate reduction, tremolo, autopan, phaser, stereo width, master compression and volume controls. A real project save, engine switch and reload restored the edited settings on both owners. The shipping path still lacks the full factory graph and bank selection. [Original PCM digests and retrigger measurements](../../experiments/sound-hardware-v1/stock-dry/README.md) |
| Juno / SuperJ6 | Pinned original-derived CoreSampler and analytic oscillator, authenticated PWM String Pad controls | All four shipping patches render finite stereo. Repeated pitches during release or sustain now reuse the ringing voice on all four. The optional factory path also clears inherited PWM and passes 298 oscillator probes plus eight authentic layer comparisons. The original build-9 release banks define all 14 selections. The 16′, 8′ and 4′ oscillator registers passed 640 combinations and 18,432 rendered frames. Complete stereo pairs measured from the AU and firmware remain at 0/14. The shipping voice and chorus optimizations reproduced the pre-change host PCM and final state at one, two and six voices. A later Linux component replay passed 36 note, 48 control and eight PWM String Pad sampler cases. The suites overlap and tap the oscillator before the filter. [Sampler results](../../experiments/sound-hardware-v1/sj6-factory/oscillator/linux-head-replay.json) [Shipping and component measurements](../../experiments/sound-hardware-v1/sj6-factory/retrigger/README.md) |

## Effects on the shipping signal path

| Engine | Original effects and available evidence | Open comparison |
| --- | --- | --- |
| 808 | All 436 original voice comparisons pass. The production KIT route now processes each voice’s own reverb send through a private reverb before kit drive and glue. All 41 full renders and all 38 energized isolated returns passed across the 16 voices, the simultaneous kit and extreme controls. Worst isolated spectral error was 0.329 dB. The original drive and glue sweep passed all 8,192 settings. [Voice and reverb returns](../../experiments/sound-hardware-v1/808-effects/private-reverb-voices-0ab227e.json) · [Drive and glue sweep](../../experiments/sound-hardware-v1/808-effects/kit-controls.json) | The private echo and reverb now precede kit drive and glue. All 12 full wet configurations pass on both channels, including combined effects. Within the 429.57 ms private delay range, 230/230 full and 207/207 isolated renders pass. Another 108 cases exceed that range. Simultaneously running 808 and 909 echoes passes all 64 tail tests. Existing saves keep the shared route until the KIT bus is selected. [Full wet chain](../../experiments/sound-hardware-v1/808-effects/bounded-echo-full-chain-03daeddb.json) · [Per-voice delay](../../experiments/sound-hardware-v1/808-effects/bounded-echo-voices-03daeddb.json) · [Concurrent tails](../../experiments/sound-hardware-v1/808-effects/simultaneous-delay-tails-03daeddb.json) |
| 909 | The pinned 9W9 host comparison exercised voice drives, reverb controls, delay divisions, master distortion and compression, and simultaneous returns. A kit hit with effects enabled had 0.000357 relative RMS error. The four-bar wet demo measured 0.000314. | Repeating the comparison with the `41eb7274` diode changes and shared glue math gave the same waveform hashes. The four-bar wet kit also passed against untouched original WAVs over 429,619 mono frames. Level error was −0.00975 dB and spectral RMS error was 0.45389 dB. Relative waveform RMS error was 0.01888. [Inputs, output hashes and scores](../../experiments/sound-hardware-v1/909-cymbals/wet-demo.json) |
| BREAK | All 28 original BB Gen effects-on comparisons passed across 2×, 3×, 4× and 8× programmed retrigger. The 22-frame crossfade also passed the click regression. All 40 straight slices retain their previous PCM results. [Measured retriggers](../../experiments/sound-hardware-v1/break-original/retriggers.json) | The original-WAV subset covers two source loops. |
| Chicago | The compact owner implements all 11 selectable inserts. Each part has a 4,096-byte history arena. Hall quantizes its allpass history and BPM delay uses a half-rate ring. [Chicago insert implementations](../../experiments/original_synth/CHICAGO-COMPACT-POLICY.md) | All 77 factory programs with an insert changed the shipping stereo output when compared with bypass. The four bypass programs reproduced identical PCM. Full wet original factory WAVs were unavailable for all 81. [Factory effects results](bass-factory-effects-result.json) |
| Miami | The implemented original-output route selects decimator, bypasses the second insert and runs the master. | All 23 factory programs with Crush changed the shipping stereo output. The shipping wheel routing passed 96 paired control and navigation cases across playback, parameter locks, both parts and four gestures. All seven bypass programs reproduced identical PCM. Full wet original factory WAVs were unavailable for all 30. [Factory effects results](bass-factory-effects-result.json) |
| Synth One | The shipping rate reducer matched the original stage at all 128 front-panel positions, including live changes, with zero stereo sample differences over 1,916,928 frames of common dry input. [Rate-reducer result](../../experiments/sound-hardware-v1/stock-dry/bitcrush-result.json). The final stereo width tap now uses a 6,615-byte history per selected owner from its existing storage. Both profiles passed 66,072 stereo frames each across 128 positions, including owner switches and cold restore. Substituting this 24-bit tap in the original host graph previously passed all 694 factory stimuli. [Production width results and reference scope](../../experiments/sound-hardware-v1/synthone-widen/stock-result.json) | Compare the shipping firmware against all 694 original factory programs with their programmed effects enabled. The ordinary image now includes the original autopan between the rate reducer and width tap. Across both owners the largest active-stage sample error was 5.22e−8. All 694 factory control vectors ran against the original stage. The 7-bit front panel rounded 501 vectors, with peak error 0.01552. The independent dual-LFO tremolo now reproduces all 694 original factory settings across 30,605,400 stereo frames with peak sample error 2.24e−8 at exact controls. The four-control stereo phaser reaches 359 wet factory settings among the 694, with maximum direct-control peak error 0.000232. Its ordinary panel-enabled populated shots pass at 11,332 µs on Elite and 11,217 µs on Pro, both zero late. The shipping graph now runs the original five-control master dynamics and volume. On 694 factory control sets and 30,605,400 stereo frames, direct controls produced peak error 2.39e−7 and worst per-program RMS error 2.80e−8. Panel controls rounded 601 sets, with peak error 0.01210. The filtered ping-pong delay, reverb and its two dynamics stages, and native factory-program playback remain open. [Tremolo qualification](../../experiments/sound-hardware-v1/synthone-tremolo/stock-result.json) · [Phaser qualification and control rounding](../../experiments/sound-hardware-v1/synthone-phaser/stock-result.json). [Production autopan and factory-stage results](../../experiments/sound-hardware-v1/synthone-autopan/stock-result.json) The full original host graph reproduced all 694 effects-on factory programs using original and compact assets. Both arms produced 66,624,000 stereo frames with zero differing samples and matched all 1,388 archived digests. [Host factory results](synthone-factory-effects-repeat.json) |
| Juno / SuperJ6 | The shipping Juno has stereo chorus. The optional factory path accepts decoded controls. | Render all 14 full presets through the original AU, including its post-engine processing. Oscillator agreement covers one component. |

## Populated-scene qualification

The ordinary Elite and Pro images at `42ae56e1d83127fd9855fb8e86f8907af216b63a` passed the calibrated default and both populated scenes. Each run used the same 922 unshifted events for eight seconds in `fm1-live` at 360 MHz with `FM1_STRICT=1`. The table includes startup. The deadline is strictly below 11,609 µs with zero late transfers and zero xruns.

| Scene | Elite peak µs | Elite late / xruns | Pro peak µs | Pro late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Calibrated default | 7,930 | 0 / 0 | 7,978 | 0 / 0 |
| Chicago and Miami | 11,492 | 0 / 0 | 11,325 | 0 / 0 |
| Juno and Synth One | 10,099 | 0 / 0 | 10,687 | 0 / 0 |

All four populated runs triggered all 11 rows in each kit, produced signal on both selected bass parts and BREAK, and finished with audible stereo output. Pro reversed the two synth owners. After multiple simultaneous drum attacks, the scheduler assigns one kit and one sounding bass to each core for the first long render stretch. The regression exercises both owner orders, short stretches, repeated single-lane triggers and worker recovery, and checks the resulting PCM against serial rendering.

The ordinary images retain 4,604 bytes of XIP on Elite and 4,616 on Pro. Both retain 2,064 pool bytes above the 8 KiB reserve. The check matched 1,923 tracked paths for each board to the measured source. [Six strict replays, native counters, PCM and package digests](../../experiments/sound-hardware-v1/selected-native/populated-42ae56e1.json)

At `87a2fbde9c58aa79d51dad7b29d9537348ee5ed8`, Elite passes both requested populated scenes. Pro passes Juno and Synth One. Its Chicago and Miami scene exceeds the deadline by 435 µs and records two late transfers. All 636 verified build inputs match this source. Elite uses 552,484 application bytes with 188 free. Pro uses 552,472 with 200 free. Each uses 96,260 bytes of normal RAM and 333,880 pool bytes, leaving 10,184 pool bytes free and 1,992 above reserve.

| Populated scene | Elite peak µs | Elite late / xruns | Pro peak µs | Pro late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Chicago and Miami | 11,554 | 0 / 0 | 12,044 | 2 / 2 |
| Juno and Synth One, INIT | 10,590 | 0 / 0 | 11,096 | 0 / 0 |
| Juno and Synth One, phaser and master compression enabled | 11,260 | 0 / 0 | 11,939 | 1 / 1 |

The four standard runs and the two additional wet runs used both synth owners, both complete kits, a loaded BREAK loop, the same 922 physical events and eight seconds at strict 360 MHz. [Standard timing results and inputs](../../experiments/sound-hardware-v1/selected-native/populated-87a2fbde.json) · [Master dynamics, six controls and wet native runs](../../experiments/sound-hardware-v1/synthone-master/stock-result.json)


At `4c1669947b29058232421a66d12b0a56282b4669`, both ordinary packages include the 808 private echo, 909 packed-delay idle fix, original Synth One tremolo and stereo phaser. All 635 checked build inputs matched this commit. Elite used 551,564 application bytes, with 1,108 free. Pro used 551,556 bytes, with 1,116 free. Each used 96,308 bytes of normal RAM and 333,808 pool bytes, leaving 10,256 pool bytes free and 2,064 above reserve.

| Populated scene | Elite peak µs | Elite late / xruns | Pro peak µs | Pro late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Chicago and Miami | 12,303 | 4 / 4 | 12,177 | 2 / 2 |
| Juno and Synth One | 10,664 | 0 / 0 | 11,167 | 0 / 0 |

Each scene used all 922 unshifted events at strict 360 MHz over eight seconds. Both kits triggered their 11 programmed rows, both synth owners rendered and BREAK played the loaded loop. The limit is 0 late and strictly below 11,609 µs. [All four captures, build identities and staged project readbacks](../../experiments/sound-hardware-v1/selected-native/populated-4c166994.json)


The four populated runs at `0ab227e2f727a9240e7df0ef972c062b5512357e` used 922 events, a strict 360 MHz clock and the full 8-second window. All 22 kit tracks triggered, both selected synth owners rendered and BREAK played the staged user loop. The counters include startup.

| Populated scene | Elite peak µs | Elite late / xruns | Pro peak µs | Pro late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Chicago and Miami | 12,336 | 7 / 7 | 12,803 | 3 / 3 |
| Juno and Synth One | 10,814 | 0 / 0 | 11,250 | 0 / 0 |

Elite placed Chicago or Juno in part A. Pro reversed the two parts. The deadline is strictly below 11,609 µs with zero late transfers. [Native captures, exact package hashes and project readbacks](../../experiments/sound-hardware-v1/selected-native/populated-0ab227e2.json)

The ordinary firmware at `93b1ad76f32b761cd2a9b04154f298c9e981093f` passed the requested Juno/Synth One timing gate on both models. Chicago/Miami exceeded the deadline on both. All four 8-second runs used 922 events at strict 360 MHz and verified both synth owners, all 22 kit tracks, the full-rate BREAK loop and nonzero final audio. Counters include startup.

| Populated scene | Elite peak µs | Elite late / xruns | Pro peak µs | Pro late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Chicago and Miami | 12,487 | 7 / 7 | 13,420 | 17 / 17 |
| Juno and Synth One | 11,003 | 0 / 0 | 11,156 | 0 / 0 |

Elite used the listed part order and Pro reversed it. [Native captures, project identities and guest counters](../../experiments/sound-hardware-v1/selected-native/populated-93b1ad76.json)

A separate optimized build at `0f019f32` produced Chicago/Miami peaks of 12,285 µs with 6 late transfers on Elite and 12,163 µs with 5 late transfers on Pro. Its Juno/Synth One peaks were 10,541 and 11,202 µs, both with zero late transfers. [Six strict measurements](../../experiments/sound-hardware-v1/selected-native/populated-optimized-results.json)

At source `3bad8d0e6acf639ce8cad1a8b7e82f6aa0b37387`, the split 808 uses whichever bass workspace is free under the selected job order. This lets bass A render while the other core finishes the metals. The Juno voice and chorus loops also avoid repeated work. The strict timing runner now requires 11 triggered rows on each kit and the expected full-rate user loop. Both ordinary development builds passed at this source. Elite occupied 547,452 bytes, with 5,220 bytes of XIP free. Pro occupied 547,440 bytes, with 5,232 bytes of XIP free. Each had 9,024 pool bytes free, 832 above reserve. The ordinary firmware at `93b1ad76` includes the independent mixer, private 808 drive and glue, and Synth One stereo width. Both native builds passed the 8 KiB pool reserve. Elite used 545,652 application bytes and retained 7,020 bytes of XIP. Pro used 545,644 bytes and retained 7,028 bytes of XIP. Both had 8,956 pool bytes free, 764 above reserve. The build inputs matched all 633 checked source, tool and asset paths. [Package and ELF identities](engine-resources-93b1ad76.json) The following historical peaks identify the tested sources.

The independent mixer on `41eb7274`, combined with private 808 glue and guarded performance-mode changes, passed 117 host callback traces. Direct controls and panel edits produced identical stereo samples and meters through all 13 tested worker masks. [Inputs, per-route PCM hashes and scope](../../experiments/sound-hardware-v1/selected-native/worker-routing/stock-routing.json)

## Timing and on-device signal in the emulator

On the earlier timing baseline, both ordinary JieLi builds fit. The measured applications occupied 546,468 bytes on Elite and 546,460 bytes on Pro. Both retained 9,024 free pool bytes, 832 bytes above the reserve. The USB reset now completes before audio DMA starts. The guest late-transfer and xrun counters include startup.

These measurements identify the combined source `29a2694ea9c64c7726ad686a218866dfb66654fa`. Both images include the 808, Chicago, Synth One and Juno corrections and the first 909 re-encode. The final ride correction at `b8384eb0b06739409be635ee2527fa9a1413574f` changes package bytes. Both ordinary models linked with that correction, retaining 6,204 and 6,212 free XIP bytes. Its native heavy-scene reruns did not return a verified result before the build environment became unavailable. Each heavy shot uses the same calibrated 922 physical input events over 8,000 ms with `FM1_STRICT=1` and `fm1-live --mhz 360`. The unshifted ordinary scene completed 647 audio halves on each model. The lane's additional 700 µs target is tighter than the requested 11,609 µs deadline.

| Initial project and synth owners | Elite peak µs | Elite raw late / xruns | Pro peak µs | Pro raw late / xruns |
| --- | ---: | ---: | ---: | ---: |
| Ordinary calibrated scene | 10,963 | 0 / 0 | 11,213 | 0 / 0 |
| Populated, Chicago and Miami | 13,587 | 72 / 72 | 15,460 | 100 / 100 |
| Populated, Juno and Synth One | 12,097 | 6 / 6 | 13,408 | 78 / 78 |

An [independent-render experiment](https://github.com/gobbajogger/DnBOX/commit/54b0b230ae2ed9db4df3ded81307b436aac1732f) rendered the parts on two cores and mixed them in the original order. The [per-model measurements](../../experiments/sound-hardware-v1/selected-native/independent-render-results.json) record 11,105 µs on Elite and 11,512 µs on Pro for populated Juno/Synth One with zero late transfers or xruns. Chicago/Miami reached 13,591 µs with 35 late transfers on Elite and 13,079 µs with 46 on Pro. This experiment predates the new private 808 effects and Chicago glide correction.

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

The Chicago/Miami command returns a failure while a required pitch, envelope or harmonic measurement fails or cannot be scored. For complete Chicago factory coverage, the project also defines a separate 81-program original capture set through `CHICAGO_AU_REFS`. For S4, the [release-bank selector](../../experiments/sound-hardware-v1/sj6-factory/release_matrix.py) reads the 14 selections from the original build-9 app. Its embedded AU supplies the reference instrument. Use the same selections, notes and timing for each firmware render. The complete stereo comparison still has 0/14 measured pairs. This run could access neither the original build-9 AU nor an online host for it. The required release SHA-256 is `dde4992f24f52d4390391069025b27092a2f327e3a53115bdd53a6ede0dffa8e`. The built-in banks supply the presets.
