# Synth One wheel routing and ID141/142 receiver

Base: exact controls-init-only/source-ui-pitchmod (cb20b1ab). No Git,
presets, device or native changes. Keep the two patches independent.

1. wheel-target.patch (main_fm1.c only) fixes what the existing physical wheel
   targets, not its range. Stock Synth One wheel incorrectly targeted the
   Chicago parameter; its actual performance bend identity is 0. Resolve
   the active bank under the already existing owner hold, and use the same
   target for physical and incoming bend motion traces. The old branch for
   Chicago, Miami and SuperJ6 is unchanged. BREAK owner unchanged.
   Host fixture extracts exact smk_wheels/target helper text, feeds synthetic
   ADC readings through the real code and engine command queue on both
   cold part owners. Peer is unchanged even if the page switches during a
   gesture. The old reader had four endpoint failures; corrected 17 checks
   pass and center restores exact legacy target. No physical hardware run.
2. range-receiver-probe.patch is opt-in X0X_S1_BEND_RANGE_PROBE=1 and not a
   product build or storage assignment. Source controls metadata proves
   ID141 -24..0 (default -12) and ID142 0..24 (default +12); portable_voice.cpp
   proves asymmetric exact raw law: lower - branch scales (8191-raw)/8191,
   upper scales (raw-8192)/8192, then 2^(signed semitones/12).
   Two center words 8191 and 8192 both use the exact existing DETM base.
   Upper endpoint 16383 is one integer MIDI step shy of the maximum phrase;
   do not approximate original with symmetric 8192 divisor. ID140 stays
   transient. Probe composes with the wheel patch for testing, but does not
   depend on it to accept standard MIDI. No permanent settings or knobs,
   production macro default, project migration, or claim original PCM parity.
   Tested real host MIDI packet queue and rendered held voices with distinct
   asymmetric fractional ranges, several near-center/boundary values,
   exact neutral and noninterference. Scalar receiver inverse is exercised
   for every 14-bit input on both owners independently. Parameter target,
   not full waveform pitch/phase identity, is compared against exp2f.

Replay from the retained packet with run-wheel.sh. The old-source arm
intentionally fails endpoints. The wheel arm applies only the one-file route
patch. The range arm additionally enables the isolated probe. It includes
the same physical route checks, but ±12 opt-in defaults are **not** a
compatible replacement for legacy ±2 without a new admitted storage/UI
design. No current storage was deemed sufficiently owned for two ordered
continuous ranges.
