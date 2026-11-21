# C nonlinear hypothesis: numeric evidence and decision

## Final decision

**Do not make `fm_tanhf(5*d)` the next candidate.** It was a plausible,
predictive explanation for the opposite signs of the remaining dry-note level
errors. The newly supplied original-filter observations instead show
proportional input/output behavior, including transients, across input sine
amplitudes 0.0001, 0.01, and 0.25 at the measured cutoff/frequency. A tanh
replacement would still introduce substantial amplitude-dependent compression.

The matched C filter-only observations are now terminal and confirm that C
introduces the amplitude-dependent loss. The **sole proposed next change is
nominal-range linear feedback, `phi(d)=5*d`, with the direct scalar solve
`d=w/(1+5*c)`**. It retains C's forward poles, centered feedback, controls,
absolute gain, full r=0 branch, timing, and tails. The C-ready change and its
bounded prediction are below. This remains an unimplemented, unrendered
hypothesis; no whole-instrument or global-linearity claim follows from the
eight observations. The prior diagnosis and frozen files remain unchanged.

## Scope and observed C status

The actual frozen C dry-voice comparison accepts **109/180 notes and 30/60
files**, with maximum RMS error **7.6914732584 dB** and maximum spectral error
**8.9590049906 dB**. These results supersede the older preparation-only wording
in C's frozen `READY.md`; that file remains a historical pre-render record.
The root separately reports that C reduced the previously sustained
high-cutoff tail from approximately 0.21644 RMS in B to approximately
2e-7 through 3.4e-7. That tail statistic was supplied by root and was not
recomputed here. [N1]

The root's signed RMS measurements are consistent with two different feedback
effects: at cutoff 64 Hz/resonance 0.9/note 36, C is **7.69147 dB too quiet**;
at cutoff 2,000 Hz/resonance 0.25/note 36/velocity 100, C is **1.12195 dB too
loud**. The saved summaries independently confirm the magnitudes; their
metric stores an absolute error and cannot independently supply the signs.

The same 64 Hz/resonance 0.9/note 36 absolute error grows with velocity:
**0.504687 dB** at 20, **6.314122 dB** at 100, **7.691473 dB** at 127.
That pattern supports investigating amplitude dependence. It does not by
itself identify a particular saturation law because whole-record RMS also
includes harmonics, transients, and release. [N1]

## Why tanh was a specific, testable hypothesis

For `|d|<1`, the existing allowed Chicago primitive is exactly, in real
arithmetic,

\[
\phi_C(d)=5d-10d|d|+10d^3-5d|d|^3+d^5.
\]

Its proposed stock alternative would have been

\[
\phi_T(d)=\tanh(5d)
         =5d-\frac{125}{3}d^3+O(d^5).
\]

Both have slope five at zero and both are bounded by one. Chicago starts
reducing its feedback strength with an order-`|d|` relative term; tanh starts
with an order-`d^2` relative term. Over most moderate magnitudes tanh therefore
returns a larger total feedback value. Examples from scalar evaluation:

| `|d|` | Chicago `|phi|` | `tanh(5|d|)` | Tanh / Chicago |
| ---: | ---: | ---: | ---: |
| 0.0001 | 0.000499900010 | 0.000499999958 | 1.00019994 |
| 0.01 | 0.0490099501 | 0.0499583750 | 1.01935168 |
| 0.05 | 0.2262190625 | 0.2449186624 | 1.08266147 |
| 0.10 | 0.4095100000 | 0.4621171573 | 1.12846367 |
| 0.25 | 0.7626953125 | 0.8482836400 | 1.11221824 |
| 0.50 | 0.9687500000 | 0.9866142982 | 1.01844057 |

This is not a global ordering. Their total magnitudes cross again at about
`|d|=0.736941907`; their local derivatives have a different ordering. For
example, at `|d|=0.25`, tanh's derivative is approximately 1.40207 versus
Chicago's 1.58203 even though tanh's total feedback value is larger.

