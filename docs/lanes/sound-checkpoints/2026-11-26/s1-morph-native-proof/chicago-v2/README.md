# Chicago branch and stereo-main workload: safe negative evidence
Given prior exact native critical path traces, Chicago voice+clock consumes
1.88–2.33 ms of measured worst blocks on Core0, excluding ~0.60–0.74 ms
wet processing. These tests target fused actual Chicago code inside that voice,
not a new scheduler or a 909 change.

Matched donor is retained exact table-share no-transaction. In isolated PI32v2
16,384-frame real kernels, exact state-safe sweep showed:
- Carry-based five-Float32 aggregate nonfinite guard: -1.90 to -2.74% slower
  in all four held/ramp, with/without XMOD. Output hashes unchanged. Host
  4,344 matched including deliberately nonfinite filter result (unchecked
  overflow should be rejected). Guarded two-model link saves 16 B each,
  leaving 948/956 APP and identical pool margin. No complete-scene timing pass
  was attempted when the leaf lost in every case.
- Shift then greater/equal compare to combine the same guard: -0.33 to -4.21%
  slower in all four. Native leaf output hashes unchanged.
- Hoist held oscillator half-gain from each sample, retain exact branch mask
  from original gains even when lossless half multiplies to signed zero:
  -1.18 to -5.86% slower. Actual baseline and candidate C voice-span compared
  4,344 cases including ±subnormal gain and signed-zero branches, exact state
  and outputs. The legacy host per-sample mirror normalizes one signed zero
  differently in both baseline and candidate held state; actual frozen
  donor is paired directly in this test instead. Native all four hashes match.
Both candidate concepts increase executed work on actual PI32v2 even when
static code shrinks. They are not contenders for the remaining >2 ms timing gap.
Power/display/USB timing not evaluated on these negative leaf fixtures.
