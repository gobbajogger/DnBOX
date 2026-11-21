# Matched C filter-only diagnostic: resonant loss depends strongly on level

Exactly eight matched component records completed successfully using the four
supplied input files and unchanged C. The result isolates a level-dependent
loss in C's resonant filter: at cutoff 2000 Hz and resonance 0.9, the reference
steady sine gain stays near **+8.219749 dB** for amplitudes 0.0001, 0.01 and 0.25,
while C gives **+7.386123, +5.411479 and −2.711459 dB**. The corresponding C-minus-
reference differences are **−0.833626, −2.808271 and −10.931207 dB**.

There is also a smaller level-independent forward-transfer difference: at
resonance zero, C's steady 2000 Hz sine gain is approximately **0.526866 dB above
the reference** at all three amplitudes. These component observations have no
new acceptance threshold and do not replace the existing static dry gate.

## Exact shared protocol

The only reference inputs read were the permitted numeric observations,
input-PCM manifest, four input streams and eight recorded output streams under
`reference_filter_inputs/host_observation_20261119`. No original capture source,
build file, header, object or engine algorithm was read or executed.

Each record uses a fresh 160-byte C owner, sample rate 44100 Hz, cutoff exactly
2000 Hz, resonance binary32 `00000000` or `3f666666`, 4096 zero-input prewarm
frames, and 32768 retained samples. The steady window is exactly
`[16384,32768)`, matching the numeric reference contract. Inputs are one
amplitude-0.0001 impulse and 2000 Hz sines at binary32 amplitudes 0.0001, 0.01
and 0.25. Their original bytes and hashes are preserved; none was regenerated.

The new host wrapper includes frozen C's own source to call its private
`filter_substep` twice with the same input, then averages the two outputs using
C's original arithmetic. It does not advance the oscillator, VCA, amplitude or
filter envelope. The prepared control state and envelope/oscillator state are
checked unchanged at the end of each record. There is no extra gain, output
normalization, phase alignment, resampling, fitted coefficient or tail edit.

One host wrapper build ran. Then exactly eight response records ran serially;
each process exited 0. All candidate output and prewarm samples were finite,
and prewarm output was zero. All eight output files and command/status logs are
retained. C source hashes and executable identity remained unchanged.

`BUILD-DECLARATION.json` freezes the wrapper, C source, diagnostic analysis,
numeric inputs, reference outputs and commands before the build. `DECLARATION.json`
adds the compiled executable hash before the first response record. The
unchanged C core source hash is
`29e04eb07c512bb390bf6f7cc7b9429baf46cf927db126bced2f1fffe1ae1bf9`.

## Whole-record and steady-window RMS gain

These are descriptive `20*log10(output_RMS/input_RMS)` measurements on the
unmodified signals. Whole record means all 32768 retained frames. Steady means
frames 16384–32767. The impulse has zero input in the steady window, so its
steady input-to-output gain is undefined and recorded as null.

| Input | Resonance | Reference whole | C whole | Reference steady | C steady |
| --- | ---: | ---: | ---: | ---: | ---: |
| Impulse 0.0001 | 0 | −13.646636 dB | −13.517035 dB | — | — |
| Impulse 0.0001 | 0.9 | −10.390572 dB | −9.878275 dB | — | — |
| Sine 0.0001 | 0 | −12.580485 dB | −12.053955 dB | −12.582742 dB | −12.055876 dB |
| Sine 0.0001 | 0.9 | +8.209534 dB | +7.379698 dB | +8.219749 dB | +7.386123 dB |
| Sine 0.01 | 0 | −12.580485 dB | −12.053955 dB | −12.582742 dB | −12.055876 dB |
| Sine 0.01 | 0.9 | +8.209535 dB | +5.406348 dB | +8.219750 dB | +5.411479 dB |
| Sine 0.25 | 0 | −12.580485 dB | −12.053955 dB | −12.582742 dB | −12.055876 dB |
| Sine 0.25 | 0.9 | +8.209534 dB | −2.713234 dB | +8.219748 dB | −2.711459 dB |

At resonance 0.9 and amplitude 0.25, the steady reference RMS is
**0.45540962498925464**, while C gives **0.12937245915422702**. The direct,
unshifted difference RMS is **0.32603829526631145**. At amplitudes 0.0001 and
0.01, direct steady difference RMS is respectively **0.00001849766684041422**
and **0.005058750395244765**. No fitted gain or time shift was applied to these
differences.

