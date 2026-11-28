# 909 tom partial staging: rejected for native timing

Apply `tom-partial-spans.patch` only on the unshared 69-row diagnostic
baseline from `display-lossless-69.md`. It walks one partial over the active
span at a time. Phase and envelope are local, and the additions occur in the
original per-sample order. The same scratch buffer is used.

`check-tom-stage.sh CANDIDATE_TREE BASE_DRUM909_C` passed 23,280 comparisons
at each of O0 and O2, including all state bytes and four output buses. Two
paired host tom-only timings improved by 8.1–14.2%; that was not a proxy for
target timing. Native Elite linked at 1,892 B free app flash (+128 B used),
and audio pool margin stayed at 33,260 B. The compact-ADSR composition also
linked at 1,204 B app free, but was not run after the independent arm failed.

Strict Elite: 14,211.67 µs held peak and 15,410 µs whole peak vs 14,065.42 and
15,293 baseline; both had 151 XRUNs. All 128 held halves missed the physical
deadline. Six voices, 132 actual hits per kit, no emulator skips. Leave the
native baseline unchanged. No Pro timing, deployment, or production claims.

A separate instrumented Elite comparison matched all 160 ordered pre-DMA fingerprints.
