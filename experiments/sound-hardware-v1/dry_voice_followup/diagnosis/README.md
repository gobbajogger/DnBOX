# S1 dry prototype: bounded feedback diagnosis

## Decision

**Variant B has a mathematically identified delay instability. Recommend one
new experiment: retain B's forward four poles and replace its two-sample
feedback delay with centered weights `1, 4, 6, 4, 1`, including an implicitly
solved current-output term.** This changes only the feedback discretization.
The zero-resonance branch preserves B's audio arithmetic exactly.

The proposal is **uncompiled and unrendered**. The previously discussed
instantaneous-output feedback option is discarded and also unrendered; it is
not another candidate. No new audio acceptance, production activation, memory
fit, native cost, or complete-engine qualification is claimed.

Files:

- `centered_feedback_proposal.c`: complete C-ready replacement substep and
  required state additions, not installed in the prototype.
- `scalar_checks.py`: reproducible bounded scalar equations and selected
  binary32 operations; no renderer or compiler.
- `scalar_results.json`: results and SHA-256 identities of the consulted
  source/evidence snapshots.

## What the frozen evidence establishes

B accepts **109/180** static dry note cases. All **45/45** zero-resonance cases
pass, with maximum RMS error **0.3138965745 dB** and maximum spectral error
**2.2837313830 dB**. At resonance 0.25, spectral errors all pass but **13/45 RMS
errors fail**. Thus instability removal alone cannot qualify the transfer.
The worst B level error is **45.1632395628 dB** at cutoff 22,050 Hz, resonance
0.9, velocity 20, note 36. All recorded samples are finite. These are the
existing results, not new measurements. [S1, S4]

The gate stays **RMS ≤ 1 dB, spectral ≤ 3 dB, fixed comparison gain 1, and
zero nonfinite samples**. The unchanged metric hash is
`bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1`.
No gain/postgain, normalization, alignment, physical-parameter edits, or
tolerance changes are proposed. [S4, S5]

## Exact failure mechanism

Let `n` count B's internal substeps, so its rate is 88,200 Hz. Put

\[
A=\exp(-2\pi f_c/88200),\quad b=1-A,\quad R=0.8r.
\]

The actual implementation uses the allowed float-only `fm_expf`. Each forward
stage is the retained Chicago recurrence

\[
y_j[n]=b\,y_{j-1}[n]+A\,y_j[n-1].
\]

Before a substep, `history[3]` is `y4[n−1]`, while `history[4]` is
`y4[n−2]`: B writes `history[4]=old history[3]` only after computing the new
output. Consequently `E=0` leaves **two substep delays**, not zero or one, in
the nonlinear feedback argument. [S1, S2]

For completeness, the permitted Chicago phase-compensation recurrence has
the feedback transfer

\[
D_E(z)=z^{-1}\frac{z^{-1}-E}{1-Ez^{-1}}.
\]

Setting `E=0` makes it exactly `z^-2`. The Chicago nonlinearity is

\[
\phi(d)=\operatorname{sgn}(d)
\left[1-\max(0,1-|d|)^5\right],\quad \phi'(0)=5,
\]

with `0 ≤ phi'(d) ≤ 5` where differentiated. Thus B's small-signal loop gain
is `k=5R=4r`, and its characteristic equation is

\[
(z-A)^4+k b^4 z^2=0.
\]

Its first unit-circle crossing satisfies

\[
\omega_* = 2\tan^{-1}\!\frac{1-A}{1+A},\qquad
k_\text{crit}=\frac{(1+A)^4}{(1+A^2)^2}.
\]

This falls from 4 toward 1 as A falls. It is not the cutoff-independent
continuous-time value 4 assumed by B's slope correction. [S1, S2; independent
derivation, reproduced in S6]

| Cutoff, Hz | B's float A | Critical physical r | B pole radius at r=0.9 |
| ---: | ---: | ---: | ---: |
| 64 | 0.9954511523 | 0.9999896068 | 0.9998814954 |
| 500 | 0.9650079012 | 0.9993660794 | 0.9990797767 |
| 2,000 | 0.8672084808 | 0.9899607811 | 0.9966415111 |
| 8,000 | 0.5655800700 | 0.8621279242 | **1.0064448244** |
| 22,050 | 0.2078796029 | 0.4889763386 | **1.2877739452** |

