# Resolve the native Synth One cutoff-wheel math dependency

The vendor linker for the exact implemented wheel-core profile
(5b9df43b manifest) found an unresolved `powf` in the Synth One MOD
path. The source logarithmic mapping uses only 128 wheel positions.
Replace the general function with seven precomputed binary32 factors
(28 bytes), selected by the bit decomposition of the 7-bit input.
Physical range, legacy projects, output clamps and route ownership do not
change. The factors are mathematically generated from the public original
transfer curve; no private DSP/audio/preset data copied.

All 128 positions match the independent original formula within
0.001391 Hz (established DAC/path test limit 0.015 Hz). Real SMK host
ADC/USB CC1 and both owners passed 8,775 checks with no writes or lanes.
Stock vendor toolchain and locked settings on both exact board builds
now reach the linker without the undefined reference. App XIP still
overflows: 32,544 B Elite and 32,532 B Pro. This is not target fit, an
executable release, or BLE qualification. No altered flash geometry.

Apply native-scale.patch with zero fuzz after the wheel-core manifest.
Full resulting firmware manifest is `../controls-init-only/WHEEL-NATIVE-SCALE-POST.sha256`.
Use the original current Init-only settings and build command for both
models. `X0X_GEN_CACHE=0` avoids writing a cache outside the workspace.
