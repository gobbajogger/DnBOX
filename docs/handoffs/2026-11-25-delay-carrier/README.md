# Delay: carrier loss versus timing, 2026-11-25

**Hold both changes out of release.** No device writes or shared pool/layout changes. The same fixes are not assumed to work for both engines.

## SuperJ6

Isolated stereo C host bridge built from the exact owner-token guarded checkout (unchanged delay include SHA-256 0e37c63cdef9d366cdf3def539fc0c925f80511635da8eca7a39e0ed66f9ea18). The comparison reference is the previously verified Build9 kernel port, not a fresh full AU graph. At 44.1 kHz, 250 ms, 0.499 feedback and 0.9 wet, tests keep the exact carrier separate from total echo energy. Compact stores one cell per 16 frames; the 2,756.25 Hz update rate folds 3 kHz to 243.75 Hz and 6 kHz to 487.5 Hz. Measured first-echo peaks were 243.6 and 487.2 Hz. Their true carriers were about 66 dB down. A higher broadband echo level was not evidence of preserved treble. Opposite-channel isolation and bit-exact bypass passed.

The separate center-only patch fixes the mean delay time on the bench. For the 250 ms 1 kHz echo, phase error improved from −61.224° to +0.001°, while level error stayed near −5.9 dB and aliasing stayed present. A 128-phase impulse sweep halves worst centroid error at the tested long strides. All 7,680,000 short-delay float outputs matched baseline. Both guarded native diagnostic links have unchanged headroom (Elite +952 B, Pro +964 B app; both +564 B pool over its base reserve). These are linker results only; no loaded native timing or audio gate claim. Geometry-only experiments are not proposed for inclusion; see the original host readout.

Run the existing sj6-fx-bench/build-bridge.sh on the exact source, then pass bench.py, the bridge, sj6-multirate and the exact delay include to repeat-spectrum.py. Run stock and again with center-only.patch applied. Native links use the same guarded owner-token donor build inputs, flags and admission checks as the 2026-11-25 DNR3 owner-identity checkpoint.

## Synth One

Narrowing the recording window keeps the 2,152 B delay arena and left all six host dry renders exact. The Pad full-tail level improved from −6.299 to −1.120 dB, but waveform residual worsened, and Sahara and Atmostrings moved further from their original levels. A controlled synthetic probe shows that a 3 kHz input becomes a 60 Hz alias in the d45 ring and that shortening the window boosted that alias by 23.645 dB. It did not recover the original treble. Reject this variant. Source and exact repair experiment are in s1/; no preset audio or binaries included.
