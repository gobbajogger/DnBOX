# Compact dry Init: signed per-layer sum before pan and headroom

Use the authenticated native same-cue anchor, the same guarded donor, and the defaults-proof raw output directory. Run bash run.sh DONOR NATIVE_OUTPUT_NPY DEFAULTS_PROOF_RAW_DIRECTORY. To recreate the third input without retaining the previous scratch directory, copy defaults-proof/render.c and native-init-ab/{cue.h,build.sh} into an empty output directory, build and run render 32 3 MODE for MODE=0,3 into defaults-MODE.f32.

The observed compact layer sums reconstruct every stereo word for both normal Init and the sustain/mute ablation with precisely the original compact Float32 order: layer gain divided by six, then center pan at 0.5. This locates a measured −21.58 dB stage in the compact path, not proof of the native mixer law. Dry compact output after this point repeats the previous known result.

Removing only the divider arithmetically in the one-layer ablation makes the held note within 0.68 dB of native, but the attack is then 13.83–21.26 dB too loud and early release 14.70–14.72 dB too loud. Held signed error is still about −9.98 dBr. A fixed gain correction is falsified as a complete fix; match source attack/release timing/state and trace the native receiver before adopting any mixer change. No code running in production was changed.
