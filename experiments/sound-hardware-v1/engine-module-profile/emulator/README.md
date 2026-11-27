# Synth One-only, exact width: sealed emulator diagnostic

The packed BREAK retrigger table belongs to the SuperJ6 factory build.
With SuperJ6 off, breaks.c selects the existing formula. The revised
sealer removes the absent inventory row only for that profile, verifies
the formula's six compiled timing constants in app XIP, and keeps the
original checks of remaining payloads, hashes, headers, and protected
ranges. It also omits the absent SuperJ6 velocity table. No product
DSP, flash partition, or device state changes.

Reproduce from the RAMTEXT donor and the parent checkpoint's typed
instructions. After applying the typed patches, restore the donor
original_runtime.c (verify width/preimages.sha256), apply
width-diagnostic-layout.patch instead of width/exact-width.patch, and
append -DDNBOX_S1_EXACT_WIDTH_LAB=1 to compose-flags.txt. This corrects
the diagnostic type assertion: the old Miami comparison did not
include the expanded controls' pre-existing footprint. The fixed
assertion verifies the actual union arm; both native maps independently
verify 9,016 bytes of audio pool beyond 8,704 bytes of reserve and
74,284 / 74,292 bytes of free XIP. Do not apply this as a change to
production layout guards.

Apply single-engine-seal.patch to the typed donor's already patched
link wrapper, and use a fresh D9_LAB_OUT. Use the stock guarded replay
helper with single-engine-replay.patch for separately hash-pinned
emulator candidates. The removed 500,000-byte historical minimum was
not a guard against BLE overlap; app upper bound, hash pin, checksums,
and protected-range comparisons remain. No installable package or
physical writes. Cold boot is not an audio stress test.

This stage exercises the existing compact delay, not an exact or newly
matched delay implementation. No delay parity claim is attached.

Loaded run: seed each candidate's own cold RAM-derived project using the
existing room-native-recovery synthetic Synth One A / two-octave arp
fixture, without activating its Room-specific edit. Verify seed and
candidate SHA, run the existing 922-event heavy scene at offset 0 under
FM1_STRICT and FM1_NO_SKIP, and keep the virtual NOR byte-identical.
The scene was audible, but missed deadlines: Elite 12,604 µs, 33 xruns;
Pro 13,029 µs, 23 xruns. The run preserves the entire emulator pipeline
and does not establish where those overruns came from. A same-source
compact-width baseline is required before attributing them to widening.
