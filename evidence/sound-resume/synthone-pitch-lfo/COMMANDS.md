# Synth One pitch LFO replay

Production preimage `136483b5`. Use the Sound branch commit that adds this receipt. The saved route uses high bits in existing voice edit bytes 10 and 11 under the LFO-format marker and leaves the existing seven-byte master/sub/filter adjunct unchanged. The cold path applies pitch-only cells. At render, the route previews positive gains from the two existing LFO states and applies the source multiplier to OSC1 and sub. The real LFO clocks still advance through the ordinary effect path. OSC2 and FM remain outside the projected voice model. When no selected LFO has depth, render takes the ordinary dry voice path.

From the repository root with the existing verified fixture build and generated host assets:

```sh
python experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT --crush-lfo --voice --sub --jobs 5
X0X_SYNTHONE_PATCH=1 sh tests/host/synthone_init_audio.sh build/pitch-voice-test
X0X_SYNTHONE_PATCH=0 sh tests/host/synthone_init_audio.sh build/pitch-stock-test
. tests/host/original_build.sh
cc -std=c11 -O1 -DX0X_HOST -DX0X_SYNTHONE_PATCH=1 -D_POSIX_C_SOURCE=200809L -Ifirmware/src -Ifirmware/src/app -Ifirmware/src/dsp -Ibuild/gen $ORIGINAL_FLAGS tests/host/synthone_patch_test.c $ORIGINAL_SOURCES -lm -o build/pitch-patch-test
build/pitch-patch-test
```

The factory limits are 1 dB level and 3 dB spectrum per channel. The existing 39-control row matrix is the independent baseline. The two native build logs exercise the stock board package checks. Strict timing remains pending on this source.
