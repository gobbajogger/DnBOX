# Combined native software qualification

Replay on the production-source composition at Integration candidate `156f4629451457b963c98776715ecf3d97abd5e5` and Sound `bb836379f561f83c07ecc94ea336f09e00fc96a6`. The Integration lane head checked for ancestry was `d63620b9253fc3656c5a9200be3f4936ddb83bc1`. This replay is a separate software composition.

Apply the Sound firmware diff from `0ac658008639c8a27ce38facd6d058a3fbc60744` through `bb836379`, excluding `firmware/src/app/engine.c`. Use `git apply --unidiff-zero` with [the composition patch](composition.patch) for that file and the Integration-specific build profile. Its portion selecting `-Oz` for 808 is available separately as [808-size.patch](808-size.patch). Leave the prior 909 and BREAK size settings and guarded Remote producer enabled.

Use the pinned Jieli and AC79 SDK. Build each model with these flags:

```sh
X0X_DEV_PADCAL=0 X0X_COMPOSE_S1_EFFECTS=1 X0X_TANH_BOOT_TABLE=1 \
X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_FACTORY=1 X0X_DNR2_NATIVE=1 \
X0X_INTEG_LATEST_SOUND_FIT=1 X0X_DNR2_SCRATCH=1 X0X_INTEG_DRUM_SIZE=1 \
MODEL=pro ./build.sh
```

Change `MODEL` to `elite` for the other package. The published [native profile](../native-profile.json) pins the ELF, FWSC and emulator flash image hashes. Run the strict 360 MHz calibrated populated playback in `experiments/sound-hardware-v1/selected-native`. Use owners 0 1 and 3 2 on Elite, and 1 0 and 2 3 on Pro. Stage highpass and full-level sub for the Juno/Synth One pair. Apply the existing `activate_mode2.py` offline fixture to the composed engine for the separate wet tests. Every replay requires a firmware-matched project seed and a mode readback on the intended owner for wet qualification.

A clean Git reconstruction preserved Integration’s legacy Juno control helper expressions. It produced byte-identical stock and instrumented ELF and FWSC files for both boards. All six repeated emulator captures matched the reported results and PCM hashes. The [reconstruction receipt](reconstruction.json) pins those comparisons and the exact ancestry.
