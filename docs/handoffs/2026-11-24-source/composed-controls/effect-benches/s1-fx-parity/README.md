# Synth One selected effects parity bench (host, stereo raw)
A headless reference/compact diagnostic, not firmware fit or audio sign-off. Original arm: verified upstream-derived Synth One oscillator/control frontend and Soundpipe effects. It is not iOS/AUv3 playback. The listening archive's preview gain is not used. Compact validation is separate stock 73749f55 DSP only; no unfinished combined candidate was rendered or changed.

## Reproduce from workspace root
Use new output directories.

    W="$PWD/s1-fx-parity"
    python fw/tools/restore-synthone-test-inputs.py --bundle fw/assets/test-inputs/synthone --output "$W/new-closure"
    python fw/tests/host/run_synthone_factory.py --source-build "$W/new-closure" --output "$W/new-reference" --build-only
    mkdir "$W/private-run"
    python "$W/build_original.py" "$W/new-reference" "$W/private-run/original"
    bash "$W/build_compact.sh" fw "$W/private-run/compact"
    python "$W/capture.py" --original-build "$W/new-reference" --original "$W/private-run/original" --compact "$W/private-run/compact" --pack fw/build/smk37-elite/fdata.img --compact-provenance '73749f55d17a0643f1ddd072114172d29eecc2ae stock host component' --out "$W/private-run/captures"
    python fx-parity-bench/score_matrix.py "$W/private-run/captures/matrix.json" "$W/private-run/scores"

The capture program verifies the authorized stock-pack and original wavetable SHA256, case names and original bank positions. Output directory is private/0700; do not commit WAV/raw/pack/build material. Retain only handwritten source if needed. To use a later compact source deliberately pass its verified source root/flags to build_compact.sh and provide actual provenance. Scope stays component + selected known factory data, not the shipped INIT-only build.

## Stimulus and arm meanings
- Shared source header specifies interleaved little-endian IEEE Float32 L,R at 44,100 Hz, no header. 128 preroll frames are run, discarded; each of six true notes lasts 33,075 frames: pitches 48/55/60/64/67/72, velocities 64/75/86/97/108/119. Final note-off starts frame 198,450. Tail lasts 66,150 frames; total 264,600. No gain, alignment, resampling, quantization or clipping. Separate processes reset state for every arm.
- 062 Irresistible Forces Pad = exact original Bonus index 21, not the same-name Red Sky program 369. 113 Dark Theater Organ = original Bonus index 72. Wet is the actual authored graph. Dry neutralizes controllable effects but retains normal master/center-pan behavior, so it is a whole-component dry diagnostic, not an oscillator-only tap.
- Delay-only and reverb-only retain that effect's authored enable and mix, neutralizing the other recognized insert effects. Organ delay is authored off and is a negative control, not an active-delay case. Wet minus dry is diagnostic, not a linear internal wet bus. In known stock compact Pad, turning reverb off changes delay decimation 47→28 due to shared memory; do not mistake the isolated-mode delta for pure kernel equivalence.
- Matrix v1 uses original-dry/original-wet/compact-dry/compact-wet, with wet capture labels referring to selected mode. Tail half-open [198450,264600), no tolerance or sign-off.

## Validation
Both drivers built on restored inputs. Two fresh runs across all sixteen captures are bit-identical per arm/case, finite and non-silent. Pad delay and reverb both differ from dry in each arm; Organ delay equals dry bit-for-bit on both arms, Organ active reverb differs. No original-vs-compact fidelity equality claimed.
