# Original Synth One Reverb Mix LFO (ID51), Init-only

Apply five app-only source files to the exact source-mod-wheels root,
MOD-WHEELS-POST SHA b71f366f8da6ed85a64dbd3ba90b4c2784503511a01558c81ead2ee53e037e3d.
All manifest entries verified. No DSP, preset, PSRAM, native, package, Git
or device changes. Existing FX SWITCHES K3 uses the currently empty slot.

Original pinned S1DSPKernel.hpp:407 ID51 four states OFF/LFO1/LFO2/BOTH,
default OFF; S1DSPKernel+process.mm:282–292 multiplies reverb mix by the
separate enable and selected descending LFO or their arithmetic mean.
Compact already applies the same scalar at the same wet integration
stage, not a claim of identical reverb topology/tone.

Transparent v1 encodings: existing short/long typed records and packed
0x00 / 0xf8..0xff headers remain unchanged for old states. New 0xd8..0xf7
high-byte range was provably impossible in the old long LFO selector:
minimum 1,703,911 at old header group 27 > maximum valid 1,642,799
(148*148*75 states). It does not overlap the 0xf8 packed family. Full
8,192 header ordinals: 7,740 valid (4 explicit ID51 ×5 cutoff-route
states ×3 Widen ×129 Reverb HP), 452 invalid. Retains fully independent
original fields with a round-trip writer and preserves all remaining
packed body layout, exact size 15 and existing zero-claim guard for
opaque tail bytes. No ARP/wheel or reserve row claim. Absent uses original
exact authored target; explicit OFF is a valid independently saved edit.

Host 164,306 checks: exhaustive canonical header coverage including
all invalid codes, zero opaque writes, both owner actual FX SWITCHES K3
routing, original per-sample wet law through real core, all modes audible
with selected mix, reverb OFF is exact null, peer PCM exact. Both owner
saved parameter round-trip and genuine note-lock application/hold/release,
plus existing wheel and FX/Pitch/Tremolo/FM/Decay/level/depth bits
unaffected. Exact prior reader refuses all 8,192 header values and two
actually serialized projects plus subsequent saves (8,296 checks).
Untouched 4,096 stereo Init frames bit-exact, host original_bass aggregate
0x2b20 unchanged. No native footprint/timing or complete original FX claim.

Reproduce: sh replay.sh /exact/source-mod-wheels after the pinned build headers are generated. The two test runs use temporary isolated sources. See verified.log for the expected outcomes.
