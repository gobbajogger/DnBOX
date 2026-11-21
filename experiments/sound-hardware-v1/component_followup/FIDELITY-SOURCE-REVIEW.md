# Sound successor: independent fidelity review and implemented numeric unit

## Outcome

A new, source-free S1 envelope proposal now reproduces **all 379,288 supplied
binary32 scalar observations exactly** across 3 static and 7 live-event cases.
All four deliberate wrong behaviors reject. The root owner can implement and
verify this small reusable component immediately from
`IMPLEMENTER-CONTRACT.md` and `proposed_s1_envelope.py`. This is new executed
progress beyond the recovered audit; production code was not changed here.

The original unresolved findings remain: current production S1 is silent;
the common candidate is a partial core; complete S1/SuperJ6 behavior, private
FX, native callback evidence and full instrument gates remain open. No fit
or sound-quality gate was loosened, no original engine code was reused, and
no native build/full suite/shared executor work was performed by this review.

## Actual execution

The root released both serialized native links before the numerical check.
The executable Python proposal consumed only the two pinned output packets.
Its actual run completed in 0.458 seconds, maximum RSS 13,204 KiB, exit 0:

```text
EXACT_COMPONENT_MATCH
379288 samples
0 different binary32 words
0 nonfinite outputs
4/4 deliberate wrong variants rejected
```

`numeric-result-01.json` records per-case hashes/errors and the source SHA.
`NUMERIC-EXPERIMENT-DECLARATION.md` predates execution. No metric is fitted,
no output time is shifted, and no tolerance was introduced.

A separate fresh byte comparison reused the earlier candidate C outputs:
all 81,920 retained old-core words differ from the corresponding 5 live
oracle cases. That check completed in 0.031 seconds with 11,520 KiB maximum
RSS. `red-existing-candidate.json` explicitly records that the old C binary
was **not** rerun. The existing C candidate source hash matches the earlier
fidelity audit (`9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa`).

The observed old AHDSHR mismatch is not repaired by changing its stage times:
the old normal trace peaks at frame 2,204 instead of 6,092, sustain edits snap
instead of approaching the target, and its 16-sample SJ6 policy differs from
the measured per-sample S1 recurrence. Preserve SJ6's separate profile.

## Why production S1 is silent

Fresh authenticated GitHub reads at live head
`b90faafd2de71063448beea67aa5d22938a5c64b` confirm:

- `firmware/src/app/original_runtime.c`, blob
  `4561eb9d20972151a76629cfc433b320594533dd`: the S1 render arm explicitly
  zero-fills both output buffers and returns success (lines 354–357).
- `firmware/src/app/original_runtime.h`, blob
  `3c5b8570aa0c1f7f905c199c8431986c87c0156a`: the union's S1 owner is only
  `uint8_t down[128]`, explicitly described as a placeholder.

The prepared native source independently retains S1 initialization as key
state, note handling as key-down tracking, no-op physical parameter handling,
and a zero-valued envelope view. `original_sound.h` exposes two S1 parameters
and one Init factory. Wiring the generic core into the render arm would make
audio but would not implement the 150-parameter/694-program instrument.

The prepared native tree's Juno adapter differs from b90faafd; these sources
were not equated wholesale. The two fresh GitHub blobs above establish the
live S1 finding directly.

## Concrete remaining code and test units