These matched inputs establish that the large level-dependent discrepancy
exists inside C's filter path in this frequency/amplitude region, independently
of the oscillator, velocity, VCA and envelope assembly. They do not establish
global linearity of the reference or its behavior at other frequencies,
amplitudes, cutoffs, resonance values or moving controls.

## Fixed-frequency transfer and harmonic diagnostics

Sine diagnostics use fixed 2000 Hz and harmonic 2–5 projections in the same
steady window with a symmetric Hann window. Ratios cancel the shared window
factor; no signal is rescaled or fitted. Phases are measured response phases,
not alignment adjustments. The combined harmonic ratio below is only the
specified second-through-fifth harmonics, not a full THD measurement.

| Sine amplitude, r=0.9 | C harmonics 2–5 relative to fundamental | C third harmonic | C-minus-reference fundamental phase |
| ---: | ---: | ---: | ---: |
| 0.0001 | −108.977252 dB | −109.022590 dB | −2.645497° |
| 0.01 | −73.235360 dB | −73.237486 dB | −1.908594° |
| 0.25 | −55.515910 dB | −55.519205 dB | −0.178347° |

The reference's corresponding measured harmonic ratios remain near the
floating-point/window residual level; exact values are retained in the numeric
observations. At resonance zero, the C-minus-reference phase difference is
approximately **+3.412233°** at 2000 Hz for all three levels.

The finite impulse-record response uses unwindowed projections at eight fixed
frequencies. At resonance zero, the C-minus-reference magnitude differences are
approximately **+0.062817 dB at 500 Hz**, **+0.212659 dB at 1000 Hz**,
**+0.526843 dB at 2000 Hz**, **+0.835437 dB at 4000 Hz**, and
**+0.978693 dB at 8000 Hz**. The DC gains are both essentially unity.

At resonance 0.9 the impulse is not a clean universal linear-transfer probe
for C: its retained low-level tail contributes to finite-record projections.
For example, the whole-record 2000 Hz ratio is +3.936136 dB for C and
+8.219670 dB for the reference, differing from the sustained sine result.
Those numbers are preserved as finite-record response diagnostics, without
treating nonlinear/rounding behavior as an exact linear transfer function.

## Reconstructed feedback argument and tiny residual tail

The wrapper observes the current returned substep output and copies the four
old output histories before each call. Outside the DSP it reconstructs

`(current_output + 4*old1 + 6*old2 + 4*old3 + old4)/16`.

This is **not the exact traced six-iteration solver value `d`**. It is a
read-only reconstruction from the committed output and old lags, computed in
host double precision, never fed back to the candidate. C itself is unchanged.

| Sine amplitude, r=0.9 | Reconstructed steady feedback peak |
| ---: | ---: |
| 0.0001 | 0.00023226482062455034 |
| 0.01 | 0.018504851264879107 |
| 0.25 | 0.18166470061987638 |

After the resonance-0.9 impulse, C's last-4096-frame RMS is
**4.225969451599134e-8**, with steady peak **5.96970721744583e-8** and
reconstructed steady feedback peak **5.9367075699867655e-8**. The reference
last-window RMS is **4.203895392974451e-45**, a subnormal residual. At resonance
zero C's last-window RMS is **1.6815581571897805e-44**.

The huge mathematical ratio between C's tiny resonant residual and the
reference's subnormal value is not an input-to-output steady gain: the impulse
input is exactly zero in that window. It must not be read as a new acceptance
metric or a large retained signal comparable to B's earlier 0.216 RMS
instability.

## Evidence and remaining boundary

- `reports/OBSERVATIONS.json`: all eight numeric rows, protocol, exact input,
  reference and candidate hashes/paths, both RMS windows, unshifted errors,
  frequency/phase/harmonic diagnostics, and reconstructed-feedback statistics.
- `CAPTURE-RESULT.json`: eight actual wrapper results and output/log hashes.
- `logs/`: exact one-build and eight-response commands, statuses and outputs.
- `outputs/*.C-filter.f32le`: all eight unchanged candidate streams.
- `BUILD-DECLARATION.json`, `DECLARATION.json`: pre-execution freezes.
- `src/filter_probe.c`: wrapper only; frozen C DSP is included unchanged.

Only numeric observations, protocol and output paths were sent to the diagnosis
agent after terminal completion. This report makes no three-note/static-gate
PASS claim. C's existing whole-voice result remains 109/180 passing notes and
overall FAIL. Native cost, phase/cutoff behavior outside this observation,
dynamic controls, full presets, FX and production admission remain unqualified.
