# Full-range private 808 echo experiment

The candidate restores the original 8W8 delay range of 87,944 samples at 44.1 kHz. All tested original full mixes, isolated returns and sustained tails passed. With the saved 808 route enabled, the calibrated Elite Chicago/Miami scene reached 16,145 µs with 96 late callbacks and 96 xruns on 87a2fbde. All five parts and both original owners were active. The same image with the default routing reached 11,730 µs with two late callbacks. The release uses the bounded private echo.

The longer history uses 348 transform packets of 112 bytes. A 1,536-sample uncompressed window preserves short high-feedback taps. The private mono bus lends its unused right allpass region to the transform. The 909 and 808 histories have independent write positions, read positions and returns. The combined histories and private FX must fit in the original 158,400-byte arena.

## Original audio comparisons

Apply the candidate in a disposable checkout of 2996e387. The independently measured 909 prefix-packing change was present in the native images. This patch applies before or after the 909 change. Supply the 8W8 checkout containing 94aa2716d4c2e5851fc7c5dbb4c17adfd133c997 and use new output directories.

~~~sh
git apply experiments/sound-hardware-v1/808-full-delay/full-range-delay.patch
p=experiments/sound-hardware-v1/808-effects
python "$p/delay_voices.py" ../8W8 build/full-808-voices
python "$p/delay_voices.py" ../8W8 build/full-808-sustained --sustained
python "$p/replay.py" ../8W8 build/full-808-chain --kit-master --kit-glue --kit-reverb --kit-delay
python "$p/dual_delay.py" build/full-dual
sh tests/host/drum808_echo_panel_test.sh
~~~

The regular report covers 338 mixes and 315 energized isolated returns. The sustained report covers all 16 voices plus the ensemble at four settings, with four seconds of repeated hits followed by four seconds of decay at maximum feedback. All 68 full renders, 48 energized isolated returns and 68 second-half tails met the 1 dB level and 3 dB spectral limits. The earlier 512-sample exact-history failures appear in history-and-fit.json.

## Active callback timing

Use the ordinary target toolchain to build both models. Apply active-project.patch in the disposable checkout before compiling the host project builder with the command in selected-native/README.md. The saved project routes all supported 808 per-voice sends to the private delay, selects the largest division, and sets feedback, tone and level to maximum. The five-part pattern, 922 input events and strict clock match the populated timing scene.

Stage that builder with selected-native/run.py --ensemble --stage-only. Pass the staged flash, matching ELF and package to selected-native/heavy.py with --owners 0 1 for Elite Chicago/Miami. The 87a2fbde capture includes firmware, staged-flash, emulator and audio digests. Timing comes from the PI32v2 emulator.
