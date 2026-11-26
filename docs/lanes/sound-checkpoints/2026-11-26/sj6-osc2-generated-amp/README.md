# Guarded current SuperJ6 generated OSC2 gain

This comparison-only change applies the observed current generated OSC2 gain
at the post-halfband multiply. It does not change the legacy analytic route
or the historical pre-halfband experiment. It requires the existing
`DNBX_HOST_MAIN_SOURCE_ORACLE` typed source witness, enabled per-lane Normal
projection, generated saw selection, and no scale-inside-halfband ablation.
It adds an explicitly opt-in host flag; production behavior is unchanged.

Current original ARM64 execution found exact Float32 post-halfband gain
`AMP * VoiceGain` on 2,048 onset and 13,056 held signed output words. The
extra historical factors 0.5 and 0.68 make every tested word differ, by
−9.37042 dB on this isolated branch. This tests the generated source branch
only. It does not explain the separate ~10 dB current receiver-to-Mac terminal
gain gap or prove full native parity. Do not remove the factors globally.

## Reproduce the guarded compile

The DnBOX source donor (58 kB, SHA starts bd5a59c1) and matching include
pack are preserved as `dnbx_core.c` and the exact compile header archive
in ChatGPT Library. Their exact SHA and instructions are in
`compile-inputs.txt`. Run `bash verify.sh /path/to/dnbx_core.c /path/to/headers`.
This proves an identical default-disabled object and successful opted-in
compilation. It does not replace the executable original stage taps;
see `stage-proof.md` for those details.
