# Signed SuperJ6 Normal routing A/B

Source-first trace: [app graph](superj6-original-app-signal-path.md), [filter envelope](superj6-source-envelope-coefficients.md). The scoped compact host experiment retained the same rendered events and waveform, compensated Normal filter and AMP-before-filter treatment, and changed only how the filter is driven. It replaced the old compact cutoff and tracking projection with the app's reached physical base cutoff, envelope strength, velocity and key tracking law, and used original resonance mapping. The coefficient target is latched per original outer callback tile, not the compact internal subspan. No reference PCM, coefficient tape, fitted gain or output normalization in that compact arm.

## Signed isolated post-voice error (dBr relative to reference; lower is better)

| Physical key | Attack before → after | Held before → after | Release before → after |
|---|---:|---:|---:|
| 36 | -14.89 → -32.11 | -25.20 → -36.73 | -29.31 → -36.29 |
| 48 | -32.25 → -34.00 | -35.13 → -35.88 | -35.31 → -35.79 |
| 60, held out | -4.98 → -35.67 | -15.42 → -34.21 | -19.74 → -33.57 |

First remaining reached cutoff divergence on physical key 60: onset 268.27768 versus 268.28220 Hz; next outer tile 268.31393 versus 268.86420 Hz; note-off 903.72113 versus 905.57007 Hz. The separate alternative FENV replacement improved the low test note but regressed the higher one; it stays off.

## Admission and exclusions

- Diagnostic only: fixed projected controls, Normal, unlinked and no unproved outer LFO routing. Actual LFO-on Brainwave (dry and authored), linked Solidify (dry and authored), and OTA Raw were bit-identical with routing on/off. This is a fail-closed scope check, not parity of those routes.
- Independently, the actual original setter for Vintage Filter ADSR sends physical sampler ID 32, not velocity-scaling ID 40. Matched fast receiver cue: switching ID32 alone changes the reached Normal cutoff law (initial 1178.6828613 Hz off versus 290.5914230 Hz on) even though both modes still advance the same scoped envelope’s authored curved segments. Do not apply a Vintage cutoff formula to the other branch.
- The isolated result does not establish installed whole-synth parity, polyphonic life-cycle parity, feature coverage, Elite/Pro fit or zero dropouts.
- Signed local result SHA-256 `de5a102118b1cad87203e3a05cc75d135b411f02782f87fac757dda09aeef6f3`. Off-domain guard receipt SHA-256 `2a7e156b224a20ba543dcc064ecc2d829c4ad633054c325f274c8fb7c0343b23`.
