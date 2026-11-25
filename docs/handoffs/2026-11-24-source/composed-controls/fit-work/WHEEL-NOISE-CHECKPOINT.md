# All-controls diagnostic successor, not installable
Apply sequential-names.patch, 909-noise-local.patch, and typed-bend-word.patch
after the v4 diagnostic fit patches. They cover distinct source files. The
typed carrier retains all 14-bit MIDI values without changing the physical wheel.

Source manifest: 90b99af1a0111c6fab1a8106f9727ab6066331bab229847fea09d49ce25d4294
Paired native link: Elite APP 552,408 bytes (264 free), Pro APP 552,400 bytes
(272 free), both +464 bytes above the hard RAM pool reserve.
Post-fit strict loaded scene: Elite offset 0/1 12,419 / 12,375 us and 3/3
dropouts; Pro 12,385 / 12,115 us and 3/4 dropouts. Offset -1 had no active
PCM and was not counted as a timing pass. Both loaded variants fail the gate.
The typed input path passed 167 host assertions. No hardware writes or release.
The guarded native asset placement does not support a mounted shared BREAK pool.
