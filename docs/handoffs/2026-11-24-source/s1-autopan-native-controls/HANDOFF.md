# Synth One authentic AutoPan target math

Ports the already verified Synth One Rate/Amount receiver in the exact
893 mixed-controls tree to JieLi-compatible scalar maths. Same physical
controls, ownership, sync division, Float32 project Hz and sidecar, phase
cadence and equal-power output. Keeps the prior control graph default
untouched: explicit X0X_S1_AUTOPAN_PORT enables only this math variant;
native typed project admission is a separate change.

The free-running rate edit uses the exact 128 original detents and 127
positive Float32 decision boundaries. The phase update uses positive
nearest-even and a constant correctly rounded 44.1 kHz phase scale
(the shipping S1 part rate). A normal single-precision division covers
alternative configured rates; cadence does not reset on edits. The
incoming domain is already finite and clamped. No added mutable state,
buffer, allocation, page, package or new field. Forward/reverse constants
use 1020 B read-only XIP. Not an overall synth audio or timing claim.

Source-pinned UBSan verifies all forward detents, local inverse
boundaries, 1,048,576 sampled Float32 inputs, 4,194,304 independent
phase steps and 1,522 tie roundings. The actual two DSP source units
compile for JieLi pi32v2 without unresolved lrintf, powf or soft double
division. Actual physical host mixed acceptance rereun with this port
enabled (893 plus prior 813 rejection before writes and existing random
controls). This is not a full native firmware link or SMK test.

Replay: use replay-leaves.sh WORKSPACE, replay-native-units.sh WORKSPACE
EXACT_893_TREE, then replay.sh (with the exact source-mod-wheels donor).

## Exact native trial, independently qualified after the leaf check

The opt-in read-only typed carrier plus this AutoPan port now physically
links with the guarded diagnostic companion on Elite and Pro. Exact
post-port uses the fully reconstructed latest 893 donor, the sealed
native-trial carrier, root-prune and the existing unchanged 909 lab span.
It adds no preset archives and never emits an installable package.
The actual linker reports 9,928 B and 9,940 B of app space, both models
544 B above the 8 KiB POOL reserve.

Four actual firmware emulator cases drive real SMK input to AUTO PAN on
parts A and B. Physical K1/K2 modify amount/rate in the target-natively
validated carrier; readback verifies the assigned DSP parameters and
authored clock. The unedited peer and entire synthetic flash are exact;
real key input produces nonzero PCM. This is not an acoustic waveform
comparison to the original app beyond the separate panner receiver proof.

Strict timing is RED: all 219 halves late in each of these short trials,
with roughly 18.6 ms worst. These controls are not a release. The previously
published S1PK and this diagnostic data span cannot coexist; no physical
hardware writes, installer changes, or normal release are authorized.

The exact native stack also needed source-faithful math on already
composed SuperJ6 receivers (SDK lacked the host-only Double runtime).
The native-only extra patch preserves Double-then-Float32 rounding without
aliases, using independently tested exact integer significands and the
full original 128-entry velocity scale. Test receipts cover 3,148,928
cutoff results and 4,113,536 signed LFO combinations. The explicit lab
carrier still rejects the unsupported velocity route; it does not route
unsupported host features through another control or alter stock-off code.
The native link otherwise retains the existing typed DSP profile.

Replay native only with local private build inputs:
1. Run materialize-and-link.sh WORKSPACE CARRIER TEMP_UL MODEL (both model links
   can share the same TEMP_UL). Carrier is the sealed read-only trial packet.
   Verify PILOT.sha256 in TEMP_UL/src; generated inputs must match the
   current guarded wheel-native-scale baseline. Use only the existing static
   909 assets; do not package output.
2. In TEMP_UL/src/labout generate relative ELF SHA256 lines; use the
   existing guarded relocation seed-non-user.py with a separate
   disposable /tmp directory (no --run) to generate bound inputs. Either
   original published test archive is used read-only as a boot template.
3. Generate target-native offsets with pan-layout.sh WORKSPACE TEMP_UL,
   then run-native-pan.sh WORKSPACE TEMP_UL SEED CARRIER MODEL OWNER, where
   OWNER is 0 or 1 and the sole simulator slot is available. Synthetic
   portable project comes from the original carrier writer, not an
   imported user project.
