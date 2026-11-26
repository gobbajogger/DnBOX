# Same fitted donor, strip only existing 909 duplicate witness
The input had a verify-only future: Core1 runs a duplicate sampled voice
before its 808 prefix/Synth One job; Core0 advances the actual voice, compares
whole voice state and PCM against the future, and emits its actual samples.
Removing diagnostic duplication leaves bus mix order, event paths, phaser,
Range, fixed-head, clocks and original fixed C1 mask. No alternate future,
new kernel, timeout, BLE, storage or gate policy adopted.

Pinned source-only helpers remove seed/worker/compare code. Removing only
dispatch left generic compare internals linked behind null, so final candidate
removes those dead call arguments and branches too. Core0 serial code uses the
old selected d9_render_smp path. Whole-state+four bus on the directly linked
stripped renderer vs original verify-only shadow: 56,000 spans, 42k ready/14k
missed, zero state/PCM differences.

Final scoped native link +1248/+1260 APP free, +564 pool. That is +480 B
measured APP recovery vs duplicate donor, not tiny leaf accounting. No
shadow/verify symbols retained; both sealed ELFs and no-write timing log are
exact. INIT-only; no matched whole-scene A/B or full product qualification.

Fresh sealed diagnostics, 360MHz unchanged original 922 events offset0:
Elite 14,301µs / 79 XRUNs, Pro 14,394µs / 77 XRUNs. Both zero skips and
writes, nonzero audio. In contrast to 87/104 drops for shadow donor, meets
neither deadline nor dropout limit. Elite changed from 4 to 3 point owner
readbacks; producer-frame admission still absent on both models, so the
top-line reduction is not causal timing or native PCM equivalence.
