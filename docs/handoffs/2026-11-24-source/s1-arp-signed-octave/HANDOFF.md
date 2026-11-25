# Synth One ARP SEQ signed octave entry

Exact base: `perf-final-proof/controls-init-only/source-s1-mono-sj6-id1`.
One firmware UI file. No storage or scheduler changes, no timing claim,
no native, hardware, Git, bank or release operations.

The pinned original contract is in `firmware/src/dsp/synthone/scheduler/
source-sequence/source_sequence.hpp` in its sequencer branch: at 12 notes
per octave, offset = base < 0 ? base - boost*12 : base + boost*12.
The currently shipped compact scheduler already uses precisely this law
in `firmware/src/dsp/synthone/sequencer/S1Sequencer.c`. Physical split
pad+keyboard ingress instead unconditionally subtracted 12; roll display
unconditionally added 12. An old user input can therefore have a display
different from what the engine emitted.

Invert the actual Synth One law: a boosted -24..-13 uses base -12..-1;
boosted +12..+24 uses base 0..12. Anything else is unrepresentable with
OCT+ retained (`OCT+ RANGE`, no edit and no incidental pad-release note
toggle). OCT+ or STEP ON remain independent; unboosted behavior,
SuperJ6 UI behavior, the event clock and wire rows are unchanged. The
roll display uses the effective signed offset, while held K5 remains
labelled OFFSET and retains the authored raw offset semantics.

Base focused fixture passed compilation and failed 10/30 checks; the
patched same 30 checks passed, as did the unchanged dual-engine 66
keyboard checks. The expanded endpoint fixture checks both owners. It
uses actual panel input to toggle OCT+, actual split pad/keyboard ingress,
keeps peer bytes unchanged, checks in-memory project save/reload, and
wraps only the real Synth One backend note call (forwards every call):
root 48 produces note 29 for the entered negative key and sends its
actual note-off. No simulated scheduler or assertion of visual capture
parity. Existing arp timing gates are separate.

Proof commands: apply patch at zero fuzz to an isolated exact source,
verify PREIMAGE/POSTIMAGE. Create host fixture helper with
`sed '/^int main(void)/,$d' tests/host/ui_arp_keyboard_test.c > tests/host/arp_keyboard_helpers.h`,
copy this packet's test.c to tests/host/ui_s1_arp_signed_octave_test.c,
and use run.sh's flags with the existing pinned host asset headers. The
fixture sets only mode/on/length through the existing engine API to
isolate the real one-step scheduler; the corrected controls traverse the
physical UI, not direct setters.

Remaining original distinction: ID64 is BPM (not a multiplier); ID147
is a beat factor quantized by S1Rate::nearestFactor to 19 source stops.
Current original_runtime.c accepts both without applying them and uses
DnBOX transport SPEED. Implementing a true independent original clock
needs a separate sidecar admission and phase policy, not aliasing Saga
division marks or multiplying the already-quantized SPEED arbitrarily.
The split ARP/SEQ phase policy remains unchanged.