At 22,050 Hz even `r=0.5` has radius **1.0088779053**. These explicit
counterexamples agree with the high-cutoff failures. At `r=0.9`, A's original
unscaled `k=5r=4.5` also exceeds the low-cutoff limit near 4, explaining why
A's instability was not restricted to high cutoff. The saturating feedback
keeps trajectories finite; finiteness therefore does not rule out a sustained
oscillation. [S2, S4, S6]

## The one proposed correction

Use this feedback signal, evaluated at the internal substep rate:

\[
d[n]=\frac{y[n]+4y[n-1]+6y[n-2]+4y[n-3]+y[n-4]}{16}.
\]

Its linear transfer is

\[
D_C(z)=\left(\frac{1+z^{-1}}{2}\right)^4
      =e^{-2j\omega}\cos^4(\omega/2).
\]

Compared with B's `z^-2`, it preserves the entire feedback phase below
Nyquist while adding the precise trapezoidal magnitude factor. Its DC gain
is one. It does not scale the output or change the stored physical resonance.
This is a derived integration change, not an audio-fitted compensation curve.

The linearized loop becomes

\[
\left[\frac{b(1+z^{-1})/2}{1-Az^{-1}}\right]^4.
\]

Let `g=(1−A)/(1+A)>0`. The term in brackets is exactly the bilinear image of
`1/(1+s)`. The normalized continuous poles are

\[
p_\ell=-1+k^{1/4}e^{i(2\ell+1)\pi/4},\qquad \ell=0,1,2,3,
\]

and each nonzero discrete pole is `(1+g*p)/(1−g*p)`. For every legal
`r≤0.98`, `k≤3.92<4`, so every `p` has negative real part and every discrete
pole lies strictly inside the unit circle. The extra redundant history modes
of the seven-state implementation are at zero in the exact linear model.
This establishes frozen-coefficient small-signal stability across the full
cutoff range without a resonance clamp. [Independent derivation; S6]

At `r=0.98`, corrected pole radii for the five cutoffs are respectively
**0.9999770313, 0.9998206449, 0.9992872504, 0.9974055102,
0.9953761557**. At the worst-case cutoff and `r=0.9`, the radius changes from
**1.2877739452** to **0.9760748770**. [S6]

The derivation also exposes a limit: it preserves B's feedback phase and
therefore its existing frequency mapping. The `k=4` crossing at requested
22,050 Hz is approximately **16,295.748 Hz** internally. Stable operation
does not establish that the physical cutoff or resonance response matches
the original instrument. [S6]

## Bounded nonlinear solve

Compute `q` by passing the input through the same four forward-stage formulas
with feedback set to zero and without changing any state. By linearity of
those stages in real arithmetic,

\[
y=q-Rb^4\phi(d).
\]

With old output histories `h1=y[n−1]` through `h4=y[n−4]`, define

\[
w=\frac{q+4h_1+6h_2+4h_3+h_4}{16},\qquad c=Rb^4/16.
\]

The current feedback value solves the scalar equation

\[
d+c\phi(d)=w.
\]

The left side is continuous and strictly increasing; its derivative is at
least one. There is exactly one root, bracketed by `[w−c,w+c]` because
`|phi|≤1`. Start `d0=w` and perform exactly **six** updates

\[
d_{m+1}=w-c\phi(d_m).
\]

Every iterate after the first remains in that bracket. The contraction
constant is at most `rho=5c`. Over the requested legal range,

- Analytic minimum `A=exp(−pi/2)=0.20787957635076193`.
- Maximum `c=0.01929127465851203`.
- Maximum `rho=0.09645637329256015`.
- Uniform real-arithmetic error after six updates:
  `|d6−d*| ≤ c*rho^6 = 1.5536260561e−8`.

A deliberately looser bound using `A≥0.20` gives `c≤0.0200704`,
`rho≤0.100352`, and error at most **2.0498034606e−8**. The documented allowed
fastmath exponential-error contract leaves ample margin above A=0.20; no new
coefficient clamp is proposed. Contraction itself holds even for the broader
`0≤A≤1` range because then `5c≤0.245<1`. [S3, S6]

These are real-arithmetic bounds. Float rounding, including the original
Chicago polynomial's cancellation near zero, is additional. In **38 declared
scalar checks** at the exact-domain and conservative c bounds, the largest
six-step real error was **1.3790696e−9**; selected binary32 errors and residuals
were at most **3.3527613e−8**. This is a selected arithmetic check, not an
exhaustive binary32 proof, audio gate, or compiler comparison. [S6]

