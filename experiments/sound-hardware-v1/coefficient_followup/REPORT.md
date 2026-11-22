# S1 coefficient review: existing fastmath is exact on the recorded packet

The existing DnBOX `fm_expf` already reproduces every retention value needed
by both pinned S1 envelope packets. A float-only denominator and its existing
polynomial produce **zero differing binary32 words across all 379,288 recorded
outputs** when substituted into the independently authored proposal recurrence.
`fm_exp2f` and `fm_expf` with a double-prepared argument also match those packets.
No new coefficient polynomial or lookup table is needed to attain this bounded
fixture result. None of these paths is established as exact over all controls.

The concrete C-ready experimental path is `candidate.h`:

```c
static inline float s1_retention_fast(float seconds, float sample_rate, int attack)
{
    float denominator;
    if (seconds == 0.0f)
        return 0.0f;
    denominator = (sample_rate * seconds) * (attack ? 0.6f : 1.0f);
    return fm_expf(-1.0f / denominator);
}
```

Use the project's existing `fastmath.h`; the absolute include in this isolated
experiment pins the reviewed file. Preparation is outside the sample loop.
Caller preconditions are finite positive sample rate and finite nonnegative
duration, with the real public per-control bounds enforced by the adapter.
Zero duration is an explicit proposal extension, not a captured original-engine
observation. Physical values remain separate from prepared coefficients.

This routine uses only existing single-precision operations/helper code. It
does not introduce tables, double arithmetic or libm into the candidate path.
`coefficients.c` links libm solely for the comparison oracle in the host harness.
No target flash-size, execution-cost or memory-fit claim follows from the host
experiment, and no production file was changed.

## Exact packet evidence

`RESULT.json` records every case, output SHA256, source SHA256 and input SHA256.
`probe.csv` records all fourteen attack/nonattack combinations of the seven
fixture durations. Conventional coefficient words for observed stage settings
are:

| Duration / stage | Coefficient word |
| --- | --- |
| .05 attack | `3f7fce7c` |
| .2 attack | `3f7ff39e` |
| .0005 attack | `3f6d5cfe` |
| .005 decay | `3f7ed775` |
| .3 decay | `3f7ffb0c` |
| .05 release | `3f7fe249` |
| .5 release | `3f7ffd07` |
| .004 release | `3f7e8d88` |

The output comparison is exact words with no fitted gain, timing shift or
tolerance. The recorder verifies both pinned packet hashes before replay.
The host C routines produce the coefficients; the existing independently
authored Python proposal produces the envelope recurrence. This is therefore
coefficient-path evidence, not an independent C state-machine test.

A deliberate one-ULP increase to the attack retention is rejected by the
unchanged live case: **8,332 / 16,384 differing words**, first difference at
frame 0, maximum absolute error 0.0008636713027954102. This is retained as the
negative control in `RESULT.json`.

## Arbitrary-control limits: exactness fails outside the packet

The source-free public catalog gives attack .0005–2 seconds, decay .005–2,
amplitude release .004–2, and filter release 0–2, all with binary32 bounds.
Recorded original output packets establish only 44,100 Hz. Additional sweep
rates 8,000, 48,000 and 192,000 are candidate robustness checks; they do not
establish original-engine sample-rate support.

`SWEEP.csv` compares the existing fast helpers to the contract's inferred
double-expression/exp/one-binary32-round mathematical hypothesis. Each role
and rate has 100,001 linearly spaced plus 100,001 logarithmically spaced
inputs, converted to binary32 before comparison. Duplicated endpoints are
counted; the CSV point counts are tests, not distinct-duration counts.
The positive filter-release sweep starts at 1e-10 seconds; it is not exhaustive
over all nonnegative binary32 values. The sweep runs three candidate paths at
four rates and four roles: 9,600,096 scalar comparisons in total.

At 44,100 Hz the float-only `fm_expf` path differs from that mathematical
hypothesis on:

| Role | Differing coefficient tests / 200,002 | Largest ULP difference |
| --- | ---: | ---: |
| Attack | 1,137 | 1 |
| Decay | 94 | 1 |
| Amplitude release | 105 | 1 |
| Positive filter release | 18,832 | 8,588,343 |

These are mathematical-hypothesis comparisons, **not newly captured original
engine behavior**. For a concrete legal attack duration,
`seconds = 0.0005599849974f`, oracle coefficient is `3f6f4afb`, while both fast
paths produce `3f6f4afc`. Preparing the input argument in double does not remove
that mismatch; its attack sweep still has 839 mismatching coefficients.

The largest positive-filter-release ULP count reflects existing fastmath's
documented underflow behavior. At `seconds = 2.597062689e-7f`, mathematical
hypothesis coefficient is `00830c37`; `fm_expf` returns zero. At
`seconds = binary32(1/(44100*90))`, mathematical hypothesis coefficient is
`0008ec27`; both fast paths return zero. The separate `edge_probes` in
`RESULT.json` include zero and the region near natural exponential underflow.
This underflow difference must not be silently promoted to a full physical
filter-release qualification or hidden by an invented epsilon.

The existing helper is a concrete exact-packet candidate, with demonstrated
arbitrary-control differences. A prepared-coefficient component API allows the
state-machine work to remain independently testable while a future broader
coefficient contract is resolved. Do not activate or claim full S1 fidelity
from these packet results. A production requirement for exact arbitrary-control
agreement with the inferred mathematical formula would require a further
routine and proof/qualification; that is not supplied by this finite sweep.

## Reproduction, resources and source boundary

Run serially from this directory (compiler was Debian GCC 14.2.0):

```sh
cc -std=c99 -O2 -ffp-contract=off -fno-fast-math probe.c -lm -o probe
./probe > probe.csv
cc -std=c99 -O2 -shared -fPIC -ffp-contract=off -fno-fast-math coefficients.c -lm -o coefficients.so
python check.py
cc -std=c99 -O2 -ffp-contract=off -fno-fast-math sweep.c coefficients.c -lm -o sweep
./sweep > SWEEP.csv
```

All compiler and replay commands exited zero. Final replay wall time was
1.653320038 seconds; maximum Python RSS was 15,140 KiB. Sweep exited zero in
0.327819492 seconds; its wrapper reported 9,216 KiB maximum child RSS.
`sweep-resource.json` retains that actual status and notes an earlier resource
wrapper attempt exited 127 because `/usr/bin/time` was absent. The numeric
sweep was rerun successfully using Python subprocess/resource measurement.
Total retained directory payload before this report was 6,153,399 bytes.

Read inputs: the source-free IMPLEMENTER-CONTRACT, numeric JSON, authored Python
proposal, pinned output packets/JSON, public S1 catalog/behavior brief and the
existing DnBOX `fastmath.h`. Read applicable canonical AGENTS/RULES, CHECKLISTS,
Sound lane state and CONSOLIDATED-STATE; current task constraints override old
lane/Git/toolchain workflow instructions. Ancestors of the isolated experiment
and fastmath reference contained no AGENTS/RULES.

No original engine implementation was opened, copied, compiled or executed.
No native toolchain, full suite, emulator, benchmark, RDC, GitHub, Git mutation,
configuration mutation or production source mutation was used. The only
compilations were the bounded host scalar tools above, serial within this
isolated directory. The root is the sole production writer and decides whether
to integrate this candidate or keep coefficient preparation deferred.