For a sinusoidal feedback argument `d=B*sin(theta)`, a more useful scalar
comparison is fundamental feedback gain. For `0<B≤1`, Chicago gives

\[
N_C(B)=5-\frac{80}{3\pi}B+\frac{15}{2}B^2
       -\frac{32}{3\pi}B^3+\frac58 B^4,
\]

while tanh gives

\[
N_T(B)=5-\frac{125}{4}B^2+O(B^4).
\]

Direct static quadrature at four declared amplitudes gives:

| Feedback-argument amplitude B | Chicago fundamental gain | Tanh fundamental gain |
| ---: | ---: | ---: |
| 0.0001 | 4.999151249 | 4.999999688 |
| 0.01 | 4.915863975 | 4.996877602 |
| 0.05 | 4.593916311 | 4.923467379 |
| 0.25 | 3.296073851 | 3.704556019 |

These are static nonlinear-function calculations, not filter or note renders.
The amplitude B is the feedback argument, not the external sine amplitude;
resonance can make those very different.

Increasing effective feedback would lower below-cutoff gain, where the loop
acts as ordinary negative feedback. Near the still-stable resonant phase
crossing, it would increase the peak. That predicted the correct direction
for both signed dry-note examples. It also predicted measurable compression
and harmonic changes across the supplied amplitude probes. The latter
prediction is the useful discriminator; the original observations below
do not exhibit that compression. [N2; independent algebra]

## Exact original numeric observations

The isolated owner supplied eight fresh component captures: cutoff 2,000 Hz,
resonance 0 or binary32 0.9, one 0.0001 impulse, and a 2,000 Hz sine at
amplitudes 0.0001, 0.01, and 0.25. Each used a fresh instance, 4,096 zero
prewarm frames, and 32,768 retained frames at 44,100 Hz. The steady window is
frames `[16384,32768)`. All supplied samples are finite. This is one component
probe protocol, not the original full-voice acceptance matrix. [N3, N4]

| Nominal sine amplitude | Original steady gain, r=0 | Original steady gain, r=0.9 |
| ---: | ---: | ---: |
| 0.0001 | -12.5827420845 dB | +8.2197486932 dB |
| 0.01 | -12.5827420248 dB | +8.2197498169 dB |
| 0.25 | -12.5827421655 dB | +8.2197483366 dB |

The complete spread is only **1.40722699e-7 dB** at r=0 and
**1.48035574e-6 dB** at r=0.9. This is not merely a statement about equal
RMS. I read only the supplied numeric PCM and compared each larger-amplitude
output with the smallest using the **recorded binary32 input-amplitude
ratio**, with no fitted gain, sample shift, alignment, or edited recording.

| Resonance | Larger input amplitude | Whole-record relative L2 difference | Steady-window relative L2 difference |
| ---: | ---: | ---: | ---: |
| 0 | 0.01 | 2.29422e-7 | 2.30051e-7 |
| 0 | 0.25 | 2.17526e-7 | 2.17760e-7 |
| 0.9 | 0.01 | 3.77894e-7 | 3.71852e-7 |
| 0.9 | 0.25 | 2.61903e-7 | 2.51889e-7 |

The statistic is
`RMS(y_large*(a_small/a_large)−y_small)/RMS(y_small)` using known stimulus
values, not a fitted comparison gain. The corresponding input-waveform
relative differences are approximately 3.97e-8 and 3.00e-8 because each
stimulus sample was independently rounded to binary32. The output differences
are consistent with accumulated floating rounding and demonstrate amplitude
proportionality over these actual streams, including the starting transient.
No original or candidate PCM was changed. All twelve distinct supplied input
and output PCM files were hash-verified against the observation metadata.

This supports a nominal-range linear behavior hypothesis for these stimuli.
It does not establish linearity at every frequency/cutoff, under control
modulation, at other levels, or for a whole instrument.

## Matched C observations discriminate the hypothesis

