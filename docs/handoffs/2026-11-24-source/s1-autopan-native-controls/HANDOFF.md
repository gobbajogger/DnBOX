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
