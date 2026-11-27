# Native Synth One prefix deadline and shared curve arithmetic

Both lanes below are guarded, disposable, and not release claims.

## Prefix scheduling

Actual populated A=Synth One, B=Chicago replay on both sealed models. Only fully joined same-span rows contribute (943 Elite / 979 Pro). Latest completed row, relative to exact producer offer:

| Model | Span | First dependent dry read | 808 finished, before Chicago | Core 0 free after Chicago |
| --- | ---: | ---: | ---: | ---: |
| Elite | 242 frames | +6.542 µs | +2780.333 µs | +2994.708 µs |
| Pro | 186 frames | +26.917 µs | +2315.417 µs | +2515.583 µs |

The first-tile approach cannot use either helper opportunity in these witnessed blocks. No late in-place write allowed. The 744 B private producer state (3×168 B +240 B) can be represented without increasing the 5120 B existing A workspace under A-only S1/matching kind; 808 and B deliberately select the other workspace. Actor generation and ACK required before repurposing. Later tiles within the same source event-bounded span are a separate unknown, not an observed gain. Witness builds still failed real replay (14386 µs/74 XRUN Elite, 14470 µs/70 Pro), and no virtual flash changed. Hashes `de2b4110f568e07e76ec02ceed38101629ae571dba028cd88b92ead43bc0dce0` and `404e2121710a2b56cef1599287fd6f0da343fc2b31825c69c870a869fc1cc4fe`.

## Shared exponent ROM

The native patch `be9037d8395f8a047b6e2967d0595bea2a429825` shares the resident two-word SuperJ6 evaluator with 19 fixed device detent curves. The old waveform-hit hot constant is replaced with the exact binary32 word and is not routed through the new lookup. Host and standard-clock bare native tested all 2432 words exact, expected raw SHA256 `c8c351223b0c805e68f88c917c5ed0a8b4983277681e2a1b43251627ffbd3faf`.

Both guarded joint-component images gain 1080 B APP free with unchanged pool. New Elite/Pro 1468/1480 B free, pool 932 usable (420 over guard). ELF `3c41147f367e1bb111adcccc0c14335bc97d1d566602a798d43e887d95624f84` / `b541b08897be959b30a51407d2e11ece4b955b0df3c58722bb46179eb211e04a`. In strict standalone 360 MHz target, old average 0.324 µs and max 6.58, new average 2.494 and max 22.46 (includes first-row cache/setup). Some remaining lookups run at automation/ISR ingress. Dynamic audio/control stress not tested; not an adoption claim. The generated-header change must also be regenerated in cached lab build flows that bypass normal generation.
