# Synth One original cutoff modulation source ID 48

Exact latest ALL-POST fine-glyph dependency manifest
eb0431ad701c6d585fab17b35a2536db855c20c8bc2df1ed92c6defa8a6c34ea.
Joined full source ledger c76fd3034e5c2151592a233bc7a99d0903b490ff88fab1588475a6e396c74d2f.
Only five app files change. Replay operates in isolated host copies.

Original Synth One S1Parameter.h ID48 cutoffLFO, [0,3], and preserved
pinned portable_voice.cpp receiver (cutoff multiplier before envelope
depth). Original kernel lfo3_1_0 is the arithmetic average. Compact
already selects/multiplies the same target in voice_run. FILTER MODE
K3 now edits OFF / LFO 1 / LFO 2 / BOTH. Existing knob count, rate,
phase, envelope law, gain and topology do not change; original whole
filter engine equivalence is not asserted.

Representation extends only the existing high-31 HP/WIDEN escape.
Keep all previous legal header bytes exactly (including original
00/00 wet/Legato and f8/f9/fa valid HP/Widen). Rank every explicit
route into one of the old previously-invalid Widen>2 or HP>128
states. Remaining invalid high-31 pairs fail closed. Exhaustive
production codec counters prove all 1,935 independent HP/Widen/route
tuples (including all absent settings): no collisions and no clobber
of the Legato/MIX/LFO tail. Enumerated all 2,048 raw escape pairs,
leaving 114 invalid. Previous reader 265 checks: accepts old HP plus
Widen project and note lock, refuses new Sound/lock before writing
any byte of any persisted project object. No new primary marker,
flag, bank boundary or record byte.

Both owners; each physical one-step move to LFO1/LFO2/BOTH renders
4096/4096 affected frames on an authentic routed voice compared to
OFF, rules out alias by distinct PCM, holds through locks and
off/unlocked/stop rollback, no writing while busy, exact startup
and immutable factory fallback. Peer PCM and both phase/smoothed
wave controls and source parameter inputs remain bit-exact.
Untouched INIT and 694 factory programs retain the same 1,024-frame
PCM bytes, digest 2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f.
Retained depth uses unchanged previous host fixture; both owners'
14 Synth One and 15 SuperJ6 part-page directions remain reachable.

Replay with bash replay-host.sh EXACT_ALL_POST_SOURCE AUTHENTICATED_S1_PACK.
No root/source edits, remote Git, native, device or original DSP parity.
