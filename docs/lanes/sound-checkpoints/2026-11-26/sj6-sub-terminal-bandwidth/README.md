# Sub-only signed terminal bandwidth, not a provider

Reuse the archived actual SuperJ6 Brainwave Sub-only AUv3 captures.
`python3 probe.py ../sj6-direct-auv3-fanin` verifies original binary,
PCM hashes, original cue, original program identity across repeat and
fixed readbacks, and then measures 13 anti-aliased integer decimation
factors. Reconstruction uses zero-phase polyphase analysis/synthesis
on the original time origin. No timing/level fitting, steady-state
loops, extracted source zones, audio output files or product edits.

At 4× decimation the unquantized reconstruction is −59.70 dBr whole,
−58.53 on the chord and −51.87 on the final release. At 8× these fall
to −41.82, −40.80 and −32.72; at 32×, −17.83, −15.30 and −14.34.
Authentic fresh repeats are −50.37 whole, −52.04 chord, −81.67 final
release. With fixed full-scale 8-bit codes, even 4× is only −19.90
whole; 12-bit at 4× needs 111,513 B for this one stereo tape with
flat quantization and still reaches only −43.67 dBr whole. No gain
normalization or fitted offsets were used in those comparisons.

The capture contains effect/routing output (pinned bypass predicates)
and several notes. These quantities describe rendered audio and the
tested codec, not storage for a mono source bank, upper keyzones,
behavior at source wraps, a universal codec bound, or the processor
cycles of a deployed decoder. They are useful evidence against an
extreme global rate cut or naive fixed 8-bit assumption. Require
actual signed strike, loop and Raw holdouts before any source or
storage recommendation; no Mac session needed for this result.
