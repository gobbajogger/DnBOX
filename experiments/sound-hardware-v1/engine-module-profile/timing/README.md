# Synth One-only heavy-scene timing diagnosis

Start with the Synth One-only exact-width diagnostic and sealed replay at
a6b0b7cf. SuperJ6 off, built-in Synth One controls and arp retained. The
target is zero underruns and a worst 512-frame half of at most 10,809 µs.
These are smoke measurements at scene offset 0, not release qualification.

stage-trace.patch instruments the actual split job runner and saves the
per-core jobs, exclusive-stage waits, wall-clock jobs total, and remaining
render stages at the worst observed half. The trace is compiled only with
DNBOX_S1_STAGE_TRACE=1. Separate core totals are concurrent, not additive.
Nested work and shared stalls may be included; no pure instruction costs
are implied. Run with the same seed recipe and sample clock as the original
comparison. Use a distinct D9_LAB_OUT (joint-diagnostic-stage) and paired
elite-stage.log/pro-stage.log, then run run-stage-replay.py.

The two largest jobs of the traced worst halves are 909 (6.16/6.07 ms)
and Synth One A (7.99/7.72 ms). Wall time in the jobs stage is 10.11/10.39
ms; the rest of the render adds 2.72/2.38 ms. The stage waits explicitly
instrumented are not large enough to explain the whole overrun. The
aggregate 909 bucket includes dry clear, drum rendering and related
sends/mix. Optional stage-d9.patch splits its actual renderer by voice.
Apply after stage-trace.patch and build to joint-diagnostic-d9 with
elite-d9.log/pro-d9.log, then use run-d9-replay.py.

The adaptive scheduler control is deliberately uninstrumented: omit only
-DX0X_CORE_PLAN=4 from compose-flags, write to joint-diagnostic-adaptive and
*-adaptive.log, then run run-adaptive-replay.py. Observed 47 underruns on
each model versus fixed-plan 33/23. Do not replace the fixed setting
with adaptive based on this workload.

The drum trace's two worst halves agree: toms 2.358/2.355 ms; individual
sample voices are all below 0.5 ms. The first targeted 909 variant splits
the steady tom body after the attack has ended, with no different
oscillator, envelope, bus sum or sample table. The same-source Huffman
fixture compared 23,352 blocks each at -O0/-O2. All four PCM buses and all
6,624 bytes of mutable drum state matched. The fixture's compressed
stream compare used the real companion lengths rather than nibble lengths
(HH 9,286; crash 19,851; ride 21,746). Native timing is separate.

Reducing only hot Synth One translation units s1c/s1u from the -Oz set and
placing them in -O2 retains -fno-fast-math and -ffp-contract=off. The host
component produced identical PCM across six presets in wet and delay-only
modes versus the same source at -Oz. The two-board diagnostic did not
clear the gate: Elite 12,706 µs / 26 underruns, Pro 12,895 µs / 36. This
is not a match to the original delay or a native equivalence claim.

The original tape exercises the Synth One render role but is not, by
itself, proof of any active oscillator. To separate that case, use
voice-admission.patch, DNBOX_S1_VOICE_TRACE=1 and voice-midi-helper.patch
with the six reference Bass A note edges from 1c4a776d. Count admission to
voice_run_kr on the separate wired-USB run; do not compare its instrumented
timing as a perfectly controlled A/B to the original non-USB fixture.

The tom tail variant did not qualify: 12,822 / 12,747 µs and 29 / 26
underruns. Keep the patch as equivalence evidence, not a production change.

The actual notes went through the USB log and triggered 267,574 Synth One
voice-kernel entries on each model. In that populated diagnostic, Elite
ran to 18,945 µs with 232 underruns, Pro to 19,281 µs with 237. Both linked
with ~72.5 KiB free XIP and 9,016 B of pool beyond reserve. This is the
first note-admission witness on this specific modular profile, not its
first ever result elsewhere in the project. No installable image.
