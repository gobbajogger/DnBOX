# Loaded owner was not proof of a playing voice

The standard 922-event synthetic scene still exercises drums, Chicago, transport, the Synth One FX/render chain and five owner reads, but it does not exercise an active Synth One oscillator. Both boards' exact KR kernel admission count was 0 with 0 skipped in that scene, and the independent PC profile agrees. Do not call the previous 7/8 XRUN diagnostic a populated Synth One playback result. The test is still useful as a firmware regression fixture.

A guarded extension retains all 922 events and all owner queries, then uses standard USB MIDI channel 2 (the actual Bass A input) with explicit held note-on/off pairs. Added packets: 2250:913064, 2300:913764, 2450:913c64, 6610:813000, 6640:813700, 6680:813c00. Stopped RAM proves 307886 voiced renderer calls on each model; all happened to hit the exact silent Sub shortcut in this test. No device operations or production build.

| Forced A+B on worker, populated | Elite | Pro |
|---|---:|---:|
| worst render | 17620 µs | 17658 µs |
| XRUNs and late | 175 | 175 |
| skipped emulator cycles, virtual flash operations | 0 | 0 |
| Rendered frames / owner checks | 319488 / 5 | 319488 / 5 |

XIP 880 / 888 B; pool raw 8756 B. These are instrumented diagnostics, not perfectly matched application timing to the older binary. Do not claim the shortcut or layout wins or qualifies; it enabled admission verification. A separate A-only worker split on exactly this input is in progress.

## Timed path in the unvoiced standard fixture

Separate instrumented half #392 confirms three stretches (4/252/256 frames). On Elite, ordered worker job completion was at 953/6402/11750 µs, stretch output ready at 1019/6828/12230 µs, and elapsed half 12365 µs. On Pro the corresponding values were 973/6416/11770, 1039/6843/12246, and 12381 µs. Synth One work in this trace is FX/scene overhead, not voiced oscillator time. Ordered A wait in the 252-frame stretch was 409/456 µs. There is no safe way to add this wait saving to reclaimed arithmetic without measuring when the later prerequisite becomes ready.

Indirectly relocating two frequently called math leaves into the authorised RAM executable region was also tested after direct relocation failed due to the call range. The paired result was 12500/12 Elite and 12221/13 Pro and uses 184 B extra app XIP; no promotion.

Receipts (following rows contain hashes for trace JSON and PCM only; no audio is attached here):
```
85d5819eaf425f9b8deafe841e807397af335fcf7f6e95be0adecba1250d2985
b10c62064be7aae05e2d61d85b944411434c273c233cf6949c4766363b81eeb9
b3032fcc26af083f933cb5e5c0219ba0707a6265f8e24e68d70f524be3cdc57e
3d3b430d5cfb394b7cd7b8a78be9a3ad3145e68e22350feb89fa984989d26497
```
