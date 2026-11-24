# Exact dual-group cold native smoke

One local calibrated native emulator process at a time, exactly three runs. Disposable flash files only, built from the archived verified 73749f55 model boot shims and native joined dual-group lab ELF load bytes. 360 MHz strict runner with no skipped cycles; whole runner, both ELF hashes, independent primary/secondary seal headers, font and S1PK hashes checked. No production packager or install format, no physical device, no product-map edits. The synthetic secondary write at FITTAB occurs only in the local seed, and the primary remains the known forbidden-on-device overlap.

| Exact model/case | LCD outcome | Audio halves | Worst cold render | Late/XRUN | Sample scanner |
|---|---|---:|---:|---:|---:|
| Elite matching primary+secondary, 3,500 ms | Readable 909 SEQ, play active | 236 | 4,019 us | 0 / 0 | 1 seeded zone |
| Pro matching primary+secondary, 3,500 ms | Same readable 909 SEQ | 236 | 3,917 us | 0 / 0 | 1 seeded zone |
| Elite second-group-only body bit corruption, 2,400 ms | Native readable DnBOX SAFE MODE / COMPANION DATA INVALID | 0 | 0 | 0 / 0 | 0 zones |

Matching builds exercised physical-edit and SEQ PLAY stimulus with notes, each produced 483,328 B stereo PCM, byte-identical to each other. Guard zero/active scope on both. Negative case keeps primary and both expected headers intact, changes one secondary payload bit, verifies CRC divergence. Guard one, zero render/scope/PCM and no sample scan; entire 1 MiB synthetic flash file unchanged. The rejected LCD's original PPM is exactly the verified readable prior safe screen. Contact sheet order Elite, Pro, secondary negative. All post-run protected extents, both companion regions, synthetic sample bytes, original S1PK and XDATA were checked unchanged; routine expected application-state writes may occur on normal boot. No actual user sample was used. Exact runner and seed/artifact digests in the source and numeric receipts.

This is **cold startup and light audio smoke only**, not strict heavy timing/headroom, an original-engine A/B, focused knob/arp native behavior, Remote qualification, user-data recovery, updater certification or a releasable build. Parent's native slot was released when the third process ended. Reproduce only with explicit native slot ownership by invoking seed-dual-smoke.py WORKSPACE EXACT_SEALED_LINK_ROOT UNIQUE_TMP_SCRATCH --run; without --run this just creates disposable pinned seeds. Source plus minimal text and LCD screenshot retained, no raw flash, firmware binaries or PCM persisted here.
