# Synth One Resonance LFO source ID49

Exact canonical predecessor f77d26a5a3a51956f398d677155612a278e0f3c059dad5c1f1bb4c68c042fc09;
full joined source 8e786d0a6423b19345dffcc3d01b1275c53f8dd0a93bdd939d8e707fb524519e.
Four app paths only, no native, device, Git, product or Codex writes.

Original pinned S1Parameter.h and Kernel/S1DSPKernel.hpp assign
resonanceLFO ID49, 0–3. Pinned original portable_voice.cpp multiplies
resonance by selected descending LFO source, then clamps; combination
uses both-source average. Low Pass consumes the resulting resonance;
Band Pass converts it using .0625*sr*(2^(1−resonance)−1);
High Pass does not use it. Compact uses the same control data flow
and bandwidth conversion. MOD ROUTE K2 exposes real OFF/LFO 1/LFO 2/
BOTH, no new page, no change to runtime audio algorithms or filter type.

The existing Synth One typed route byte keeps each old Noise/Osc byte
exact if Resonance is unset. The previous-invalid bit-7 family
0x80–0xe3 ranks the 100 explicit new combinations as
128+25*(resonanceCode−1)+5*noiseCode+oscCode, with zero meaning unset
for each existing field. Exhaustive state test: 125 valid unique
tuples; 131 remaining raw bytes fail closed. No other note, tail, mask,
or preset bytes changed. Each of all 64 explicit route triples
was captured through actual note lock in both owners while preserving
the exact existing packed cutoff/HP/Widen extent.

7,592 focused checks cover three distinct routing modes on both
owners and authentic low-, band- and high-pass presets. Each mode
changed 4096/4096 low/band pass samples, 0/4096 high pass. Target
base resonance, peer stereo PCM, both LFO phases/shape/rate/depth
exact; held editing, lock/unlock/stop, busy no-write rejection and
save/reload passed. Prior exact reader accepted old Noise/Osc/cutoff/
HP/Widen Sounds and note locks; refused new Sound/lock without
writing (271). INIT+694 1024-frame unedited compact PCM bit identical.
Both owners still walk all 15 Synth One/15 SuperJ6 panels, no overflow;
14 untouched SuperJ6 LCD examples byte-exact. Retained Osc fixture
is adapted only to expect that K2 is now occupied and to use a still
invalid header as its negative canary.

Replay: bash replay-host.sh EXACT_OSC_MIX_SOURCE AUTHENTICATED_S1_PACK.
No claim of original overall audio equivalence or native fit.
