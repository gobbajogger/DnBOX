# Existing strict runner cannot anchor this scene to owner + rendered frame
Runner pinned fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017.
The documented FM1_BENCH_KEYS and FM1_USB_SEND variables schedule independent
events in emulated milliseconds. FM1_WARMUP_MS only changes fixed time and
is listed for --bench, not as a data-dependent --shot readiness fence.
FM1_BENCH_TRACE, FM1_TPROF, FM1_DUMPRAM and DAC PCM files are observational and
lack an owner-gated event feed. Passive observer is additionally unavailable
for sealed custom projected APP content; its protected-package rule was not
relaxed. Public runner is a prebuilt executable, not rebuilt or modified.

Existing tape event #1 is 2220 ms. HELLO seq7 is requested at 1300; after actual
configuration, ACK varies across ELFs and, for examples inspected, comes only
after the tape starts. The owner request seq10 is issued at 2060, but no
qualified reply for it exists in the four examined donor/tom logs; the first
actually qualified role status is seq11 at roughly 2.9–3.0s. Four successful
GETs in a report are post-start point samples, not evidence of pre-roll
settlement. Probe.py merely copies owner.count while owner-scan's separately
computed allScheduledOwnerReadbacksAgree expects five good responses. Do
not use either as a wait-before-scene mechanism.

Existing exact same-ELF A-A archival repeats record identical USB admit and
owner-read times and bitwise PCM. Other baseline/candidate runs yield
different handshake latencies and first DAC-active halves even using exact
input tape. No B-B of the latest tom candidate was run merely to guess more.
Neither owner ACK wall time nor nonzero DAC establishes exact internal event
consumption frame, which is absent from this runner.

## Exact hook needed
1. Start identical guarded no-write projected boot and continuously run stock
   360-MHz audio/DMA. Sequentially issue four unique validated PARAM_GETs
   after HELLO; require actual expected synth A/B kinds/status, cutoff and arp
   values, no KEEP and no request error for each. Abort on timeout. Do not
   count replies after first scene event or accept duplicate values by count.
2. Arm the entire unchanged 922 authored event tape at the first documented
   audio-producer frame boundary after successful readbacks and observed
   readiness/no lateness; retain order of all equal-time events and every
   relative event interval. Derive boundaries from unaltered 44.1 kHz producer
   clock, not estimated DAC onset, external time, or tuning for a waveform.
   Full stock 512 half / 256 rendering / internal event cuts and strict
   deadlines remain unmodified. Use the same external physical/DIN paths.
3. Add a read-only ingress-consumption witness matching event identity,
   requested frame, actual consumed renderer frame/span and owner generation,
   plus first late half ordinal. An A/B may still differ because its
   normal physical/DIN polling takes other time. Fail closed on such mismatch
   rather than reordering, bypassing UI route, freezing playback or applying
   events directly to engine state. Start timestamps/counters and PCM must
   verify A-A/B-B determinism before cross-build comparisons.
4. Keep exact binary/model/package and sealed project SHA guards and
   byte-for-byte before/after NOR check, no stock or release-gate waiver.
   This hook needs supported runner/firmware trace capability; the deployed
   documented CLI cannot implement it with a shell wrapper alone.

## Upstream source corroboration (not pinned binary source)
Public source is joakimeriksson/fm1-emu snapshot
736aded65c8e2e9d652cf595fca248734483a877. No accessible source ref ba83317a
(404) in this upstream repository; do not substitute this source for the
guarded binary. Public main.rs lines 892–907 and 1024–1054 queue the input
tape before the headless loop. Lines 1269–1281 send USB requests strictly by
periph.ms and only print replies. This gives the precise integration points
for the missing preflight/test hook above. Virtual MIDI is supported only in
the live transport, which is paced against wall time, not owner+producer
clock. Public fm1-soc/src/periph.rs audio_frames_so_far() includes boot DAC
time and leading silence, and is deliberately distinct from I2S frames the
firmware successfully rendered. It must not become the readiness threshold.

Reference source:
https://github.com/joakimeriksson/fm1-emu/blob/736aded65c8e2e9d652cf595fca248734483a877/crates/fm1-live/src/main.rs
https://github.com/joakimeriksson/fm1-emu/blob/736aded65c8e2e9d652cf595fca248734483a877/crates/fm1-soc/src/periph.rs

Offline audit.py derives six chronological receipts from preserved exact
logs. compare.py demonstrates fail-closed verdict on A-A and both model
candidate-vs-donor pairs (exit 2); even identical A-A cannot establish
actual sample admission. Neither script writes settings, firmware, cloned
flash, or emits a replacement timing verdict. Full no-xrun would explicitly
witness absence of a first late half (as opposed to null because no trace
source). The current runs already report late>0 but no indexed first event.

Authorized project checkout search found no Cargo.toml/main.rs emulator sources
or ba83317a commit object in local project repositories; only the restored
prebuilt binary. Library title search ba83317a, fm1-emu, fm1-live returned
no matching emulator source package. Public source exact-ref lookup earlier
returned 404. No off-target toolchain or emulator build/timing substituted.
