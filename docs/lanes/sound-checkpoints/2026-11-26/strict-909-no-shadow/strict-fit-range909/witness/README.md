# Paired attribute-only native diagnostic
Exact current S1+Low Range+909 donor, source files patched with port.patch.
Magnitude of diagnostic link delta relative to uninstrumented donor
+352 B APP, +36 pool used, +416/+428 APP remaining, +528 pool usable.

Worst detected *actual* DMA miss: Elite ordinal 419, 14,342 us, TIMER5
17,951 24-MHz ticks (747.96 us); Pro ordinal 390, 14,789 us, TIMER5
18,951 ticks (789.63 us). Each five stretches mask 4, 808 split prefix,
zero dispatch fallbacks, before/after half 0→1. Timer measurement is nested
handler elapsed time, not audio wall-time redistribution or exclusive
hardware overhead. Even if all of this time vanished the misses would still
exceed 10,809 us. Flag open_join means loop ever had wait state 1; existing
HAL returns 0 on success and some code omits writing state 2, so do NOT
attribute this flag to timeout/failure. No more code claims from this.

Uninstrumented exact-pair result was Elite 14,817 us / 87 late; Pro 14,603
us /104. Instrumented changes to 14,342 /83 and 14,789 /74 are evidence of
diagnostic perturbation *not* speed or exact-overhead changes. All four
runs have byte-identical before/after simulated NOR and no skips. No data
about actual producer-frame consumption or pre-zero settled owners; never
a gate pass. Loading this code onto a physical device is out of scope.
