# Authored-wet-only S1 room diagnostic
One patch against exact guarded Dual-Room old pair (14560fd6…/0612a1ae…)
makes the first room choice at the existing one-time delay partition.
A positive canonical [0,1] initially authored mix plus room enabled
and delay off selects RV8; otherwise RV4 remains warm. All live 0/wet,
enable and delay toggle actions preserve that already chosen topology
and state. No skip of a zero-output reverb history.

Before first partition the previously attached pool already zeroed the
whole existing 2,100 B room arena, so a second RV8 clear is redundant.
Checked exact extent 2,100 B from a dirty 0xA5 preimage for both owners,
with the following Delay byte unchanged; construction agrees with
s1u_rvb_attach words + words / 16 + 128 == S1C_RVB_BYTES used by attach.
The candidate doesn't borrow Delay or change any global memory.

Reproduce native strict link:
  DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1 bash replay-near-maps.sh

Paired real native APP headroom: +24 Elite/+36 Pro, +564 above POOL
reserve each; guards and companion span unchanged. Float multiplication
and inline integer experiments were rejected at 52/40 B over; separately
outlined cold function was 68/56 B over. No relaxation of hard limits.

Host matrix compares ordinary 4-line and old 8-line component respectively.
Both 65,536-frame trajectories and matching continuous-warm control shadow
histories agree bit-for-bit across every 64-frame block, with independent
owner arenas. Time-matched events are in host/*/trajectory-*.txt. Six dry
and all five not-Organ authored wet outputs match RV4 bit-for-bit; the
Organ wet output matches the improved dual-room candidate bit-for-bit.
Thus prior authentic Organ comparison unchanged (last-second -0.049 dB,
left/right spectral errors 2.568/2.748 dB; waveform null still not pass).
No Pad Delay fidelity change nor specific preset changes. This is an
isolated guarded diagnostic, not a device image or release.
