# Centered 909 workspace and usable pool recovery

Both board links use the exact fast centered-send helper verified against more than 1.64 million signed host frames. The 1,024-byte ordinary BSS scratch is eliminated rather than aliased; three separately-owned UI allocations consume the freed space, so 996 bytes become genuinely available to the audio pool. Both boards retain the hard 8,704-byte floor and the fixed XPOOL boundaries.

- Elite: 424 bytes free APP, 9,908 bytes free raw pool, 1,204 above the hard floor, 164 bytes ordinary BSS gap.
- Pro: 436 bytes free APP, same pool/boundary measurements.
- Isolated PI32: 2.69 to 10.24 µs faster per 256 frames depending on tested send state, with no signed differences. This is not a loaded deadline result.
- Actual serialized strict loaded 922 replay: Elite 14,455 µs with 62 late/dropouts; Pro 14,227 µs with 78. Both exceed 10,809 µs and are not release. Both retained all expected audible frames and no skipped cycles or virtual flash changes. Their exact applied event phase and waveform digest differed from the prior arm; do not infer a deterministic timing improvement or PCM regression solely from these two pairs.
- The center-only link used 336 extra bytes of APP, and the placements cost 128 more. The late-section placement candidate must be independently verified before assigning APP credit.

Never-install linked identities `82a456cb3d2f26deb609b7d24e8deb1fc523ed26d8f1f116377f51debab66ba0` and `3c10fef3f955a6c0130b37c2da93bd8386c5ded122481f95ceddc414e8ecb8e8`. Source/link archive SHA-256 `ffbe1662fb8d141f0ae7190f0ed133e03a2392cceb9a55562fe77389ce415ada`; strict replay source and results SHA-256 `93dc91c0cc86a0233653b79562b07ce299ac6892aa174ef0f2011827ae191641`. No release or hardware operation.
