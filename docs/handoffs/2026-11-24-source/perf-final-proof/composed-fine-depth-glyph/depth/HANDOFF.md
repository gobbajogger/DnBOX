# Synth One original LFO depth pair

Exact grouped control-only patch on pinned 5bf045d03a95eca770d1527098ce8c49c5ec0f2bea2e4a69517e453791394cc5;
full before/after manifests included. No native, Git or root writes.

Original S1Parameter.h IDs 14 and 46; Kernel/S1DSPKernel.hpp both
0–1, default 0. Original Kernel/S1DSPKernel+process.mm and compact
s1u_lfo_tick use matched depth algebra for both LFOs and both
polarities: .5(1+wave)*amount and 1−.5(1−wave)*amount. Bound only
the amount targets, not adding a destination or changing LFO source,
phase, shape, rate, tempo, note handling or signal-memory topology.
TEMPO SYNC K2/K3 show real %; K1 retained; last slot unassigned.

Same base cell. Old validators disallow bit4/5 and require bytes4/5
zero (without these bits); each typed position 0–127 plus absent owns
only its prior canonical byte. Note-lock base is already copied in
full. No packed tail changes, no dependencies on other edits, no
widen marker reservation. Old reader accepts old Part Level + lock,
and refuses both new Sound/lock types before zero-write verification.

Host: 5,421 checks, both affected owners. Both authentic cutoff-routed
cases change 4096/4096 selected owner frames. Neither unrouted case
changes a frame. Peers are sample identical, compared phase positions,
rate and shape are exact. First detents follow the displayed target;
factory precision and owner state restored at note-lock rollback and
preset reselection. Previous reader 523 checks. Untouched INIT+694
audio digests identical at 1024 stereo frames each. Actual both-owner
directional walks preserve all 15 SuperJ6 and 14 Synth One part pages,
no page count change. The 14 SuperJ6 actual before/after LCDs compare
byte-for-byte. New LFO depth row is captured directly in the two PNGs.

Replay with bash replay-host.sh EXACT_VOICE_UI_SOURCE AUTHENTICATED_S1_PACK.
Data and generated PCM remain local only. No global DSP parity,
no hardware, native fit, timing or publishing claim.
