# SuperJ6 separate Delay: source-authenticated 24/27 bounded host route

Exact source-wheel-core-parity manifest 5b9df43bac1c8cf8f6a34f243c4cab1f8d0cd3e73183436329dace203d979c81.
Original Build9 SHA a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1.
Original Conductor bindings:
24 Enable ivar 0x100809c90 → 0x1000f1270; raw Float32 >0 sets
TempoDelay.isOn. OFF sets AKStereoDelay DryWetMix=1, **not graph suspension**.
ON re-applies authored Mix.
27 Mix 0x100809c98 → 0x1000f1424; retains normalized raw even while
OFF, and when ON sends dry coefficient Float32 cast of (1−.9×u^1.1).
26 Feedback 0x100809ca8 → 0x1000f13c0; 0..0.99 linear.
25 Time 0x100809ca0 → 0x1000f08ec; free 0..1.5 seconds linear.
Synchronized branch indexes round-away(u×17); entry zero uses free
time, remaining tempo-grid values from jump table 0x100609873:
[.0234375,.03125,.046875,.0625,.09375,.125,.1875,.25,.375,.5,.75,1,2,3,4,6,8]
times four beats (240/BPM), still bounded by actual receiver capacity.
Do not substitute ALT3's different rate grid.

Original AKStereoDelay running DSP 0x10035a3e8 calls independent
time/feedback/mix rampers in chunks ≤8, then stereo core 0x10034aac4.
Core uses dry*x+(1−dry)*tap and maintains wet recurrence while dry=1.
Non-ping-pong uses separate channels with fractional tap interpolation;
the fixed integer steady sub-case becomes two independent float rings
with FMA feedback. Native DSP constructor includes 10,000-frame ramp
defaults; no exact AU host scheduling duration inferred for other events.
Ping-pong path entirely separate and not covered by this fixture.

Patch is gated by X0X_HOST plus X0X_SJ6_DELAY_WITNESS. Only explicit
caller-owned stereo line lengths and qualified known steady integer-tap
/ no-ping-pong / running ramper scope admit its DELAY page. K1 ENABLE and
K4 MIX are actual editable distinct original endpoints; K2/K3 remain
empty (no bogus time or feedback controls). Changes are runtime-only,
no dirty saved Sound and no borrowed MIX FX bus. In current compact graph
post-chorus/pre-master; original delay is post-chorus and feeds later
processing not wholly present in DnBOX. Host fixture supplies fixed 250 ms
and .5 feedback as controlled admissible points, not original defaults.
Exact dry bypass is measured after the receiver ramp settles, not
promised during a fade; history remains independent and continues OFF.

543 real host assertions across both physical owners, direction, label,
page admission, audible engine signal, peer, writes and saved Sound
isolation. Direct fixed-tap impulse verifies left/right isolation and
successive feedback echo amplitudes .75/.375/.1875. This is not original
full wet graph A/B, native capacity, runtime measurement, or approved
production activation. At just 1.5 seconds, original-scale float stereo
for two owners needs 1,058,400 B at 44.1 kHz before metadata.
Independent compact line design/rights are separate. Time and Feedback
must remain off-panel until actual dynamic fractional retiming,
interpolation cadence, contextual tempo clock and feedback qualify.

Replay in isolated source-wheel-core-parity with usual host build inputs:
apply delay24-27-witness.patch, copy sj6-delay-probe.c to tests/host and
run this run.sh. No product, presets, device, Git or release changes.
