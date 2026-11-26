# Current ARM64 source receipt, authorized original comparison only

The original sample path used actual code at binary SHA 6d163d79…2850.
The eight known taps on the sender/receiver path and the compared data:

| Value | PC (within current image) | Register |
| --- | --- | --- |
| AMP | 0x3b6238 | s11 |
| VoiceGain | 0x3b6238 | s0 |
| Product | 0x3b623c | s15 |
| Halfband output | 0x3b6d94 | s0 |
| Applied gain | 0x3b6d94 | s1 |
| Scaled output | 0x3b6d98 | s0 |
| Normal filter input | 0x3b78e0 | s0 |
| Normal filter output | 0x3b7934 | s1 |

Use the held-cue fixture and pinned current transition definitions in
`../sj6-osc2-source/transition.py`; consume frames 16384–18431 and
24576–37599 from `render(True,4,211.95989990234375)`. Float32 word checks:
AMP times VoiceGain equals the applied word; halfband output times that
word equals pre-Normal input; Normal output equals returned signed
samples. Onset signs: 698 positive / 1350 negative. Held signs: 6415
positive / 6641 negative. The variant with the extra .34 multiplier
matches none of these 15104 words.

A separate legacy branch multiplies by .5 then .68 at 0x3a7af4 and
0x3a7b00, behind a different voice-state check at 0x3a7ae4. Retain that
path. The exact current generated-path product precedes the original
stored s15 gain at 0x3b67f0 and post-halfband multiply.
