# Full-range 808 private echo with block prediction

This experiment extends the independent mono 808 return to the original 1.994195-second limit. The ordinary Elite and Pro images fit, with 224 bytes above the 8 KiB pool reserve. The calibrated Elite Chicago and Miami capture with the private echo at maximum measured 12,658 µs, 14 late callbacks and 14 audio xruns. It remains an experiment.

Apply `full-delay.patch` to `7acc039a861da947a80f12a440fcc135e1ee76db`. The patch includes the upper-SRAM placement, binary16 allpasses on both drum buses, the separate 808 history, and the comparison changes. It keeps the shared delay and its TAPE/PING controls. The qualified builds selected the normal SMK targets. The source formatter ran on the private history header after the native images were captured.

Each 64-sample group fits in 44 bytes. Smooth segments choose a zero-, first- or second-order predictor and a shared exponent. Wideband segments store an exponent per 16 samples and use second-order quantization-error feedback. The 1,536 most recent signed samples retain their original precision. A cache decodes only the requested group. The private mono reverb's unused right allpass holds its state, read cache and first 43 groups. The other groups and recent samples occupy 62,428 bytes. All full-range time settings remain independent of the shared 909 line.

For the original 8W8 source at `94aa2716d4c2e5851fc7c5dbb4c17adfd133c997`, every whole-output, energized return and sustained-tail case passed the existing 1 dB level and 3 dB spectral RMS limits. Results were 338/338 and 315/315 on the two-second tests and 68/68, 48/48 and 68/68 on the eight-second tests. Both simultaneous lines passed 64/64 tests. All 12 independent private-chain cases passed in both channels. The unchanged shared-route reference comparison is recorded separately.

```sh
python experiments/sound-hardware-v1/808-effects/delay_voices.py ../8W8 build/full-short
python experiments/sound-hardware-v1/808-effects/delay_voices.py --sustained ../8W8 build/full-long
python experiments/sound-hardware-v1/808-effects/dual_delay.py build/full-dual
python experiments/sound-hardware-v1/808-effects/replay.py --kit-master --kit-glue --kit-reverb --kit-delay ../8W8 build/private-chain
python experiments/sound-hardware-v1/808-effects/replay.py ../8W8 build/chain
python experiments/sound-hardware-v1/909-cymbals/wet_demo.py ../9W9 build/wet-909b
```

Build each normal image and stage the active project with the existing `808-full-delay/active-project.patch` fixture. Use the selected-native 922-event heavy capture at 360 MHz, with an independent two-second boot and eight-second capture. The receipt records the firmware, seed, signal, capture-length and crash checks, and the 9W9 source differences.
