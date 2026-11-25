# Native SuperJ6 warm stereo delay comparator

No release, packages, device writes, or device hardware. Two native guarded
images per model, matched except for the delay cell-centering patch. A sole
emulator lock is required, target clock 360 MHz, strict no-skip. Both arms
also carry the same already-known DNR3 delay Time/Feedback descriptor fix
from ecb5e158. Exact linker receipts give both arms' flash and RAM budgets,
not a loaded release qualification.

This diagnostic builds a synthetic Init project with the existing
project/tuple readers. The new tuple is created only in a disposable
virtual NOR image. The typed project is necessary: selecting SuperJ6 on a
stock legacy project loaded a Juno-compatible owner, but never armed the
typed delay ring, so positive DNR3 writes reported conflict and those dry
native probes were discarded. No missing, refused or stale control reply
is treated as a pass.

1. Make two guarded donor trees, apply the same delay descriptor patch to
   each, then add center-only.patch to the trial. Run their exact guarded
   native builds and existing emulator-only projector. Save the projector's
   temporary directory output to the projection files.
2. Compile project-layout.c using the exact app headers. Generate
   delay-layout.s with the target flags from the guarded build (same as
   the authored-wet room's layout procedure). Use the existing
   room-native-recovery/fixture as the LZ encoder. Use the ordinary cold
   stock project adoption read as the init-cell source to authored-seed.py;
   the script checks its shape.
3. Run run-pair.sh for each model and both arms. wet-controls.mjs verifies
   same settled owner through actual USB DNR3 replies; the warm fixture
   starts at feedback 63, writes 64 before notes, independently verifies
   the five controls, and confirms five notes. delay-witness.py separately
   confirms an armed, nonempty ring and effective wet .90 / feedback .499
   from target RAM.
4. Compare measured counters for identical scope in every arm. This is
   five notes with active delay, not the 922-event heavy scene or full
   original FX parity. Do not redistribute the produced images or PCM.
