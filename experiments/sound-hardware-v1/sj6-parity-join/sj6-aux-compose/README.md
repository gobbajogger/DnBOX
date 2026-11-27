# SuperJ6 independent Sub and Noise buses

This is a guarded dry **host reference** joining the original source receivers to
the actual DnBOX `original_runtime.c:render_backend`, not a product sampler or an
installed-Apple graph capture. Reuses the signed stereo inlet source patch in
`sj6-runtime-source-mix/runtime.patch`, SHA256
`88a1bfae302e64a9849c1f2bdd1612db33f40adde33e3fa4205e2d32053cc0da`.
No further runtime source change was needed: all four roles genuinely supply
their own post-voice stereo arrays, so this is an input/provider test patch.

Run from workspace root with exact privately available IPA/current images and
authorized samples:

    sh sj6-aux-compose/build.sh ipa
    sh sj6-aux-compose/build.sh current

The harness validates the IPA sample pack and SFZ hashes plus each payload CRC,
builds the original 12-region receiver keymap, and sends pinned previously
reached receiver words with actual ARM64 sampler functions. The first accepted
warm note is physical key 36 velocity 86 (16,384 warm, 22,050 held, 8,192 sampled
release, 128-frame callback and exact shortened gate callback). This test does
not claim the later chord and full Swift host scheduling are reproduced. Source
samples and PCM stay in RAM; JSON contains summaries and digests only. Sub
receives no forced phase, retime or fitted gain. Noise is truly muted in the
authored controls; its positive arm changes only the receiver's physical
fader 0 to .25. The emulator substitutes deterministic entropy for imported
Apple RNG, so that positive does not validate installed Native RNG bytes.

Tested both versions: all 362,904 words each matched the explicit stereo
model (full, Sub-only, Noise-only for both fader arms), through the real backend
with factory Chorus off and unchanged unity master. Both versions produced the
same signed Sub and Noise hashes in these controls. The source-model mixer trim
is the reached Brainwave .8670601845; negotiated Audio Engine pan/routing,
active FX, hardware and whole-preset sound remain unqualified.
