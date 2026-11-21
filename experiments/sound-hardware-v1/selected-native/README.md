# SMK selected-instrument playback

This smoke test plays a saved project on the normal SMK-37 firmware for 2,800 ms. It sends the physical sequencer PLAY press at 1,800 ms and releases it at 1,900 ms. It records the stereo PCM transferred to the emulated DAC and reads the firmware's part meters, selected synth IDs and runtime status from guest RAM. The test uses private flash copies. The separate eight-second heavy-scene qualification uses its own input and seed.

Two projects per model play bass A, bass B and BREAK at once. Each bass plays three notes at velocity 100, an octave apart between A and B. The production selector supplies preset zero of each chosen engine. The BREAK part plays the repository's 44.1 kHz 909FL loop in the second user sample slot. Both drum patterns are silent. Swapping the pairs on Pro exercises each engine in both owner positions across the two models. This covers selection and a note pattern at the DAC. Original-reference sound comparisons and the factory bank inventory have separate fixtures.

Two further projects per model play only a 909 or only an 808. The other four part meters must stay at zero. Each drum pattern cycles two of the kit's eleven voice rows per step, covering all eleven in six steps. The guest's last-hit timestamps must register all eleven rows of the selected kit. The kit meter and DAC establish part-level audibility. Separate original-render fixtures compare the individual voices.

## Measured playback

Each scene completed 188 DMA callbacks and recorded 96,768 stereo frames at 44.1 kHz. Every loaded bass reported runtime status 1 (ready). All eight captures were silent in the first half second and audible in the last. No callback crossed the limit and no DMA xrun occurred. The values are from the final ordinary Elite and Pro images with the re-encoded 909 cymbals. The [per-shot receipt](results.json) gives both package and ELF hashes, seed, project, sample, emulator and complete PCM digests.

| Model | Sounding parts | Part meters (909, 808, A, B, BREAK) | DAC peak (PCM16) | Peak callback (µs) |
| --- | --- | --- | ---: | ---: |
| Elite | Chicago A, Miami B, 909FL | 0, 0, 6326, 8253, 28672 | 13,124 | 6,996 |
| Elite | Juno A, Synth One B, 909FL | 0, 0, 1192, 2126, 28672 | 13,129 | 6,717 |
| Elite | 808 | 0, 18294, 0, 0, 0 | 13,122 | 9,102 |
| Elite | 909 | 24470, 0, 0, 0, 0 | 13,113 | 7,906 |
| Pro | Miami A, Chicago B, 909FL | 0, 0, 8327, 6326, 28672 | 13,116 | 7,223 |
| Pro | Synth One A, Juno B, 909FL | 0, 0, 2295, 1747, 28672 | 13,129 | 6,720 |
| Pro | 808 | 0, 18294, 0, 0, 0 | 13,122 | 9,098 |
| Pro | 909 | 24470, 0, 0, 0, 0 | 13,113 | 7,918 |

## Replay

Use the project's normal host dependencies, Python 3 with NumPy, GNU nm, the observer-enabled fm1-live from emulator commit `ba83317aa55cbcdb2830c5da97d5f64b763499e2`, and the matching ELF and FWSC for each model. Build the project fixture from the repository root on a little-endian host.

```sh
sh host/build_host.sh --generate-only
. tests/host/original_build.sh
test_dir=experiments/sound-hardware-v1/selected-native
project_builder=build/host/selected-instrument-project
cc -std=c11 -O1 -DX0X_HOST -D_POSIX_C_SOURCE=200809L \
  -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen \
  $ORIGINAL_FLAGS "$test_dir/project.c" $ORIGINAL_SOURCES -lm -o "$project_builder"
```

Set `ELF`, `FWSC` and `SEED` for the same model. Use the warmed X0X `seed/flash.bin` from the [strict-timing replay](../strict-timing/replay.py) with empty user slots and an ordinary, empty built-in BREAK bank. The runner replaces the protected image with the supplied FWSC. In the seed's data area it stages the production `project_save()` objects in the SMK A/B store format and fills the second sample slot using `tools/sampleio.py`. The program verifies the generated payload against the checked-in 909FL ADPCM, then requires the guest's CRC-validated catalog to identify that slot and zone as its sole loop.

```sh
python3 "$test_dir/run.py" "$EMULATOR" "$ELF" "$FWSC" "$SEED" "$project_builder" "$OUT/bass1" 0 1
python3 "$test_dir/run.py" "$EMULATOR" "$ELF" "$FWSC" "$SEED" "$project_builder" "$OUT/bass2" 3 2
python3 "$test_dir/run.py" --kit 909 "$EMULATOR" "$ELF" "$FWSC" "$SEED" "$project_builder" "$OUT/kit909" 0 1
python3 "$test_dir/run.py" --kit 808 "$EMULATOR" "$ELF" "$FWSC" "$SEED" "$project_builder" "$OUT/kit808" 0 1
```

The example uses the Elite bass pairs. For Pro use `1 0` and `2 3`. The IDs are Chicago 0, Miami 1, Synth One 2 and Juno 3. In the kit cases, 0 and 1 identify the two loaded, silent bass engines. Each output directory must be new. Add `--rescore` to verify an existing capture against regenerated project and flash bytes and the selected firmware ELF. To listen, open `stereo.s16le` as 44.1 kHz signed little-endian 16-bit stereo PCM. The first half second must be silent and the last half second must contain signal. These short scenes also check for zero late callbacks, zero DMA xruns and a peak below 11,609 µs.

## Populated seeds for the strict eight-second scene

Use the same per-model ELF, FWSC, warmed seed, builder and bass IDs as above. The staging command writes `flash.bin`, the original project objects and SHA-256 identities in `inputs.json`. A populated project retains both kit patterns, both bass patterns and all 16 BREAK steps. Use a fresh copy of each seed for every strict run. From the repository root:

```sh
python3 "$test_dir/run.py" --ensemble --stage-only "$EMULATOR" "$ELF" "$FWSC" "$SEED" "$project_builder" "$OUT/ensemble1" 0 1
python3 "$test_dir/run.py" --ensemble --stage-only "$EMULATOR" "$ELF" "$FWSC" "$SEED" "$project_builder" "$OUT/ensemble2" 3 2
```

These IDs are the Elite assignments. For Pro substitute `1 0` and `2 3`. The independent strict runner supplies the original 922-event scene. Given its output directory with `ram.bin`, `run.log` and `stereo.s16le`, `measure(elf, output, duration_ms=8000)` reads target observations without editing the project or launching the emulator.
