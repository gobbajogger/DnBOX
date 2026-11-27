# Combined source-backed Raw reference

The source/control bench uses the same authenticated 44.1 kHz Raw dry/Chorus I tape and the actual current original components. It preserves the pinned callback and note chronology. The new original-Sub check executes the original tuning-model path with default tuning and confirms exactly 440 Hz, the source region selection and fractional cursor, and the physical `.808 × raw` Sub fader. An SFZ inclusive-end correction does not change this cue. No retune, phase shift, or PCM fitting was applied.

At the branch boundary the previous fixed Main hypothesis is retained. An independently frozen 2× Sub diagnostic, determined from the isolated first source contribution and never fitted to the combined render or its release, yields these whole results:

| Signed residual | Prior | With both fixed branch transforms |
|---|---:|---:|
| Dry | −13.233 dBr | −23.595 dBr |
| Authored | −11.883 dBr | −23.053 dBr |

Whole level errors are +0.0202 dB dry and +0.0165 dB authored. First 64 frames are −62.67/−75.28 dBr. First release owner is −24.62/−18.21 dBr, first after-owner −16.24/−15.40, final release owner −17.32/−18.05, final after-owner −12.48/−13.26. Complete reference match is not accepted. These are isolated source hypotheses, not a proven mixer level correction or permission to change the master.

The genuine original Sub waveform remains separately sampled. Actual media is not resent or included in the patch. The shipping engine must still use the independently authored synthesized carrier and meet signed holdouts, timing, and memory. Active work: per-note final Sub trajectory/release owner, exact compact J60 converter source, and synthesis implementation.

Exact source-only archive SHA-256: `9020b88e8aadfc4be1055bbc7ed57f2d606b566afd97d75bfbaa2ae152689af4`. No device, firmware install, or release.