The independent implementation worker supplied eight terminal C component
records with the same input-file hashes, fresh state, prewarm, frame count,
cutoff, resonances, and steady window. They run the frozen C filter substep
twice with held input and average the results; oscillator, VCA, and envelope
advancement are absent. All eight candidate output hashes were verified, and
all candidate samples are finite. [N5]

At resonance 0.9:

| Input sine amplitude | Original steady gain | C steady gain | C minus original | Reconstructed C feedback peak |
| ---: | ---: | ---: | ---: | ---: |
| 0.0001 | +8.219748693 dB | +7.386123159 dB | -0.833625534 dB | 0.000232265 |
| 0.01 | +8.219749817 dB | +5.411479316 dB | -2.808270501 dB | 0.018504851 |
| 0.25 | +8.219748337 dB | -2.711459097 dB | -10.931207433 dB | 0.181664701 |

Relative to its lowest-level measurement, C loses **1.974643843 dB** at 0.01
and **10.097582256 dB** at 0.25. The original's gain spread is only
1.48036e-6 dB. At resonance zero, C's steady gain instead remains
approximately **-12.055876 dB** at every amplitude, about **0.526866 dB**
above the original. This separates an amplitude-dependent feedback defect
from a remaining amplitude-independent transfer discrepancy. [N5]

C's measured combined harmonics 2 through 5, relative to its fundamental,
grow from **-108.977252 dB** through **-73.235360 dB** to **-55.515910 dB**
over the three resonant levels. The original corresponding measurements are
approximately -165, -163, and -169 dB. These are the worker's fixed-frequency
Hann-window projections, not a claim of full-band THD. The reconstructed
feedback peaks above are read-only combinations of current output and old
lags, not an exact trace of the scalar solver's internal d. [N5]

Tanh would weaken the compression and could improve individual dry notes,
but it would preserve a nonlinear amplitude dependence absent from these
reference records. It cannot be the selected explanation for the observed
proportional response across all three amplitudes. It is therefore rejected
as the next candidate; no tanh filter is implemented or rendered.

### Tiny-signal qualification

For the resonant 0.0001 impulse, C leaves about **4.22597e-8 RMS** in the last
4,096 frames, while the original's corresponding value is approximately
4.20e-45. Its finite-record 2,000 Hz impulse-response gain is **+3.936136 dB**,
compared with the original's **+8.219670 dB**. C's small sine response at the
same input amplitude is +7.386123 dB. The impulse's reconstructed feedback
peak is only about 5.07e-6; its residual feedback peak is about 5.94e-8. [N5]

Chicago's subtraction `1−(1−|d|)^5` has severe relative quantization at these
tiny arguments. This provides a source-based reason for the measured impulse
and sine responses not to behave as a single exact linear model at that
scale. The finite-record impulse diagnostic also includes the residual tail,
so it must not be treated as an uncontaminated LTI transfer estimate for C.
No tail is removed or suppressed in this analysis. [N2, N5]

Thus "unchanged small-signal transfer" below means the intended exact-real
linearization of C. It does **not** promise bitwise preservation of C's
measured near-zero cancellation and residual-tail behavior.

## The sole next proposal: nominal-range linear centered feedback

The slope five is already the exact small-signal derivative of C's permitted
Chicago primitive. It is not inferred or fitted from the original outputs.
Retain `R=0.8f*physical_resonance`. In C's existing scalar equation,

\[
d+c\phi(d)=w,
\]

substitution of `phi(d)=5d` gives the direct solution

\[
d=\frac{w}{1+5c}.
\]

### C-ready bounded change

Only these two nonzero-resonance changes are proposed:

```c
/* Replace C's private six-iteration helper body. No libm or allocation. */
static float feedback_solution(float w, float c)
{
    return w / (1.0f + 5.0f * c);
}

/* In the existing NONZERO-resonance commit path only: */
float d = feedback_solution(w, c);
float tmp = history[3];
float nonlinear = 5.0f * d;
float v = input - R * nonlinear;

/* Then retain all four forward-stage formulas and all lag writes verbatim. */
```

