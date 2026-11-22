# Newest pitch-LFO stock build and bounded replay

The exact source composition merges Sound `ad86777e`, whose production source is `630c4d96`, into the proven stock WIDE and native Remote tree. The local merge before the additional compiler option is `0ae710bea34d553ec2eb05eb83c7e5f70401ce6a`. The Integration size profile now compiles 808, 909, and BREAK with `-Oz`. The 808 change recovers a measured 2,352 B per model and changes no DSP source.

| Stock model | XIP before 808 change | XIP free now | Pool margin | Eight-second peak | Late halves / xruns |
|---|---:|---:|---:|---:|---:|
| Elite | 216 B | 2,568 B | 732 B | 7,822 µs | 0 / 0 |
| Pro | 228 B | 2,580 B | 732 B | 8,222 µs | 0 / 0 |

Both standard package and flashmap checks passed. Both exact packages passed the native emulated USB one-scalar Apply/Undo exchange, with stale post-CLOSE writes refused. The populated calibrated run exercised both drum kits, saved Synth One highpass and full sub, legacy SuperJ6, and a CRC-valid BREAK loop. Audio capture checks and both raw and scene late counters passed. Guest RAM verified the configured mode and owner state.

The untraced compact-program pass remains applicable because all ten inputs match. The previously independently approved branch-only Nightly artifact remains paired to the earlier `54266973` firmware package hashes. This packet does not transfer that review to these new packages. Active WIDE wet-path deadline and the final combined and strict ten-minute gates remain open.