The proposed code re-evaluates the existing four forward stages once using
the final feedback argument, then commits their states once. The identity
`y=q−Rb^4*phi(d)` is exact in real arithmetic; the two floating evaluations
need not satisfy it bit for bit. This remaining roundoff is not hidden by the
solver bound.

Six finite iterations also retain the linear stability margin. With
`t=5c`, their small-signal result is the exact centered-feedback system with
effective loop gain `k_eff=k*(1+t^7)/(1−t^8)`. This follows by summing the
seven-term geometric series from the initial `d0=w` and six updates, then
expressing its final feedback argument in terms of the actual current output.
Using the conservative `t≤0.100352`, the gain factor is less than
**1.000000113**, hence `k_eff<3.920000443<4`. The finite iteration
count therefore does not rely merely on a small absolute-error assertion for
the exact-real linear stability claim. Binary32 roundoff remains separately
qualified as above.

## State, arithmetic, and qualification boundaries

The precise pre-step history mapping is:

| Stored member | Output history |
| --- | --- |
| existing `filter_history[3]` | `y[n−1]` |
| existing `filter_history[4]` | `y[n−2]` |
| new `feedback_lag3` | `y[n−3]` |
| new `feedback_lag4` | `y[n−4]` |

Before overwriting the existing histories, shift `lag4=lag3`,
`lag3=old filter_history[4]`; then keep B's existing `history[4]=old
history[3]` update and commit the new pole outputs. The current output is
never mistaken for an old history. Zero initialization applies to both new
members. Only **two extra floats (8 bytes per voice)** are needed because the
fourth pole already stores the most recent output. The expected voice size
is 160 bytes, requiring a fresh compiled layout check later.

The proposal copies the complete B zero-resonance audio arithmetic, including
the original nonlinear evaluation and multiplication by zero. Only extra lag
assignments are added there. Exact B output identity is a structural claim to
verify in the later compiled experiment, not a completed PCM comparison.

It retains B's two fixed held-input substeps and arithmetic-mean decimation.
At nonzero resonance each substep uses six nonlinear solver evaluations plus
one final evaluation, and an additional four-stage q computation. No cost
optimization is attempted. Native cycle cost, code size, and simultaneous
engine memory remain unqualified.

The bounded Chicago nonlinearity also means finite, bounded input drives a
bounded forward cascade: `|v|≤|input|+R`. This explains why an unstable
small-signal loop can still produce finite samples. It does not prove absence
of every nonlinear limit cycle in the corrected system. Frozen linear
stability does not establish arbitrary dynamic-cutoff stability, original
amplitude-dependent behavior, or sound equivalence.

The next owner-authorized experiment should freeze this single C change,
preserve the original gate and references, verify B's zero-resonance PCM
identity, and report the existing 180-note matrix honestly. Dynamic cases,
all intact presets, controls, FX, native resources, callback deadlines, and
listening approval remain outside this diagnosis. No compiler, audio render,
native build, emulator, full suite, timing, GitHub, RDC, or device job ran here.

## Sources and permitted-input boundary

- **S1:** Frozen B source and header:
  `/workspace/scratch/e803955aec60/s1_dry_voice_unit/variant_b/src/s1_dry_voice.c`
  and `s1_dry_voice.h`; frozen A source was read to confirm the earlier
  coefficient/resonance difference.
- **S2:** Allowed DnBOX recurrence:
  `/workspace/scratch/e803955aec60/DnBOX/experiments/original_synth/chicago_mp4.c`.
- **S3:** Allowed stock math:
  `/workspace/scratch/e803955aec60/DnBOX/firmware/src/dsp/fastmath.h`.
  Allowed `juno6.c` was also inspected; no other DSP source was needed.
- **S4:** Frozen A/B `all180_summary.json` files and selected B paired metric
  reports under `/workspace/scratch/e803955aec60/s1_dry_voice_unit`.
- **S5:** Source-free packet `case_plan.json`, `README.md`, and unchanged
  `metrics.py` under `/workspace/scratch/e803955aec60/dry_reference_packet`.
- **S6:** `scalar_checks.py` and `scalar_results.json` in this directory.

Repository instruction/state documents were read as required. No original
S1/SJ6 algorithm source, original tables/assets, prior source-aware candidate
algorithm, or reference-owner build input was opened. The named
`hardware_candidate/src/compact_profile` path was not present and was not
used. No external code or external DSP implementation was imported. All
writes are confined to this diagnosis directory; prototype and canonical
files are unchanged.
