# Q24 all-pass history bare PI32v2 timing

Exact root source commit cf85551d7c1c72d397264ec7a8f5b6112cfefe35; SHA checks in run.sh. The only harness-portability changes strip the unused host export, route both arms through the same firmware-derived tanh wrapper, and explicitly request the same four-byte memcpy semantics as a compiler intrinsic on the packed arm (the freestanding harness disables standard builtins).

Both arms use the same pinned r3 PI32v2 compiler flags (-Os, -ffp-contract=off), host-proven separated arithmetic, same deterministic input seed 0x71c5adf3, 512 frames/fresh zero state, two inner clocks per frame, identical coefficients/settings. One run per arm, shared PRNG/checksum included. TIMER4 ticks at 24 MHz and same strict no-skip, non-DMA emulation. No extrapolation to complete firmware or sound.

* 52 B Float32 reference: 49,775 ticks, output hash 08e73264; the digest matches the earlier independent no-FMA fixture.
* 44 B Q24: 55,225 ticks, output hash 7843f0f9. +5,450 ticks (+10.95%; +0.444 us per tested frame).
* Identical linked clock_2x bodies by size (210 B); measured caller text 424 -> 758 B (+334 B). This is the bare measured symbol footprint, not product APP/XIP or per-owner state cost.
* Neither leaf references external FMA or a memcpy runtime helper. Distinct output hashes are expected and do not establish authentic sound tolerance.

Replay from workspace: bash sj6-ota-bare-q24/run.sh. No full firmware linker, physical device or publication used.

## Factored Q24 with one shared all-pass helper
Replay with `Q24_NOINLINE=1 bash sj6-ota-bare-q24/run.sh`. The sole source delta is `ap_q24` noinline (still inlines the pack/unpack); no other operations or compiler flags change.

* Exact same Q24 output hash 7843f0f9. Comparator repeats at 49,775 ticks and hash 08e73264.
* Q24 helper 108 B + Q24 caller 280 B = 388 B, saving 370 B versus inline Q24 (758 B) and 36 B versus the 52 B comparator (424 B). Both clock_2x bodies stay 210 B.
* Factored Q24 58,296 ticks, up 3,071 / 5.56% versus inline Q24 (+0.250 µs/test frame) and up 8,521 / 17.12% versus 52 B (+0.693 µs/test frame).
