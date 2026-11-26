# Independent current pre-Normal signed holdouts: both families rejected
Independent source owner replayed the current ARM64 original locally against generated candidate output. Same held note cue, velocity87 at 44100, blocks128, onset16384, actual .3b78e0 pre-Normal snapshot. No fitted gain, phase or time. Original/plugin/raw audio excluded. Residual RMS relative to actual signed fixture in dBr; columns sparse 32/7/Q8, 64/12/Q8, 64/16/Q16 for each shape.

| Note | Sine 32 | Sine 64/12 | Sine 64/16 | Triangle 32 | Triangle 64/12 | Triangle 64/16 |
|---|---:|---:|---:|---:|---:|---:|
| A2 | −42.69 | −42.96 | −44.23 | −31.86 | −35.31 | −36.43 |
| A3 | −42.71 | −42.97 | −44.22 | −31.86 | −35.32 | −36.44 |
| A5 | −42.71 | −42.84 | −43.76 | −32.58 | −36.86 | −38.49 |
| A6 | −44.07 | −44.88 | −45.93 | −35.75 | −37.05 | −37.74 |
| A7 | −37.27 | −37.34 | −37.93 | −31.07 | −31.10 | −31.30 |

Held pitch bend key65:
- 32: Sine −42.80/−42.80/−42.80; Triangle −32.00/−31.97/−32.00 (pre/during/restored).
- 64/H12: Sine −43.05/−42.98/−42.98; Triangle −35.65/−35.59/−35.65.

Sparse32 level error ≤.0227 dB; sparse64 ≤.0333 dB, never gain-normalized for signed scores. A7 H1-magnitude-equalized H1–18 validation only: Sine32/64 −43.68/−42.57, Triangle32/64 −33.27/−33.05. Neither measured family qualifies for same sound; authored amplitude is not cause. Non-sparse Triangle about −21.5 dBr and analytic original typed stock −15.35 to −11.77 dBr. Release waveform and later whole AU still not qualified, no product or resource link triggered.

## One exact source-clock-only ablation
Source revealer verified real loaded mode at .3b63fc→.3b67f0→.3b6bb8/.3b6bdc advances the real voice phase with four successive F32 quarter-steps and per-step wrap, not one full-rate step. Same phase0 reset, no fitted time/phase. In sparse32 this reduces A7 h1 residue Sine +.58008→+.33217° and Triangle +.57798→+.33007°; measured change -.24791°. Saw/PWM remain byte identical. Not a heldout waveform qualification for this clock subvariant.

The analytic transfer of the known two authentic halfband stages (halfband.py) itself differs by -.33465° at A7 relative to the linear DC delay used by both surrogates, accounting for a separate comparable component. A third-order Taylor phase derived strictly from section coefficients predicts that transfer within .004015° at A7; does not use measured waveform, does not apply a fitted phase shift. Isolated compile of phase-prep.c gives 30 B PI32v2 text and three hardware multiplies, one add, zero persistent state; replacing existing linear delay would add two multiplies and one add per prep. Not integrated/tested as a new oscillator: dynamic live pitch and allpass memory, plus higher-harmonic phase/crossover shape errors, remain. No native timing claim or marginal link, no new approval for full integrated replacing.
