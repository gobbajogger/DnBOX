# Synth One: start before the ordered 808 mix, then borrow the retired peer

Experiment only. Nothing here changes a product default, installs firmware, or clears the timing gate. The pinned checkout is the Clip/delay source e00f689d; the neighboring joined receipt is the preceding f191f9fc experiment. Exact Elite/Pro target linker, fixture, controller, NOR and strict 360 MHz emulator; six held Synth One B notes after sequencer start, quiet Chicago A, both kits. The input hash manifests identify each run. Audio repeats are nonzero PCM repeated at a 1024-frame lag for at least 512 frames.

| Trial | Model | Max µs | Last µs | Completed halves | Late parity / stale spans | XIP free B |
|---|---|---:|---:|---:|---:|---:|
| Previously joined/optimized kits, dynamic plan (last mask 2) | Pro | 18329 | 17182 | 324 | 67 / 66 | 5060 |
| Both kits on peer, control; wait before B | Pro | 22643 | 20640 | 323 | 68 / 67 | 5540 |
| Both kits on peer; render B early, ordered bus publish | Pro | 19903 | 17147 | 340 | 50 / 49 | 5348 |
| Synth One on peer, 808 body shares peer | Pro | 19286 | 16939 | 337 | 54 / 53 | 5268 |
| Synth One on peer, no 808 split | Pro | 18388 | 16446 | 336 | 55 / 54 | 5268 |
| Early B plus fixed-point smoother elision | Pro | 18179 | 14955 | 340 | 50 / 49 | 5204 |
| Early B plus smoother elision and late peer claim | Pro | **16047** | **13906** | 340 | 50 / 49 | 4980 |
| Same lazy composition | Elite | **16234** | **14767** | 340 | 50 / 49 | 4972 |
| Optional oscillator phase cache added to lazy | Pro | 16058 | 14535 | 340 | 50 / 49 | 4804 |

All linked with 27,392 B of audio pool margin above the unchanged 8,192 B reserve. Stack watermarks on the control/early builds were 2084 of 4352 bytes on core 1; whole-Synth-One-on-peer trials used 1480 B in this fixture. Those are not general stack qualification. The oscillator cache does not improve this gate and is not selected. It changes only derived voice storage; the host comparison projects out both copies of the six added fields. Its optional receipt uses a 192 B voice stride rather than 168 B, without changing the six root/gate offsets relative to each voice.

The both-kits map has no split-808 metal scratch. B's private DSP renders early only on that owned map and only for active Synth One B with no arp engagement, pending arp tail, or stale owner expiry. B still publishes, meters, and mixes after A. A's work and B's parent engine state are advanced in their former render order. For lazy admission, the next eligible 8-frame boundary borrows the existing typed worker workspace only after the drums have published their ordered commits, retired their stages, and acknowledged the mailbox. No wait on a live drum mailbox. No change to effect math or mixer sums.

Host `check-lazy.sh STOCK PATCHED` requires the unmodified sources and factory assets for STOCK, and the assembled lazy trial source for PATCHED. It verifies 694 factory programs plus INIT, 8,573,419 exact stereo frames and the full engine and arena state. The mock denies some claims, admits only after waiting through multiple polls, and interrupts some admitted worker jobs after an eight-frame tile. That is a library DSP/fallback proof, not an end-to-end native PCM equality claim for a run with underruns. Active arp continues through the stock runtime path; this does not qualify its heavy timing.

The trace build is separately instrumented, not the throughput build. In its worst half (ordinal 304, 3 subspans, both kits on peer) it measured 20057 µs end-to-end. Excluding ordered stage waits, the main core spent 16928 µs for Synth One B, 770 µs for A and 37 µs for BREAK; the peer spent 4871 µs for the 909 and 4984 µs for the 808. Main/peer stage waits were about 1/8 µs, respectively. The trace captures the same half, not per-task smoothed estimates. Do not confuse mixing-order waits with the remaining serial and concurrent compute costs.

Reproduce builds with `bash build-overlap.sh /path/to/fresh-e00 pro lazy` (or elite). The variants map to the rows above. Use the higher-level replay and seed scripts in the parent directory and the same fixture (`SYNTH_OWNER=B SYNTH_KIT=2` via build-project.sh). The test-scene USB chord uses channel 3. Timing/profile binaries are deliberately not installer assets. No variant is at zero dropouts in the heavy scene.