The entire `R==0.0f` branch remains byte-for-byte unchanged in the candidate
source, including its existing `chicago_nonlinear(history[4])` call and
zero multiplication. Keep the Chicago helper solely where needed by that
preserved branch; do not replace its body globally. Do not modify q, w, c,
coefficient preparation, two-substep hold/mean, histories, reset, physical
range checks, oscillator, velocity, envelope, note events, or any gain.

There is no new state or coefficient. Expected voice storage stays at the
already measured C value **160 bytes**. The new scalar solve uses a fixed
number of ordinary float operations and one division per internal substep,
instead of six nonlinear iterations plus the final nonlinear evaluation.
It has no mutable globals, heap, double, FMA, fast math, or external code.
Retain the established `-ffp-contract=off` and `-fno-fast-math` contract.

For every legal existing coefficient, `c≥0`. The denominator is at least one;
the previous conservative physical-domain bound makes it less than **1.101**.
There is no ill-conditioned division or iterative termination condition.
Native cycle/code-size cost remains unmeasured; fewer operations are not a
timing result.

### Transfer and stability prediction

Let `b=1−A`, `k=5R`, and let z refer to the 88,200 Hz internal substep rate.
The proposed exact-real linear transfer is

\[
H(z)=\frac{b^4}
{(1-Az^{-1})^4+(k b^4/16)(1+z^{-1})^4}.
\]

This is C's intended small-signal centered-feedback transfer. C's six finite
iterations introduced a minute gain factor below 1.000000113 in the exact
linear model; the direct solve removes that perturbation. It also removes
the Chicago compression and near-zero subtraction. It does not change the
cutoff/resonance coefficient law.

The characteristic equation is

\[
(z-A)^4+(k b^4/16)(z+1)^4=0.
\]

With `g=(1−A)/(1+A)>0`, its four nonzero poles are the bilinear images of

\[
p_\ell=-1+k^{1/4}e^{i(2\ell+1)\pi/4}.
\]

At the largest binary32 legal resonance, the existing float products give
`k=3.920000195503235<4`; all real-model pole real parts remain negative and
all corresponding discrete pole magnitudes remain below one. The extra
redundant history modes remain at zero. This proves frozen-coefficient
linear stability over the full existing cutoff/resonance ranges. It is not
an exhaustive binary32 stability proof or a proof for arbitrary parameter
modulation.

Removing saturation means the earlier bound `|input_to_poles|≤|input|+R`
does not apply. The relevant justification becomes the stable linear
closed-loop transfer for fixed controls and the supplied nominal levels.
Do not silently replace that distinction with a new limiter or clamp.

### Quantitative prediction without fitting

The complete output-rate transfer must include the existing held input and
average decimation. For output frequency f, set `omega=2*pi*f/88200` and
evaluate the internal H above. Its exact-real output-rate transfer is

\[
H_o(f)=\cos^2(\omega/2)H(e^{i\omega})
      +\sin^2(\omega/2)H(e^{i(\omega+\pi)}).
\]

Using C's existing float A=0.8672084808349609 at cutoff 2,000 Hz, with no
coefficient or output adjustment, the predicted 2,000 Hz fundamental gains
are:

| Resonance | Analytic linear proposal | Original measured fundamental |
| ---: | ---: | ---: |
| 0 | -12.055958344 dB | approximately -12.5828023 dB |
| binary32 0.9 | +7.415658970 dB | approximately +8.2196708 dB |

These gains are amplitude-independent in the model. The first prediction
agrees with the actual C zero-resonance fundamental measurement to about
3e-8 dB, providing a check on the two-rate derivation. The resonant prediction
leaves approximately **-0.804012 dB** versus the original, while removing the
several-dB level dependence. It predicts only about +0.029596 dB change from
C's smallest-sine fundamental but about +10.127207 dB from its largest-sine
fundamental. These are analytical predictions from the permitted recurrence,
not a rendered linear candidate or acceptance result.

