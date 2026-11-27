# Synth One isolation pilot (not installable)

The paired vendor links are a measurement baseline, not a feature-complete
Synth One firmware. Built from the previous RAMTEXT guarded donor, both
linked images include the compact Synth One renderer and omit the new SuperJ6
renderer/object, while preserving legacy Juno. Bank-free; arp was enabled.
The compact S1 typed trial and typed Remote are off in this measurement.
There is no saved-project fallback policy change, audio parity claim, or emulator pass.

The links completed; the old dual-engine diagnostic hook stopped afterward on
its FITTAB inventory assertion because the reduced image has a different set
of live tables. No package was produced. Use the measured physical section
sizes and audited stripped/selected symbols in linked-receipt.json, not a
packaging or timing pass. XIP uses the actual ELF load offsets and final marker.
Do not attribute the entire XIP difference to engine isolation alone.

The first attempt with full S1 typed controls found independent guard and
ownership couplings; see typed-compile-blockers.txt. Fix those without
dropping S1 controls or silently substituting legacy for typed projects.
The RAM floor and package inventory gates are not waived.

Reproduce: copy the RAMTEXT donor directory beside prereqs/, apply diagnostic.patch
with -p1 from the copied donor root, then run bash run.sh under the shared native
build lock. Expect the two isolated ELFs and maps before the FITTAB seal rejects.
Then run the profile audit on the ELF and map using vendor pi32v2/bin/nm.
The temporary union assertion change only permits observation; use the actual
linked memory receipt, not the replacement assertion, to validate occupancy.
