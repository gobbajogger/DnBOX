# Reusable S1 envelope component: source-free implementation contract

## Ready next action

Implement the scalar state machine below as a separate S1 profile in DnBOX.
Keep the existing, independently qualified SuperJ6 AHDSHR unchanged. Use the
two numeric packets below as the test oracle. The isolated Python proposal
matches all **379,288 binary32 outputs exactly**; four deliberate behavioral
mutations fail. This is real bounded component evidence, not full S1 admission.

No original engine source was opened or copied to derive the proposal.
The inputs were the existing behavior briefs, recorded scalar outputs and
the current independently written DnBOX core. The authored Python proposal
and its pre-execution declaration remain beside this file.

## State and ownership

An adequate proposed per-envelope state is:

```c
struct s1_envelope_state {
    float level;
    float retention;       /* coefficient latched for current stage */
    uint8_t stage;         /* initial 0, attack 1, decay 2, release 3 */
    uint8_t previous_gate;
    uint8_t reserved[2];
};
```

This is 12 bytes on the current 32-bit ABI, subject to a static assertion.
Use two independent states per note (amplitude and filter), six notes per
part initially. Proposed envelope history is therefore 144 bytes per part.
That arithmetic does not include physical controls, filter/oscillator
history, modulation, FX, allocation or callback workspace and is not a native
RAM-fit result. Existing shared state must not be overwritten to attain it.

The parameter adapter can prepare three retention coefficients plus the live
sustain target once per physical-control update. Each envelope copies only
the current stage's prepared coefficient when that stage begins. Keep the
original physical values available to the real control/UI layer. Time edits
must not overwrite a running envelope's latched coefficient.

## Per-sample event and arithmetic contract

All initial state is zero. Parameter edits and gate events take effect before
the output at their numbered frame. In this order:

1. Apply a parameter edit to the shared current physical values/prepared
   coefficients. Do not modify a running stage's latched coefficient.
2. If gate differs from `previous_gate`, a rising gate enters attack and
   latches the current attack coefficient; a falling gate enters release and
   latches the current release coefficient. A rising gate retains `level`.
   Update `previous_gate`.
3. If the stage is attack and the previous output `level >= (float)0.99`,
   enter decay and latch the current decay coefficient. This test occurs
   before the sample recurrence. No additional hold stage occurs.
4. Set target to 1 during attack, the current sustain value during decay, or
   0 during release/initial state. Sustain has no snap or separate settled
   stage: its target is read for every decay sample.
5. Compute in binary32 with no fused multiply-add or reassociation:

   ```c
   float old_term = retention * level;
   float target_weight = 1.0f - retention;
   float target_term = target_weight * target;
   level = old_term + target_term;
   ```

The output is the new level. The inferred coefficient formula used by the
passing numerical proposal is:

```text
retention = binary32(exp(-1 / (sample_rate * binary32(seconds) * scale)))
scale = 0.6 for attack; 1 for decay and release
```

The denominator and exponential in that proposal are evaluated in double
precision, left-to-right as `(sample_rate * seconds) * scale`, then rounded
once to binary32. C tests should use `-ffp-contract=off -fno-fast-math`.
Coefficient preparation belongs outside the sample loop. Introducing target
`exp`, `expf`, DnBOX fast-math, tables or polynomial preparation is a separate
equivalence and flash-cost check: **do not claim that the Python arithmetic
qualifies a particular target coefficient routine**. A prepared-coefficient
component API allows the state machine to be tested independently of that
choice.

The prototype accepts zero time as coefficient zero, but **the recorded
fixtures do not qualify that extension**, arbitrary rates, out-of-range
values, or all gate/parameter coincidences. Physical S1 public domains remain
the existing catalog's domains. Release approaches tiny nonzero/subnormal
values in the recordings; exact-zero detection is not a qualified voice
retirement policy. Do not flush or force the tail to zero as part of this
component change.

## Exact fixtures

Static packet directory:

`/workspace/scratch/ef5bc87a67de/recovered/package/staged/experiments/sound-hardware-v1/mapping/synthone_followup/implementer_brief`

- `s1_envelope_observations.f32le`: 1,058,400 bytes, SHA-256
  `9b164e9b7ebf3fe95665ceaa64afb39d51442e4167305c5d7bc55e115995ade4`.
- `envelope_observations.json`: 3 contiguous cases of 88,200 samples each,
  gate on at 0, off at 44,100, rate 44,100 Hz. Physical settings are
  `(0.05, 0.005, 0.8, 0.05)`, `(0.2, 0.3, 0.4, 0.5)` and
  `(0.0005, 0.005, 0, 0.004)`; use the JSON's exact binary32 values.

Live packet directory:

`/workspace/scratch/de30bbe568fe/fidelity-work/implementer_brief`

- `reference_observations.f32le`: 458,752 bytes, SHA-256
  `2cafbc8eb4b7aba8e22e0820ce8116d9dc7a57e6f8c41ed162c44ed98c474cfe`.
- `observations.json` and `case_plan.json`: 7 contiguous cases of 16,384
  samples, rate 44,100 Hz, initial `(0.05, 0.005, 0.8, 0.05)`, gate on at 0
  and off at 12,288. Cases: unchanged; A=0.2 at 1,000; D=0.2 at 7,000;
  S=0.2 at 10,000; R=0.3 at 13,000; gate low at 8,192 only; gate low at
  1,000 only. Those last two are component gate events, not whole-note
  allocator semantics.

No original engine executable is needed to replay either oracle. Preserve
the exact files/hashes and append new results; never regenerate or replace
the reference from a candidate.

## Red and green evidence

`numeric-result-01.json` records the proposal's exact match across all 10
cases: zero different words, zero nonfinite outputs, max absolute/RMS error
zero. Four wrong alternatives reject: nominal attack time constant, sustain
snap, resetting level on retrigger, refreshing a running time coefficient.
The run took 0.458 seconds with 13,204 KiB maximum RSS after the root released
both native links. No compiler or original executable ran.

The existing core remains a valid red baseline, not a new failure introduced
here. Its source hash is
`9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa`.
The retained candidate outputs are at
`/workspace/scratch/de30bbe568fe/fidelity-work/evidence/envelope-live-02/candidate.f32le`.
These five 16,384-sample cases correspond to live oracle cases 0–4. Its
`RESULT.json` already records baseline maximum error 0.422284856 and sustain
edit maximum error 0.597291633. Reuse these genuine red measurements; do not
claim this proposal reran the old C binary.

For the root's new C component, add a focused executable that emits the same
case-major binary files. Compare all output words with the two pinned files,
without a shifted alignment or loose epsilon. Compile and run that component
only after the root's exact source edits, then retain compiler flags, source
hashes, outputs and actual terminal status. Add state isolation and block
partition tests at that component's real API; those are new candidate
invariants, not substituted original-app observations.

## Remaining integration

This component closes only one concrete prerequisite. The current production
S1 wrapper is still intentionally silent. Before activation it needs a common
per-note oscillator sum -> squared velocity/component gain -> amplitude
envelope -> nonlinear four-pole filter path, exact physical cutoff law,
semantic waveform mapping, 150-control namespace/preset loader, full FX,
note/control lifecycle, both stock native fits and real callback evidence.
The current two-layer post-filter VCA core cannot be enabled as S1 and called
equivalent. The existing 180 dry / 72 dynamic-filter fixtures are the next
bounded sound tests; they cover zero intact 694-program presets.
