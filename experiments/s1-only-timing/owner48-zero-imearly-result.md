# Overlap BREAK rendering and accelerate the 909 decoder without changing sound

Source-only strict companion to [the three-tom diagnostic](owner48-zero-result.md).
Precisely the same scene: both 132-hit kits, six held Synth One voices,
128 held buffers; no controller, clock, waveform, DMA, or deadline change.
`owner48-zero-compose.sh` then apply `owner48-zero-ima-transitions.patch`
and `owner48-zero-break-early.patch`.

The 909's compressed data are unchanged. At initialization it creates a
5,696 B RAM table of IMA predictor deltas and next indices. The hot loop
does a single lookup and the original predictor clamp. The table adds no
sample transfer or storage requirement. The exhaustive host comparison
checked 93,323,264 combinations of nibble, decoder index and predictor;
every output and next state matched.

Separately, render BREAK before the synth waits for the drum mix, only
where the existing reverse-core lab rule places BREAK on Core 0. Its
mix and MIDI drain still occur after all the other parts in the original
order. The sole scheduling change is when its separate DSP work runs.

| Strict emulator, instrumented only for separate audio comparison | Elite | Pro |
| --- | ---: | ---: |
| Dropouts (previous paired checkpoint → this) | **10 → 8** | **10 → 8** |
| Held halves over physical 11,609.98 µs | **9 → 7** | **9 → 7** |
| Worst held | **11,720.25 µs** | **11,720.38 µs** |
| Held halves over the 10,809 µs target | **23/128** | **23/128** |
| Free app flash | **1,108 B** | **1,116 B** |
| Audio-pool margin beyond unchanged 8,192 B reserve | **37,620 B** | **37,620 B** |

[Elite](owner48-zero-imearly-elite.json) and [Pro](owner48-zero-imearly-pro.json)
are uninstrumented timing. [Elite fingerprints](owner48-zero-imearly-fp-elite.txt)
and [Pro fingerprints](owner48-zero-imearly-fp-pro.txt) each show
**0/160 different pre-DMA blocks** versus 402bd0e4. There were zero
skipped emulator cycles and no oversized/raw fallback draws. The 48-row
display allocation and its 10,080 B saving are unchanged.
The worst held block is still about **110 µs** late on both; this is
not release qualification, a new normal scheduler or evidence for wet playback.

The table alone illustrates why the paired replay matters: in a separate
instrumented comparison its 909 cost fell from 5,194 to 5,020 µs during
frame 129,024, but uninstrumented peak only improved by 7.29 µs with
unchanged dropouts. Early BREAK alone also stayed at 10. Combined, the
two changes made a difference. The other individual variants have their
own receipts; do not add independent timing deltas or ship the small/XIP
or traced arms as release firmware.
