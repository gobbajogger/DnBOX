# Guarded two-range tanh diagnostic, paired maps only

Start from tested receiver-final-join. Firmware/generator patch guarded-tanh-split.patch applies inside the source tree. Placement patch guarded-tanh-placement.patch applies to the enclosing link-guarded-exact.py runner directory. In an isolated copy use refresh-generated.py (or normal generator), then DNBOX_TANH_SPLIT_LAB=1 bash run.sh. Add DNBOX_TANH_REFILL_LAB=1 for the measured refill arm. The production/default raw table path and all deployment guards remain unchanged.

Both preexisting native generated tables matched all 1,026 canonical binary32 words. Head [0..399] is 1,600 B SHA 9cce98ce…; tail [400..1025] is 1,252 B SHA 6f4f3d42…; every word exactly reconstituted as 0x3f7f0000 | suffix. Adjacent-pair decoding has independent head, mixed, and tail branches; the same interpolation operation is unchanged. No device timing claim; the extra hot branch remains a latency risk.

| Actual guarded map | Elite XIP over | Pro XIP over | FDATA used |
|---|---:|---:|---:|
| Exact combined baseline | 11,500 | 11,488 | 0xef90 |
| Split only | 12,092 | 12,080 | 0xeaac |
| Split + immutable LCD refill | 10,876 | 10,864 | 0xef6c |

Split recovered 1,252 B of FDATA at a 592 B APP cost. Refilling with 1,216 B of existing arc-turn, arc-pixel, palette and corner draw data gives 624 B net APP savings on both. New-pins.json independently pins the native values using the actual JieLi object ABI. They only serve normal UI; boot and read-only recovery use plain graphics/font paths and their fallback font remains resident. Existing FDATA, Slot3/FITTAB layouts, table hash checks, XIP cap and RAM guard unchanged. BSS still 172 B free and POOL used 0x51e54 (+428 over reserve). Because all links reject hard XIP, same-image sealing/placement hash replay and native bit/timing proof remain unrun. No binary package, device writes, runtime, or Git update.
