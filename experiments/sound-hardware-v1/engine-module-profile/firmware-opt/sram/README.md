# Reversible Synth One voice placement experiment

Donor is s1-focus-vstage, same active chord cue, four job plan and fixed 922
events; no changes to phase, controls, envelope decisions, voice order or FX.
voice-sram.patch extracts an existing every-eighth-sample block into
an exact noinline XIP call. voice-sram-far.patch additionally provides
immutable absolute call veneers read through volatile pointers for both
directions. The native PI32 compiler warns -mlong-calls is unused; direct
branch relocations across firmware SRAM and XIP exceed its range. No linker
range bypass or manually placed device code was used.

Host exact PCM and complete voice/RNG states: 576,672 frames on all frozen
controls, LP/BP/HP, 1/6 voices, morph/blep changes, mono/poly, multi-note
release/new admission, including 744 internally active arp spans. The
whole board uses external arp, unchanged. Native CPU-only data relocation
is reversed in both far-arm variants for sufficient executable RAM.
XIPfar and SRAMfar use the exact same source and far-call cost; only
DNBOX_S1_VOICE_SRAM varies. Board includes do not alter initial state.

No actual device, package output, flash install, shipping gate or Git.
Both guarded captures must continue to require the exact prior project
seed procedure, stock flash guard, strict runner and retained event tape.