The reported finite-window RMS gains differ slightly from the fixed-frequency
fundamental gains; comparisons above consistently use the latter. Remaining
linear transfer/phase and high-cutoff discretization differences are not
addressed by this proposal. No second correction, slope fit, postgain,
cutoff remapping, clipping, or tail rule accompanies it. The unchanged
180-note gate remains RMS≤1 dB, spectral≤3 dB, comparison gain 1, nonfinite 0.

## Solver implications and cost of the rejected tanh option

For exact tanh, `0≤d[tanh(5d)]/dd≤5` and `|tanh(5d)|≤1`. Therefore the
existing centered equation `d+c*phi(d)=w` remains uniquely solvable in
`[w−c,w+c]`. Its six-step real contraction bound is unchanged:
`5c≤0.096456374`, and the prior uniform real error bound remains
approximately 1.55363e-8. This is a statement about the ideal function, not
an exhaustive binary32 proof of the piecewise `fm_tanhf` implementation.

The allowed stock function uses three relevant branches for `x=5d`:

- `|d|<8e-5`: returns `5d` directly.
- `8e-5≤|d|<0.1`: evaluates its fixed odd polynomial.
- `0.1≤|d|≤1.8`: uses `fm_exp2f` and division; above that it saturates.

C would call it six times in the solve and once in the final feedback per
substep, for fourteen evaluations per output sample. Its polynomial work is
larger than Chicago's three-power-multiplication primitive, and the exp2/divide
branch could occur at relevant levels. State need not grow, but target cost
would require measurement. No native efficiency claim is warranted.

The tiny-argument linear branch would also avoid Chicago's cancellation in
`1−(1−|d|)^5`. That could change residual tails without a tail cutoff. It does
not justify a saturating replacement when the measured nominal behavior is
proportional. The original tanh proposal is rejected rather than retained as
a second candidate. [N2]

## Sources and boundary

- **N1:** `/workspace/scratch/e803955aec60/s1_dry_voice_c/results/audio180/SUMMARY.json`,
  SHA-256 `d20dcf4f1279db0b8c727c814be8bb1d4b0a980bc68c54cb27a028cd4df92182`;
  root-supplied signed/tail statistics are explicitly identified above.
- **N2:** Allowed stock `/workspace/scratch/e803955aec60/DnBOX/firmware/src/dsp/fastmath.h`
  and frozen B/C `src/s1_dry_voice.c`, especially C's unchanged Chicago
  primitive, six-step feedback solver, and centered recurrence.
- **N3:** `/workspace/scratch/e803955aec60/reference_filter_inputs/host_observation_20261119/observations.json`,
  SHA-256 `607905ab1441abc2078a076855bc6fd24879f7930dcadb246ffef1e737e0113e`.
- **N4:** The same packet's allowed `metadata/input_pcm_manifest.json`,
  `inputs/*.f32le`, and `outputs/*.f32le`, with identities verified against N3.
- **N5:** `/workspace/scratch/e803955aec60/s1_c_filter_probe/reports/OBSERVATIONS.json`,
  SHA-256 `85c088668cb1106cc5c32e6438c0e6535c82ea328d21435c91bb45863c3b372a`,
  and its eight hash-verified numeric C output files. No capture source or
  compiler/build input was opened for this analysis.

No original engine algorithm, capture wrapper, build input, object, header,
table, or asset was inspected. The original numeric packet is the only
original-filter input to this analysis. No slope or gain was fitted, no
cutoff clamp or tail cutoff was introduced, and no acceptance threshold was
changed. Only source inspection, saved-numeric analysis, scalar algebra, and
the C-ready proposal in this report were performed. No new candidate was
implemented, compiled, or rendered; no native, emulator, timing, GitHub,
RDC, or device job ran. All writes are confined to this additive report in
the diagnosis directory.
