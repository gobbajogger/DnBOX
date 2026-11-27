# Editor serialization and BREAK seek memory diagnostic

Two independently measured, opt-in source overlays. Neither is a release, an end-to-end sound match, or timing qualification. No installer, flash, hardware, or device image writes.

## Reproduction and exact donor

Rehydrate the guarded H16 recovery donor from the DnBOX-Guarded-H16-Replay-Inputs.tgz manifest or the retained half-attribution donor, using the pinned JieLi PI32v2 toolchain and guarded model link. Check the actual source against `break-base.sha256` before applying the BREAK overlay. Editor preimage SHA-256: `77313f96caf0d8e8d45949c8ff1c27c01a58a9ec834e20b58e388aaa7e26ef91`. Apply `editor-wire-fold.patch`, then `break-checkpoints.patch` with zero fuzz. This diagnostic predates the source-order/cue timing donor, so do not add this delta to a newer link without linking that exact composition.

## Evidence

Editor emits identical little-endian 7-bit fields through its existing writer. The extracted actual writer differential passed 420,000 length/value/buffer-end cases under UBSan. To repeat: `python3 verify-editor.py /exact/candidate/firmware/src/app/editor.c > verify.c && cc -std=c11 -O2 -Wall -Wextra -Werror -fsanitize=undefined verify.c -o verify && ./verify`. This tests field equivalence, not an independent end-to-end USB transport run.

BREAK splits ADPCM predictor and index arrays, dropping unused padding while preserving the checkpoint grid and publication. Actual PI32 objects each shrink 1,024 B in the live owner data and 1,024 B in the side bank data. Host signed paired BREAK comparison matched all 4,705,792 frames (loop, seek and adaptive cases separately exercised). No public preset/PCB/factory-sample format change. This saves RAM but not enough to fit a full matching Synth One history.

| Guarded native model links | Elite APP free | Pro APP free | pool usable after 8 KiB reserve, both |
| --- | ---: | ---: | ---: |
| Control | 24 B | 36 B | 564 B |
| Editor only | 168 B | 180 B | 564 B |
| Editor and BREAK | 120 B | 132 B | 2,612 B |

Final patches recover 96 B APP and 2,048 B usable pool relative to this control. The 512 B pool floor leaves 2,100 B; neither board yet passes the joint playback/sound release goal. Link receipts in `link-receipts.txt`; exact candidate timing and composition with the latest instrumented donor remain unverified.

Two further timing leaves were validated then rejected separately: 909 dual-send loop fusion saved 13.3 µs per eligible voice per 256 frames but added 310 B of native text; even impossible all-voice eligibility savings are under 0.3 ms. A bit-reservoir Huffman read matched 331,647 actual PI32 decoded samples; its savings at 128/256-frame batches were under half a microsecond per equivalent 256. Even an entirely free decoder would save at most about 0.35 ms in the tested 909 scope, not the multi-millisecond joint deficit. Do not rerun either as the primary fix.
