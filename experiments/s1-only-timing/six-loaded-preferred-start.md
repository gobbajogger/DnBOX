# Keep the original 808 cymbal in the next loaded iteration

Apply the same optimized six-dry baseline and `dry-six-drums.patch`, then `dry-six-stationary.patch` and `808-reverse-body.patch`. Omit `808-cy-staged.patch`.

Strict Elite run, same six note presses/releases and 132 actual hits per kit during the held window: 14,311.96 µs held peak, 15,460 µs whole-run peak, 151 full-run XRUNs and zero emulator skipped cycles. App free 2,948 B; pool margin 26,492 B. All 160 ordered pre-DMA fingerprints matched the original loaded benchmark in separate correctness builds. It saves 2,224 B app flash for only +15.92 µs held peak versus the narrower staged-cymbal variant. Treat the single XRUN difference and modest run-level peak change as timing details, not a qualification or preference based only on dropout count. This is the better starting point for additional work, not a release.

Moving one or two toms into the prefix as well as kick/snare did not help the staged-cymbal branch: 14,486.17 / 14,918.17 µs, each 152 full-run XRUNs. The kick/snare-only profile in that same branch had ~9.65 ms Synth One plus ~1.36 ms 808 body on Core 0 and ~5.19 ms 909 plus ~5.89 ms 808 remainder on Core 1, normalized per 512 held frames. The jobs are already fairly balanced, before stage waits, peaks and final mixing. Next, attribute final-bus and handoff costs and lower total compute needed to render the same sound; avoid another blind reassignment of whole jobs.

No hardware, installer, production defaults, or expanded synth scope.
