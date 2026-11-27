# Tom and held-REC ARP lab qualification

Runtime changes: c3e637cf1380ce0ea8baa30400ea820512308e7c. Later branch commits changed tests and receipts only. No release config, device writes, or installer promotion.

- Six phase offsets per model through the 922-event wet native emulator scene at 360 MHz, 619 audio halves each. All 12 runs: no skipped cycles, late audio halves, dropouts or virtual NOR writes. Worst Elite 10,601 us; worst Pro 10,658 us, 151 us under the 10,809 us threshold.
- 17,216 B net shared RAM reclaimed versus the original canvas; each board has 21,832 B shared pool after the 8,192 B reserve. XIP headroom 852 B Elite and 864 B Pro.
- Ordinary unmodified packages from the six-flag lab build (the replay separately adds its pinned controller): Elite d050700cafc986fdebddebdd169cc06cb05f991f492b98ae54f1e144fdedfa03; Pro 07e0f424393ecd17d9bf1f3c5a19f248e80943cfd2291ba48b6bffa59d2f2725. ELF hashes Pro ee2f71efae239fc251316dcf616e434da353cd2c6e656b358f752c595c207013, Elite 3d8cf229eeed76640d0f08aff5a4633353f2e4a942be88e5d98cca5f27469f2f.
- Both exact table sets match at boot and through every phase; 528 private cymbal adoptions and zero fallbacks per run.
- Owner-provided SuperJ6 bank export: all 640 programs in 12 banks, all 90,549 fields mapped, no non-finite values. Separate filled-in ARP/SEQ host fixture: 640 programs decoded, 110 ARP configurations, 122 sequence configurations, 2,552 emitted notes in reference order, all 640 secondary envelopes recognized. Bank data is not committed or added to app flash. This verifies bank data and note sequencing, not the original app's sound.
- Held Stop integration: 337,756 focused checks passed in normal and ASan/UBSan diagnostic modes with matching rendered image sets; leak checking was disabled only in that focused rerun due this runner's ptrace incompatibility. The broader gate remains unqualified.
