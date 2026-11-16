# Full original Synth One sequence/arp composition contract

This is a source-bound, caller-owned experiment, not an admitted firmware port.
The original full sequence/arp oracle is authenticated at AudioKitSynthOne
6466a3715c96b7ecf1dd255a218cbd571408a314. Its timing, arithmetic, callbacks,
atomic objects, and container behavior are retained in the host-only binding.
source-sequence/verify_source.py checks the six exact public inputs and complete
oracle token bodies after explicitly listed platform bindings.

## Serialized target architecture

SerializedSequence is owned by ONE audio/render execution context. Construct,
init, reset, set/get notes-per-octave, get beat count, and process are serialized
with each other. No direct concurrent UI reads/writes are supported. The product
adapter must deliver controls through its independently qualified owner/event
handoff and publish copied beat snapshots/notifications. This requirement is
consistent with the current caller-owned Scheduler contract; it is NOT proof of
that future handoff or arbitrary original-app thread interleavings.

This architecture uses ordinary signed32 owner fields, not an atomic IRQ shim.
The pinned pi32v2 compiler lowers BOTH relaxed and sequentially-consistent atomic
int loads/stores into SDK __sync_* routines, whose real compiler_rt requires
unprovided __rt_local_irq_disable/enable services. Neither controller-only IRQ
nesting code nor fabricated stubs are substituted. The original host oracle's
atomics remain unchanged. A different owner model requires new implementation
and qualification, not ignoring this contract.

## Behavior and access

The host reference keeps the original binary64 per-sample time and fmod.
The firmware-target compile (`S1_NO_DOUBLE`) stores those same binary64 bits
and updates them with integer add, divide and fmod. A step therefore crosses
a multiplier boundary on the same sample as the original clock for every
finite tempo, multiplier and sample rate, for as long as the sum stays finite.
It preserves all supported source control ranges/casts, all three arp directions,
1–4 octaves, interval scaling for notes-per-octave, 1–16 sequence steps, on/off,
octave boost, pitch offset, held note order, chord velocities, and callback order.
The selected-index algorithm replaces temporary note-list construction, not the
musical sequence. Fresh original resize(1024) initialization has1024 pending zero note-offs.
Restore keeps the existing pending-note prefix and pads with zeros to1024;
this behavior is retained, not silently fixed.

All input spans must be mapped/readable for declared lengths. Controls, held
records, owner, and callbacks must not race/change during a sample call.
Callback sinks need capacity, must not mutate inputs/owner, and must not recurse.
The standalone API rejects nonfinite/out-of-source-range controls, invalid held
MIDI coordinates/counts, and original-undefined arithmetic domains before
callbacks/state mutation. The host oracle is not invoked on undefined domains.

SourceScheduler owns its sequence state and validates its WHOLE final owner
against borrowed packed/sine/tuning spans and output spans. Packed canonical
content authentication remains the loader's responsibility. Cold initialization
constructs a fresh experiment graph. Restore retains original sequence init
behavior, including existing pending notes/clock and appended zero entries.
The public cold init adapter is not an original-app lifecycle emulator.

Render/control calls are serialized. Public set_notes_per_octave returns false
and preserves the WHOLE live owner when proposed tuning would make original
integer conversions/additions undefined. Prospective control targets and restores
are checked against current tuning before any mutation; a safe large tuning is
not arbitrarily capped. Render preflights the current sequence domain before DSP,
ports, counters or output writes. Sequence-relevant controls are unsmoothed.
Input/event/range/alias validation rejects preserve the live owner. No full-block rollback is promised after illegal
borrowed-input mutation, forged internal state, callback violations, or an
original-undefined long-running counter domain. This is not a full app MIDI,
sample-accurate cross-thread scheduler, UI, FX or storage admission.

## Proof boundaries

The actual compact engine is compared with separate original raw-table note
code, original parameter ports and complete original sequence/arp bodies.
The original oracle never calls the compact reader. Host sample/state equality,
native objects and a retained API ELF do not establish target execution, correct
platform placement, full product resources, CPU, acoustic app fidelity, device
behavior or the whole hardest-configuration zero-discontinuity gate.
The host reference still uses binary64. The firmware-target closure is compiled
without soft-double helpers. `X0X_SYNTHONE=1` is the opt-in firmware measurement
build in BUILDING.md. It does not by itself admit the image.
Default LeakSanitizer failure under ptrace is retained; detect_leaks=0 passes
are explicitly not leak-detection passes.
