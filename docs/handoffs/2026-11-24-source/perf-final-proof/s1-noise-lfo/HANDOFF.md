# Synth One original Noise LFO (ID53)

Exact canonical pooled cutoff source 83f1490251d47a3b16b7d7a6f72ec3513a4ee740e9c8592e52507956f1e09ff2, joined full source ledger 888724f2d2fb1e586503f876cae6298a6148495357c1877d4b59532f3a025722.
Only six app source files changed. No native, Codex, Git, device or shared
product writes.

Original S1Parameter.h ID53 noiseLFO, range 0–3 and exact original
portable_voice.cpp path: noise only multiplies the descending amplitude
source, originally unity when off, LFO 1, LFO 2 or arithmetic both.
Compact existing noise receiver already implements the same modulation.
No noise generator, RNG, panning, delayed-line layout, filter algorithm
or cutoff topology changes. FM / NOISE K4 real OFF/LFO 1/LFO 2/BOTH, no
new panel or parameter alias.

The same 32-byte immutable-shape Synth One Sound cell stores one typed
optional three-bit field (bits24–26 of the existing edit word): zero
absent, 1–4 explicit controls, 5–7 rejected. Old validators reject
all these new bits in Synth One, while each other engine uses its own
discriminated validator. Base mask persists prior Legato, Widen, HP,
cutoff and LFO depth without any reinterpretation or packed-tail
write. New records retain the usual whole-cell note lock and no-write
old-reader veto. No new independent marker or extended extent.

Host 927 focused checks. Both owners; all three modulating selections
each touch 4096/4096 frames for a genuine authored noise preset, and 0/4096
for the authored zero-noise control. Each genuine mode is distinct.
Both phases, depths, rates, target/peer stereo and untouched packed
tail remain exact. Physical K4, lock apply/release/transport-stop,
save/reload, and busy no-write checks. Exact previous reader 267 PASS:
prior cutoff/HP/WIDEN Sound+lock accepted; new Sound+lock rejected
before changing any of six project objects. Untouched INIT+694
1,024-frame PCM identical. Existing cutoff rerun 26,342 checks PASS.
Both-owner panel direction walks retain 14 Synth One and 15 SuperJ6
part panels, with 14 old directional SuperJ6 LCD frames byte-identical.

Replay: bash replay-host.sh EXACT_POOL_CUTOFF_SOURCE AUTHENTICATED_S1_PACK.
No overall audio algorithm equivalence or native fit is asserted.
