# SuperJ6 warm-high host proof and native cost

In addition to the 67830 exact post-filter cold-and-live words in the earlier record, a genuine warm transition from zero to k=4.12 at 8510.413 Hz and back reproduced 22610/22610 post-filter words exactly. Actual live route reached the other seed path at frame 26112; the existing history is retained and only the source’s small perturbation is added. This brings the exact host witness to 90440 words across four cues, not all presets or full audio.

Native fastmath reproduction is close but expensive: source-matched host error at least 111 dB below reference signal over the three full benchmark cues, not bit-identical. Native strict 360 MHz synthetic, same seeded 512 mono frames and 32 rotated trials:

| Implementation | Avg µs / 512 one-voice samples |
|---|---:|
| Stock filter leaf | 459.26 |
| Scoped k=0 linear history | 409.90 |
| Nonlinear fastmath, live moderate | 2434.46 |
| Nonlinear fastmath, cold high | 2943.50 |
| Two-iteration reciprocal, moderate / high | 2720.64 / 3228.87 |
| Three-iteration reciprocal, moderate / high | 2808.64 / 3316.82 |

Both reciprocal approximations are slower than dividing. The faster of them also worsens cold-high host residual to −77.76 dBr, from −111.20. Do not adopt. Coefficient-hoisting separately saved about 38 µs for fixed 512-sample inputs and spent about 37 µs when cutoff moved; not a viable fit remedy. 79/640 of the authenticated banks’ authored resonance values are zero, so relying on k=0 alone does not cover the catalog. Raw authored zero does not imply every modulation frame remains there.

Actual note-off sender and ownership across real sample media agree between Main and Sub in controlled Raw and Brainwave cases. Installed-app muted-arm closure rules out large stem-additivity/gain artifacts; this still leaves the role-specific source level/shape/release mismatch. No firmware code promoted, no device flash touched.

Source receipts:
```
74417152d1c09b04d9b5e6d731d909a74dd9cafe3b944a94cecbcf1f2a79ed00  root-warm-high.json
4f103a97e4bee5ce976a87580d39cda28787f29e48758e64a397f033d3d2175e  sj6-reciprocal-native.p1okwuvw/result.json
ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8

```
