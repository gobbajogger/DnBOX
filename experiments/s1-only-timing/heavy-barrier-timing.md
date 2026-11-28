# Elite native two-kit dry Synth One: active work versus wait

Apply the diagnostic-only `heavy-barrier-timing.patch` to the preferred 69-row
source stack. For the comparison, apply the explicit separate-branch ADSR
sharing patch before adding the same trace. The profile retains separate
per-core counters; normal project, kit, note and mixer code is unchanged.
It records peak held/early-release halves and fixed generated-frame IDs in
`bench_barrier_rows`, not wall-clock positions. Run the same loaded fixture
and read `read-barriers.py audio RUN` alongside `read-loaded-timing.py RUN`.
The few extra timestamps change cycle/cache alignment and can perturb the
adaptive scheduler in short spans; do not claim uninstrumented perf from
these runs. Wait columns are inclusive in job elapsed; do not sum twice.
Takeover is counted separately at final join. No device or release writes.

Both instrumentation runs validated six real voices and 132 real hits per
kit, 128 held halves and zero skipped cycles. In the three non-initial held
samples with matched frame IDs the envelope arm's Bass A non-stage-wait
work fell by **1,545.8 / 1,640.4 / 1,593.0 µs**. The measured stage-2 wait
rose by **1,338.5 / 1,681.3 / 1,626.6 µs**, and complete half time changed
by **+92.46 / +11.17 / +35.83 µs**. Thus the envelope change removes work
but not elapsed time under this placement.

At the baseline's slowest held half (generated frame 139,776), Core 1 spent
5,634.33 µs in the 909 job and 6,177.29 µs in the remaining 808 job. Its
808 body wait was just 15.58 µs; the synth on Core 0 waited 1,462.33 µs
for stage 2. Core 0's 808 body cost 1,450.33 µs; its Bass A job was
9,056.84 µs excluding stage-2 wait. Post-join processing cost 948.17 µs.
So waiting is a symptom, not expensive polling. The immediate bottleneck
is the drum work left on Core 1. In this measured half the sum of active
work on the two cores also exceeds the two-core physical budget once the
serial tail is included; balancing alone with the baseline jobs unchanged
is unlikely to pass.

Same-frame worst early release (171,520) had a different plan. The
instrumented baseline routed 909 plus 808 to Core 1; the instrumented
explicit ADSR arm put 909 on Core 0 ahead of Synth One. Do not interpret
the 147 vs 151 whole-scene dropout count or this difference in release
timing as proof of a held-path gain.

| Separate uninstrumented experiment (Elite) | Held peak | Whole XRUNs |
|---|---:|---:|
| Preferred 69-row reference | 14,065.42 µs | 151 |
| Extend the prior reverse allocation into first 16,384 release frames | 14,071.71 µs | 160 |
| Move the 808's three toms to Core 0 on that existing held reverse route only | 14,594.46 µs | 151 |

The fixed release extension removed the roughly 15.3 ms whole-scene
peak, but increased XRUNs; keep it out. Moving all three toms is too much
for Core 0 with the original envelope loop; keep it out as a standalone
change. None of these experiments reduced voices, changed the cymbal, or
qualified zero dropouts. No DSP promotion at this point.

### Combine the two complementary changes

Apply `adsr-share-branch.patch` and `808-rebalance-r3.patch` together, without the release
extension or the diagnostic timing probe. Core 0 renders the contiguous
808 kick/snare/three-tom prefix, then starts the unchanged six-voice
Synth One render with the guard-proven envelope reuse. Core 1 does 909 and
the remaining 808 work. No cross-core each-sample changes, no added blocks
of latency. This pairs actual Core 0 work savings with load moved off the
critical drum core.

Elite: 13,115.29 µs held peak (950.13 µs lower than preferred baseline),
86 of 128 held halves above the physical period (42 inside), all 128 still
above the engineering target, and 111 whole-run dropouts (151 baseline).
Worst whole-run elapsed remained 15,291 µs. Quality still requires a separate
fingerprint run. Do not promote based on this report, host performance, or
the lower count.

Strict Pro: 13,106.67 µs held peak (959.16 µs lower than the preferred
reference), 86/128 above physical, 128/128 above engineering, 108
whole-run XRUNs (151 reference) and 15,339 µs whole peak. Both Pro and
Elite linked with 33,260 B pool margin with the 8,192 B reserve intact,
and XIP margins of 1,132 and 1,124 B respectively. A separate Elite
pre-DMA correctness build matched all 160 ordered held and release
fingerprints. That is evidence for the exact dry fixture, not wet or
all-preset parity. Use this pair as a new **dry diagnostic** starting
point only. Keep release/installer unchanged. Next measurements should
isolate the new worst held/release per-core times and expensive
drum/render work rather than repeating the envelope-only trial.

### Where the paired arm spends its time

A separate, instrumented Elite run of the paired arm identified a new
worst held half at generated frame 134,144: 13,211.50 µs total. Core 0
now spent 3,125.08 µs on the expanded 808 prefix and 7,467.87 µs in the
Bass A job excluding the 199.46 µs wait for 808. Core 1 spent 4,981.50 µs
on 909 and 4,202.38 µs on the remaining 808. Post-join took 1,074.42 µs.
On the worst early-release half at frame 171,520, Core 1 still ran both
kits for 13,106.58 µs. The waits fell during sustain; the active work
on Core 0 plus serial work is now just as important. On release, the
old route regains control and is still too concentrated on Core 1.
Next, measure a smaller contiguous drum prefix on held spans and
a cost-aware release route separately; the rejected blanket release
extension is not a recommended change. Full fit and dropout metrics
above use the separate uninstrumented runs.
