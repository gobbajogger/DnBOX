# Fixed-range 909 triangle lookup, host and PI32 fixed leaf
Input donor: table-share native exact source and generated table; checksums
in pins.sha256. Work does not alter the generated table or data layout.
For fixed triangle diode oscillator inputs the original fold limits abs phase
wave to 1; max absolute table coordinate is 256 on the 2.0x shared snare/tom
route (204.8 on the 1.6x kick). Adjacent index at most 257, all in the original
400-element float head. This scoped route omits repeated head/tail search and
uses original tap loads and original non-FMA interpolation. Other distortion
types and table bodies remain untouched; incompatible table layouts fall back.

Copy pinned donor drum909.c to base.c and candidate.c and apply pin-head.py to
candidate only. For standalone copies point the ../x0x_cold.h include at
x0x_cold.h, resolved by the source include path. Run host.sh for the 56,000
full kit state/four bus test and native.sh for the same fold/lookup leaf at
360 MHz. Leaf covers wrap/signed corner guards, not whole-kit or heavy-scene
timing. No generated table, binaries or audio in source deliverable.
