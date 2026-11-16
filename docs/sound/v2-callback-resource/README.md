# Per-callback resource diagnostic: still FAIL

An optional compile-time diagnostic observes thread resource counters and CPU IDs outside the original timed region. It does not change DSP, voice count, effects, floating mode or deadline arithmetic. Every timing sample remains retained; counters are associations, not causal proof. The benchmark/runner/source closure is captured and checked as before. Twelve focused tests pass.

The eight-program instrumented pilot had 25 wall /23 CPU misses, zero nonfinite words, benchmark TracerPid0. None of the25 missed callbacks coincided with a recorded page fault;8 coincided with a context switch. All thread resource CPU deltas on missed rows broadly track the thread-clock costs, including additional probe overhead. This does not support a fault-only or wall-scheduling-only explanation. No single cause or correction is established.

The subsequent default-mode control run still fails with7 wall /7 CPU misses and zero nonfinite words. All8 diagnostic FNV output digests agree with the earlier tracer-bound run and resource pilot; FNV agreement is not a full cryptographic/audio-oracle qualification. The whole-process gprof profile includes cold construction/validation, so its percentages cannot all be attributed to callback work.

Exact small result receipts are included. Full trace/resource CSVs, source snapshots and commands remain preserved locally, with identities in the receipts; no new denied trace upload was attempted. Next: independent diagnostic source review, then separate-host characterization and an exactly tested static-sequence validation optimization if its invariants hold.
