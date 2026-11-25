# Host-only Synth One original clock wired through the actual app

Base: `controls-init-only/source-s1-reverb-lfo`, manifest
`f74afed6ce7fc9087c4b1e3a8b2e53a3e5e5e2ad57fc1ea878ce5c49399b077b`.
Patch `clock-app-host.patch`; independently reproducible via `replay.sh`.
The byte-level math/reference packet is the sibling `s1-arp-clock-receiver`.

`X0X_S1_FENCED_CLOCK` cannot be enabled without `X0X_HOST` and is off
by default. Its serialized test owner-config accepts independent BPM 1–200
and original factor 0–18; no new wire, no alias to project tempo and no new
control page. The active experimental receiver is free-running Synth One
only at GATE 100; otherwise the existing DnBOX engine/transport retains
its timing. After active transported playback, wait for held keys to clear
before returning to independent phase. Neither stock nor experimental
settings require rewriting any project's wire.

Tests exercise actual host input, production SMK live key admission and
owner event queue, shared sequencer, voices and direct audio render.
Physical ARP page and GATE continue to work, audible on both parts, and
specific source onsets land at sample 137 and 206, respectively.
Transport compares a key held across STOP with and without a host owner-clock
configured; then the next newly pressed key resumes original phase.
Separate unchanged legacy PCM comparison is against unmodified base,
running the existing 55% GATE stock ARP/SEQ fixture.

Product enablement remains unproven: an explicit user-facing distinct BPM/
factor policy plus persistence, sub-100 GATE interaction while transitioning
from an active experimental clock, and native flash/real-time arithmetic
cost still require qualification. This packet is not a build or release.