| Unit | Current source/evidence | Required implementation and bounded verification |
| --- | --- | --- |
| S1 envelopes | New numeric proposal exact on 379,288 scalar samples; current shared core differs | Root implements separate per-note amp/filter states; compile its real C component and compare both pinned packets word-for-word; preserve update ordering and independent states |
| S1 dry topology | `dnbx_render` filters each layer first, then applies amp/velocity; normalizes by 6 and half-pans channels | One common per-note oscillator sum, component gain and squared velocity, VCA before filter, mono duplication with absolute gain; no fitted output gain |
| S1 waveform/control law | Generic waveform endpoint 1 is square, generic cutoff ends at 20 kHz, modulation is additive Hz | Map S1 endpoint 1 to saw and explicit bandlimit ID146; implement `clamp(C*(1-D*(1-E)),64,22050)` with legal D=1.2 |
| S1 filter | Generic two-pole bounded recurrence has different excitation and transfer | Independent DnBOX-derived four-stage/nonlinear realization; run preserved 180 dry +72 dynamic-filter matrix at RMS<=1 dB/spectral<=3 dB, gain1, original metric; then BP/HP and live type-history tests |
| S1 control/presets | 150 IDs, 144 ordered writes per program, 46 smoothing memberships; production slot has 2 fields | A typed 150-ID adapter, ordered application, dependent controls and smoothing, free/sync rates, oscillator2/sub/FM/noise, tuning, LFO routes, mono/legato/glide and event lifecycle; replay real factory paths |
| S1 FX | Existing recovered full graph has rate reduction, autopan, phaser, filtered stereo delay, compressed reverb, master compression and widening | Independent fitting graph preserving tail duration, warm restore, bypass, channel ownership and two-part isolation; prior rejected codecs/layouts stay rejected |
| SuperJ6 controls | All 218 `dnbx_store_raw` IDs are storage-only; no automatic DSP binding | Reconcile named 64-control requirement with 218 identities/170 order positions/167 ordinary applications/174 serialized names; test each actual audible route at enabled settings |
| SuperJ6 preset identity | 640 programs/12 banks retained; two unresolved serialized names | Preserve exact raw values and present-zero fields, global ownership212–214, reserved207; do not invent aliases for `neontubeRageAmount` or `neonlimiterAmount` |
| SuperJ6 complete sound | 42 selected-component captures/942,080 frames at48k; full preset N01/14/56 and some modes not authenticated | Use component captures within scope; recover exact missing original outputs and route dictionaries; do not relabel S1 56-program evidence or bypassed envelopes as full SJ6 coverage |
| Whole runtime | S1 silent; prepared compact core only partial; flash/timing qualifications owned separately | Both simultaneous stock models, unchanged reserves, private mutable histories and FX tails, real full callback/stack evidence, exact qualified gate; no activation from a scalar component PASS |

The source locations above are from the recovered package's
`experiments/sound-hardware-v1/hardware_candidate/src/dnbx_core.c/.h` and its
`mapping/*/implementer_brief` packets. The recovered late audit at
`/workspace/scratch/de30bbe568fe/fidelity-work/FIDELITY-AUDIT.md` retains the
prior shortcomings and measured controls. This review did not duplicate its
full original-source inventory or regenerate original recordings.

## Smallest useful next delivery

Finish the new S1 envelope component and focused C tests first. A proposed
12-byte state holds level, one latched coefficient, phase and prior gate;
both amp/filter remain separate. A prepared-coefficient interface keeps the
per-sample state machine independent of the eventual target math choice.
The numerical check used double `exp` rounded to binary32; target coefficient
preparation and native cost are still unqualified. Do not silently substitute
fast math and call it the measured result.

After that component is integrated, finish a six-note one-raw-saw dry profile
with the required signal order and filter. The existing dry matrices are an
appropriate next step but cover zero intact full presets. Full S1 and SJ6
control/FX gates remain separate closure requirements.

## Files authored here

- `proposed_s1_envelope.py`: independently authored executable numeric proposal.
- `NUMERIC-EXPERIMENT-DECLARATION.md`: pre-execution scope and exact comparison.
- `numeric-result-01.json`: real positive and negative terminal results.
- `red-existing-candidate.json`: fresh comparison of retained old-core outputs.
- `IMPLEMENTER-CONTRACT.md`: detailed C state/update contract and exact fixtures.
- This review and `MANIFEST.json`: ownership, source map and file integrity.

Root remains the sole repository writer. All work here is confined to the
assigned isolated review directory; include it in the existing Sound
GitHub/evidence publication and recovery package without creating a parallel
production lane. No GitHub/source/index mutation was performed by this agent.
