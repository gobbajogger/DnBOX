Transposed cascade uses exact same allpass transfer with 8 history words not 16
and keeps four source-style F32 phase updates. Stage cases include
actual notes 45,57,72,81,93,105, sign-reversed impulses, independent
sine/triangle, live bend and post-input silence. Float worst -137.73 dBr.
Packed Q14 float arithmetic tone -86.94 dBr, impulse -57.17 dBr; integer
interior variant -80.40/-55.61 dBr. Both explicitly truncate at zero input
to clear demonstrated fixed-point zero-input limit cycles. No release mute.
Authentic signed holdout summaries use existing hash-verifying scorer and
immutable 26-capture archive, one A4 per shape plus 11 non-A4 and one repeat.
No gain, phase or time was fitted. Focused held results are unchanged to
0.000006 dB max signed relative to prior 4x one-anchor bench; absolute held
signed residual -3.42..-2.53 dBr, level -11.94..-9.76 dB. Not whole parity.
Source-only archive omits private data, generated arrays and PCM.

Compile test with repository stock R3 float flags:
  clang -DPACK=0 -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Oz -c target.c
Repeat PACK=1/2. Only component code/state measured, no flash fit or cycles.
For host holdouts use sibling sj6-spectrum-anchor stage scorer and build
this stage.c using sibling private one-anchor header; same native-signed
arguments, no regeneration or refitting. No production integration.
