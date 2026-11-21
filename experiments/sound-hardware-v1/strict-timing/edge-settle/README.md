# Panel startup on the first regular DMA release

This experiment starts the panel settle interval when the first regular DMA interrupt consumes the silent spacer. A one-half timeout also permits startup with a stopped audio clock. The underlying change keeps the first render on the original bank, acknowledges completion before rendering, preserves the active Chicago cost through rests, and reuses the two verified integer gain ramps.

| +1 ms physical scene | Elite | Pro |
| --- | ---: | ---: |
| Worst block, µs | 10,373 | 10,394 |
| Raw late / scene late / xruns | 0 / 0 / 0 | 0 / 0 / 0 |
| Completed halves | 648 | 648 |
| Changed final project bytes | 11 | 10 |
| First different stereo frame | 97,856 | 97,856 |
| Application bytes | 546,516 | 546,508 |
| XIP free bytes | 6,156 | 6,164 |
| Pool free bytes | 8,972 | 8,972 |

Both saved Clip boots matched stock through the first 30,208 stereo frames and all 36,420 project bytes. The first six wire halves contained 0, 0, 0, 10, 0 and 0 nonzero PCM16 words. Live input still diverged on both boards at +1 ms. That failure rules out production promotion and a passing six-offset result for this build.

Apply `firmware.patch` to `a305021382739c5472321ae8fb682b6d232cdb57`. Build stock and patched trees with the project AC79 1.2.14 SDK and JieLi 20260730.1 toolchain, using `BOARD=smk37 MODEL=elite sh build.sh` and `MODEL=pro` respectively for each tree. Extract [the original qualification driver](../direct-dma-startup.tar.gz) and run its documented `qualify.py` invocation with `--offsets 1` on each pair of matching model ELFs and packages. The complete same-offset comparison and binary identities are in [result.json](result.json).
