# Dual Synth One room fit rejection at packed-live-key baseline

Base loaded-perf-pool-live-keys strict guarded diagnostic pair: +496/+508
APP, +564 POOL; all bounds and exact external immutable bytes unchanged
as baseline semantics. This lane operated on copies only. No testing on
physical units, native audio emulator, install, source graft or Git.

1. Applied prior independent eight-line room patch fuzz 0 (line offsets
   only), removed original host-only compile rejection in the scratch to
   estimate real native size. Both actual model objects compiled before
   link was attempted. The ordinary 4-line and added 8-line room state
   still alternate within the exact single existing Synth One room arena;
   delay keeps its own history. No static RAM cost.
2. Exact strict guarded pair failed XIP by 612 / 604 B.
3. Safe compaction: share canonical IMA step/index tables and exact decoder
   rounding with existing engine primitive. Proof enumerates 1,424 step/
   nibble cases of original shift-add delta versus shared integer formula;
   sign, signed saturation and index clamps unchanged. Consolidate the
   identical sample accumulation, interpolation and reset statements of
   both different-rate rooms. Their coefficients, scheduling count,
   effect selection, state and 4/8-line kernels remain.
4. Optimized strict links fail XIP by 412 / 400 B, improving 200/204 B
   versus unoptimized. No final image marker added on these failed links.
   POOL remains 0x51dcc (+564 B above original 8 KiB reserve), BSS end
   0x1c1ff54, RAMTEXT 0x1c0111c. Cannot approve or graft this branch.
5. Host fixture of both exact target source variants runs six canonical dry
   and six authored wet programs with private bank fixture only. All
   12 optimized vs unoptimized host outputs bit-identical. Six dry and five
   wet optimized vs fitted are byte-identical. Organ last-second error
   -4.697→-0.049 dB; spectral L/R 9.800/8.525→2.568/2.748 dB; waveform
   residual +1.263 dB relative. No objective parity sign-off or additional
   numeric tolerance invented. Program-specific retuning absent; selection
   is the existing prototype's first-render gate policy (delay-off/reverb-on).
   Native INIT-only factory profile cannot be judged by host FDATA output,
   nor may its gameplay timing be inferred from the host.

dual-room.patch is unified against the exact packed-key donor. Reproduce
with replay-near-maps.sh; DNBOX_CONTROL_BUILD_ONLY=1 stops before linking.
Host fixture must compile S1 with the same canonical IMA tables available
(either real bundled 909 object or X0X_IMA_TABLES_DEFINE in the standalone
component). FIT.sha256 pins real logs and map. No private PCM or binary
is appropriate for any checkpoint.

Exact replay requires both cold-path caller flags, which the baseline
driver does not enable on its own:

    DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1 bash replay-near-maps.sh

Initial diagnostic runs with those shell flags absent had unrelated
Chicago table deltas (+512/+256, packed table replacement symbols
-153/-78 and helper -138). Do not use those initial runs as source
cost. See *.exact-delta.txt for corrected map comparison and
map-delta.py to reproduce on exact paired maps.
