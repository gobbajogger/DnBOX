# Native 909 tom work and exact host body-only variant
Read-only PI32 TPROF 17 ms windows on same v10 ELF/scene. Toms account
for 188527 of 426482 inlined 909 core0 weights (44.2%) Elite, 175133 of
415440 (42.2%) Pro. Sampled CH/OH/CR/RD share one 88637/88709-weight PC
loop. Time-weighted PCs are not per-voice exclusive cycle counts. Exact
PI32 function boundaries: d9_render to 0x2074642 noise, BD to 0x2074b26,
SD to 0x2075030, tom-group to 0x2075776, rim/clap dispatch to 0x2075c28,
sampled to 0x2076200. TPROF agrees in full PCM with unprofiled exact Elite;
Pro TPROF slightly moves trajectory and PCM. Not a timing pass.

Body-only variant admits exactly-zero stick and upper partial countdowns,
and pitch/body strictly outlasting span, preserving original Float32 signed
zero/order, all phase/state and final cur. Original stale/warm timeout outside
branch unchanged. Host 56000 complete-state/four-bus comparisons including
PCM and deliberate attack/timeout/boundary states exact in normal and
ASan+UBSan. Actual paired links consume +544 B app, +388/+396 B still
free, both +564 pool, unchanged no-PSRAM/no-write guards.

| Board | Source donor / late | Body variant / late |
|---|---:|---:|
| Elite | 13168 /44 | 12941 /54 |
| Pro | 13046 /60 | 12815 /41 |

All 319488 PCM frames, no skip/mailbox errors, identical disposable flash.
Elite donor got only 3 settled status reads and variant 4; Pro both get 4.
Neither pair matches full captured PCM. Earlier candidate onset is first
difference: frame 71232 versus 79424 Elite (16×512), and 66624 versus
70208 Pro (7×512). First role status at Elite 2359 vs 2485ms and Pro
2254 vs 2336ms; these are USB reply times, not confirmed first effective
note times. Exact immutable replay reports total but not first-late
time. No PCM shifts, no separately proved DSP error, no adoption.
