# Experimental nine-bit Synth One room build

The optional source adds seven Synth One reverb controls to the normal selector, recorder and project format. It targets SMK-37 Elite and Pro. Every selected owner has 1,548 nineteen-byte RevSC packets. Each owner borrows 216 packets from its own retired Chicago insert, starting after its 4,096-byte stereo-width area. The default build keeps the qualified Chicago Hall and BPM delay.

Apply `normal-nine-bit.patch` in a disposable checkout of `d842bba4da1aa50177803011e02ca10f4e2e2b43`. The patch SHA-256 is `32d2cfa354de13f04c5b396a1b8645e46c9b0084a91b82a2932585cbf6a8fe6a`. All 18 existing inputs matched their Git blob IDs. An independent unified-diff applicator verified exact hunk offsets, lengths, context and removed lines, then reproduced every byte and all 25 target blob IDs. The other seven paths are new files.

```sh
git apply --check normal-nine-bit.patch
git apply normal-nine-bit.patch
python3 verify-normal-profile.py . roundtrip.json
BOARD=smk37 MODEL=elite X0X_DEV_PADCAL=1 X0X_SYNTHONE_ROOM=1 sh build.sh
BOARD=smk37 MODEL=pro X0X_DEV_PADCAL=1 X0X_SYNTHONE_ROOM=1 sh build.sh
```

The Elite ELF is `build/smk37-elite/x0x.elf`, the Pro ELF is `build/smk37/x0x.elf`, and the packages are `build/x0x-smk37-elite.fwsc` and `build/x0x-smk37.fwsc`. Save these products separately before building with `X0X_SYNTHONE_ROOM=0`. Compare both disabled ELF/package pairs against the stock `d842bba4` receipts in the Chicago factory-insert evidence. The pre-existing Elite `e3ffb3d8` disabled build was reproduced byte-for-byte. Its ELF SHA-256 was `284c38d383dc2f2f53aeefe5a4d8c09bf963c43f3826f1c0a95bbde8dca06b5a` and its package SHA-256 was `c7f2627d37c7f45b50c86ff64bbafa64a813ae3d53efc90094d5b4c127c6802c`.

For native qualification, stage the selected-native populated projects and replay the 922-event, eight-second scene on each exact ELF/package. Exercise the ordinary Chicago/Miami and Juno/Synth One pairs, followed by one and two active rooms. The wet project fixture saved all seven factory #459 room controls on the normal INIT dry instrument. Verify audio duration, fault records, peak callback below 11,609 µs, zero raw late and zero xruns. Read the two selected owners after the dual-room test. Each must have its own disjoint history, eight advancing lines, and nonzero authored/mix/on state. Also switch each owner independently between Chicago full Hall/BPM and Synth One, checking the other owner's stereo-width prefix, encoded room history and output across re-selection.

**Still pending on this exact source** are both native links, link-map bounds, default ELF/package equality on `d842bba4`, the four standard timing gates, active-room timing, optional-controller startup and the mixed-owner selection test. The workspace filled before these could run. Earlier corrected nine-bit firmware on `bf48ce83` passed all four standard gates. One active room ran at 14,383/14,295 µs on Elite/Pro and two at 16,498/17,226 µs. The newer fast scalar room reproduced all four original-control output words throughout 91,816,200 frames across 694 programs and lowered the isolated 256-frame peak from 2,893.17 to 2,472.54 µs. Those measurements use separately identified builds. Physical hardware timing and cache behavior remain unmeasured.
