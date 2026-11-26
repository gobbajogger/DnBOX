# Native isolated timing – SuperJ6 warm delay coarsening
Exact previous source-only candidate warm-coarsen.patch applied only in a
diagnostic Pi32v2 build. No production app boot or release image. The emulator
boot envelope is projected from its pinned known stock Elite package, preserving
boot bytes and regions outside the application. Fresh virtual NOR only.
TIMER4 uses the same 24 MHz modeled counter as actual firmware render
instrumentation. The emulator CPU-cycle register reported zero, so not used.
The same pinned strict --mhz 360 harness used for earlier full scenes reports
its unexpected 5760 MHz header here and in prior full product runs; these are
relative emulator TIMER4 readings, not physical silicon timings. No test of
simultaneous scheduler/double-slot/drops. Both diagnostic runs assert exact
unchanged entire virtual NOR and zero flash operations.

Cases per pass: full1024 ratio2 at wrap1023, 1023 ratio2 near opposite wrap,
full1024 ratio8, ratio64 at sync120, ratio512 at sync12, unchanged stride.
Initial typed owner has signed stereo history, nonzero signed pending sums,
real positive signed-loop feedback and settled delay/wet state. Tests isolate
the actual control config and first 480 audio frames under silent new input.

At full ratio2: stock reset takes 17.42 us first test/cold vs 1.21 us warm.
Candidate coarsen takes 122.88 us cold vs 102.92 us warm. Stock first block
237.62/226.33 us cold/warm vs candidate 329.96/319.96 us, while stock outputs
zero and candidate emits 480 nonzero samples. Combined per-owner incremental
about 198 us cold / 195 us warm in this bench. Ratio8–512 are less expensive
in configuration; ratio512 creates no audible signal at bounded first block
due to physical/read/history geometry, not an assertion of a passed extreme
audio parity case. Compare simultaneous candidate vs active stock steady
cost separately; do not multiply or infer scheduled worst-case headroom.

An optional optimization treats ratio2 via true signed truncation and rolls
pointers in general. 24 warm / 4,114 guards on host independently match the
old candidate numerical receipt byte-for-byte. No adoption from host. The
measurement receipt time.json **initially covers base/new only**, not opt,
unless updated with genuine lock-protected opt capture.
If still too much for existing heavy margin, shadow strategy would allocate
at most 512 stereo cells =2048 B/owner ratio2, work on oldest complete groups
first before overwrite while keeping old ring live, mirror new complete old
cells as part of chronological grouping, then switch format on an occupied
coarse boundary with physical time and target rescale atomically. Need buffer
lease and measured bounded worker; not yet implemented. An in-place deferred
write on fully occupied old delay would corrupt live reads without a proven
non-overlap admission. At current near-deadline scenarios this candidate
requires loaded live-edit timing on both models or an actually timed sliced
alternative before considered for product.

## Completed optimized native witness
time-opt.json adds actual opt (18 rows) from the same strict locked preset
and scratch. Full ratio2 98.25 us cold / 75.75–75.79 warm vs old candidate
122.88/102.92, saving ~25/27 us. All three matched signed history/config
fingerprints and nonzero frame counts in each case; host output receipt
also unchanged. Render480 cost stayed ~320 us; active unchanged-stride
guard is ~323 us warm while silent reset ~226 us. Worst paired total
increment against reset now ~173 us cold / ~168 us warm; relative to the
ongoing active unchanged-stride guard about +73 us warm, not a held
release timing claim. Config fits neither budget by assumption, since
real heavy render near deadline and shared synced changes can trigger
both slots inside one CPU0 half. Use both-model independent matched
protected stress gates before adoption. General-pointer path is ~2.7–6.3
us slower in tested ratio8–512 than original candidate. It remains below
ratio2 largest. The logs and custody provide virtual strict timing only.
See OWNERSHIP.md and coexist.py for the executable zero-extra generic
split counterexamples; no extra arena availability claim.
