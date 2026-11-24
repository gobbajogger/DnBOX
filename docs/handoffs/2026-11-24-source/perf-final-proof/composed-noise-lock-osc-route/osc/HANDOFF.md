# Synth One original Osc Mix LFO (ID50)

Exact lock-repaired Noise source cc188cb2f36cb97cb8c3fb85fbccd0289272a8f494d4f16f79f2a346c33bc3e0;
joined full source f77d26a5a3a51956f398d677155612a278e0f3c059dad5c1f1bb4c68c042fc09.
Six app files only; the Noise ownership fix is separate and required.

Original S1Parameter.h ID50 oscMixLFO is 0–3. Preserved portable_voice.cpp
adds the selected rising unipolar source to oscillator balance and clamps
0–1 before linear crossfade. Original kernel combines LFO1/LFO2 by
averaging. Compact kbal/lfo01 already implements these rules; no DSP,
LFO, modulation curve, noise, rate, phase, or topology change.
New MOD ROUTE K1 is OFF/LFO1/LFO2/BOTH. It fills the remaining 15th
Synth One part page; no prior page truncated or removed. Both directions
reach all 15 on both owners. Fourteen SuperJ6 directional screenshots
remain byte-identical prior/after.

Typed Synth One base route group retains Noise bits24–26; Osc Mix
uses a second grouped lane bits27–29. Absent=0; explicit original
0–3 encoded 1–4; codes 5–7 and high bits30–31 rejected. No new record
extent, discriminator, buffer, or mixed cutoff/HP/Widen/Legato writes.
All 25 independent optional route pairs exact and all 64 raw pairs
classified. This exhausts independent three-bit lanes, not every
possible future format. For more destinations prefer a validated
migration of a defined whole group, not reuse of other fields or
arbitrary high bits. The fifteenth page still has three free controls.

1,992 host checks: authentic active and exact zero-depth programs,
all three active sources each affect 4096/4096 per owner with distinct
output, zero-depth 0/4096. Peer PCM, raw balance, depth, rate, phase
and shape exact. Real lock release and stop restore factory. All 16
explicit Noise×Osc Mix pairs in both owners captured as valid notes
with packed cutoff/HP bytes unchanged. Exact prior guard reader 269
checks: old Noise/cutoff/HP/Widen Sound and locks accepted, new Sound
and lock rejected without writes. INIT+694 unedited 1024-frame PCM
comparison byte-identical.

Replay: bash replay-host.sh EXACT_LOCK_REPAIRED_NOISE_SOURCE AUTHENTICATED_S1_PACK.
No native/device, Git, root mutations or overall original DSP parity.
