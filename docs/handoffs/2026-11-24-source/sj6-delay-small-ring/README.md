# Independent compact two-owner SuperJ6 Delay test

The 12-bit packed random-access stereo ring stores left/right in exactly
three bytes per frame per owner (not block-predictive, no large tables).
Finite 7-bit segmented mantissa with 4-bit exponent, sign, Q19 scale.
Two strictly independent owner allocations carved from an explicitly sized
exclusive caller-owned span, with strict exact length, overlap and
overflow rejection. No MIX sharing, no stock/audio pool allocation.
This is an executable storage option, not product allocation or active app
selection. Desktop float-model and typed physical proof remain unchanged.

Replay with python3 replay.py. It extracts only the pinned typed/SYNC
DSP source from its two patch receipts. It compiles that exact Float
float-cell reference and a view with the ring get/put seam substituted,
not a wholly reimplemented DSP. Shared time/feedback/mix/clock laws,
settled Enable behaviour, ramp, fractional tap and feedback ops (FMA),
owner timeline and dry branch are unchanged. Out-of-range sample, a
separate compact-format restriction (>7.9375 magnitude or non-finite),
latches an explicit owner range fault and dry, rather than wrapping or
silently continuing with saturated wet; no output limiter or adaptive
resume is implied. DSP clock changes beyond selected capacity decline
before new targets start; the existing clock-refusal contract holds
dry and continues advancing history. No generalized compact amplitude
headroom assurance for arbitrary feedback/material.

At 44.1kHz with a 125ms admitted span (5,514 cells per channel with guard),
two stereo owners use 33,084 bytes of storage, versus 88,224 Float bytes.
At full free 1.5s the same codec takes 396,906 B for two stereo owners
(1,058,416 B Float). Longer sync spans can exceed even the free range.
With the previous comparable ~540 B headroom, even before code/state,
packing both stereo owners leaves at most 90 cells/channel and <2.02ms:
first nonzero Free (1/127) needs at least 522 cells/channel and 3,132 B,
and first nonzero synced grid at 120 BPM needs 2,069 and 12,414 B.
That headroom cannot admit a useful full parity profile. No zeros-only
claim, selecting one owner, feature cut, cap clamping or borrowed MIX
ring. Resource rearchitecture and chip evidence are needed separately.

UBSan compiles/runs same actual DSP, both owners simultaneously different
Time/SYNC/mix/feedback, opposite-channel silent, first delayed echo
same exact sample index as Float. Actual waveform dry bypass is bit
exact, and a note impulse during clock refusal arrives after returning
to representable clock (history survived). No ring canary changes, no
peer cursor change, explicit oversize/clock/profile and range refusal.
Wet is measurably lossy even on this one dual impulse: 41.45–45.48 dB SNR,
0.000874–0.001075 full-scale peak error. No blind correlation across
signed-zero wet values. Host throughput timings in receipt are diagnostic
only (sanitized stock Float vs compact), never SMK block/original A/B
qualification. No current stock fit, product UI/format, package, preset,
native device, release or remote changes.
