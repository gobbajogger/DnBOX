# SuperJ6: reached original filter-envelope update

This complements the [original app signal path](superj6-original-app-signal-path.md). This is a read-only receipt derived from execution of the original ARM64 sampler, not a whole-app or board A/B pass. Original image SHA-256: `6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850`.

## What is different

The compact firmware interpolates its own envelope values across a render tick. The original filter envelope advances once at the sampler's update boundary and feeds that value to the cutoff calculation. Its attack and decay advance in callback-local 16-frame slices, including a split at a two-frame callback remainder. Sustain uses its authored value and skips the recurrence; when a moving segment crosses the endpoint, the output clamps to the endpoint and advances the segment.

For the original curved segment setup at image offset `0x398048`, start `s`, endpoint `t`, curvature `c>0`, width `n>0`:

```
m = exp(-log((c + abs(t - s)) / c) / n)
a = (t + (t > s ? c : -c)) * (1 - m)
next = fma(m, current, a)
```

Note-off uses the live double-precision current value, not the rounded start value in the descriptor. In the observed release, those were 0.4652406513175238 and 0.4652406573295593, respectively; using the rounded value changes both coefficients. The measured filter-envelope update is at `0x3b59c0…0x3b59d4`. The zero-width curved branch assigns the endpoint immediately. The linear branch stores a slope; the curved FMA rule is not a substitute for its advance path.

## Proof and limits

- Directly executed segment prep: 784 curved and 196 linear cases with exact tested coefficient words, covering rising, falling and immediate segments.
- Observed 3,089 original update words bit-identically predicted by FMA, across two notes with the authored Brainwave filter settings and two deliberately short diagnostic envelopes. All 10 reached attack/decay/release descriptor rows matched coefficient generation exactly. For Brainwave the first two held values were 0.00042445711664674054 and 0.0008487669359331984. The compact renderer is not assumed to match those values.
- The separate fast-envelope fixture changed physical attack/decay widths; overriding the attempted release control before note-on did not change the reached release width. Do not infer the public release-control mapping from this receipt. Steady sustain, all preset routes, full range of callback schedules, and installed AU terminal sound are not claimed as covered.
- Receipt SHA-256: direct setup matrix `8270ee48b9e9be0030cef2b78dde9152f89f17dbcef66a9b81599777b11d6365`; reached descriptors `1849444fda3b952842f0f886f532de44dc98601d3a86549adfca9bfbec688148`.

Fix the stage coefficients, update boundary, and position in the cutoff signal path as shared engine behavior, then compare signed renders with the existing untuned presets. Do not fit gain per preset or deduce the original filter timing from final RMS.
