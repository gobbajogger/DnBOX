# Combined engines: native diagnostics, not qualification

Conditions: disposable PI32v2 native emulation, strict emulator, no device or persisted flash writes. Requested heavy-scene target: at most 10,809 µs and zero XRUNs, lateness and skips. Both engines use current diagnostic INIT storage. No perfect-preset or release-firmware claim.

| Branch / same joint replay | Elite peak / XRUN | Pro peak / XRUN |
|---|---:|---:|
| Plan 4: Synth One on Core 1; SuperJ6 and 909 on Core 0 | 24,563 µs / 182 | 24,582 µs / 181 |
| Plan 5 adds 909 to Core 1, rejected | 23,818 µs / 222 | 20,554 µs / 239 |

Plan 5 fits, but more XRUNs means the lower peak does not qualify it. Native request timing and end-of-run note gates differed slightly despite equal replay inputs. Do not treat plan 5 as a PCM comparison.

A paired instrumented plan-4 overlap witness had mean measured role work 24.021 ms Elite and 22.600 ms Pro, both above 21.618 ms across two cores before mastering/orchestration. Measured inter-core waits were small. Splitting the SuperJ6 group on a separate Elite witness gave 5.721 ms in the stereo synthesizer and 0.540 ms downstream, about 91% in synthesis. That split's Pro replay did not retain jointly audible spans; its values don't qualify a cross-board claim. Keep physical event activation and linked owner checks in the next paired run.

Six-key Elite Synth One diagnostic: before, 4,190 offers completed and 0 were accepted. The ready-ACK correction allowed 3,980 computed/accepted results, preserving producer bounds and making no new waits. Peak was 21,500 µs with 237 XRUNs, versus 21,817 µs and 237 with worker disabled. It fixes the handoff, not playback; no default admission.

## Small exact SuperJ6 cost reduction

In the dry INIT scope legacy chorus computes a delay tap with wet mix zero. Simply turning it off would lose modulation phase and history for the transition back to active mix. The [diagnostic patch](../../experiments/sound-hardware-v1/sj6-legacy-zero-mix-diagnostic.patch) skips only the unused tap for nonzero signed stereo inputs. The same history and phase update always happens. Actual zeros retain baseline arithmetic for their signed-zero semantics, and active chorus uses the original path.

- Host sanitizer differential: 3,174,030 stereo frames and full chorus state after every span exact, including wet/state transitions and signed-zero cases.
- Bare target differential: 32,768 frames and complete state exact over four conditions. Mean per 128 dry nonzero frames, 100.15 → 28.82 µs; active mix 99.91 → 103.36 µs; all-zero dry 97.78 → 102.22 µs. These are isolated leaves, not measured underrun improvement.
- Guarded linked pair: 244 B Elite and 256 B Pro free APP, 908 B usable pool both. Change costs 48 B APP versus its donor. It cannot close the multi-ms stereo voice-render gap by itself and is not default-on.

### Additional small leaf measurements

- The [909 unity cursor candidate](../../experiments/sound-hardware-v1/909-unity-cursor-local-diagnostic.patch) holds serial samples, Huffman cursor and envelope state in locals until the end of a native-rate span. Existing non-unity and final-sample admission remain. Hosted sanitizers and a strict PI32 differential agreed on four sampler voices: 159,744 stereo frames per target run and matching decoder, envelope, scratch and four bus arrays. Isolated timed slices improved 7.54–7.58% across CH/OH/CR/RD, roughly 5.4 µs per 128 frames per voice. Measured +46 B in the unlinked target object; a combined footprint is not claimed. No adoption just to post a percentage.
- Hoisting invariant SuperJ6 pan, gain and LFO step arithmetic reproduced 1,048,896 stereo host frames and complete state; 49,152 native stereo frames and state also matched. It costs 48 B APP in both diagnostic links and saves only about 10.6 µs per 128 isolated stereo samples for six voices. This is too small a trade while executable headroom is limited; leave it out of the current candidate.


### Combined dry chorus + 909 cursor check

I combined just these two separately exact component changes on the current source-order donor, with Plan 4 and all original owner/memory guards. Actual native paired link: 228 B Elite / 240 B Pro free APP (64 B combined versus donor 292/304); pool unchanged at 908 B after 8 KiB reserve, or 396 B after the final 512 B floor. Same project bytes, five exact settled owner responses on both, no skipped cycles, no virtual NOR modification, stable Core 1 stack paint and no bad mailbox.

Elite reported 13,228 µs / 67 XRUNs and Pro 20,417 µs / 185. Elite's terminal voice state had no evidence of triggered SuperJ6 slots; Pro retained a gated SuperJ6 note. The valid owner responses alone are **not** proof of equal active-note work. Do not compare these peak/XRUN numbers as an improvement in the joint workload. The real integrated candidate remains unqualified pending a replay with measured per-render joint activity. No shipping changes.


### Recovered app flash and independently verified joint activity

Sharing byte-identical cold compressor ratios and SuperJ6 labels recovered 60 B APP on both links (228→288 Elite; 240→300 Pro), with every value and the RAM/companion geometry unchanged. [Exact source change](../../experiments/sound-hardware-v1/cold-linked-duplicates-diagnostic.patch).

The historical USB owner checks were not enough. An independent early DIN variant retained the same project bytes and three correct owner responses at 4.6 seconds, but neither board retained any SuperJ6 voice or Synth One root at that held checkpoint, so that input path did not qualify a joint speed test.

A separate guarded [engine-entry cue and activity observer](../../experiments/sound-hardware-v1/joint-fixed-render-cue-diagnostic.patch) prepares the same project and calls the actual note helper when both typed owners are ready. Both acknowledged the seeded notes. The observer counts successful render spans in which **both** separate pre-mix signals exceed the existing audibility threshold somewhere in the span. This is not claimed same-sample overlap, MIDI ingress qualification, or a transparent timing probe.

| Strict instrumented seeded run | Elite | Pro |
|---|---:|---:|
| Joint-audible render spans | 312 | 354 |
| Worst time | 20,233 µs | 20,359 µs |
| XRUNs | 205 | 209 |
| Skipped cycles / bad mailboxes | 0 / 0 | 0 / 0 |

All five settled owner responses and immutable disposable virtual NOR checks passed on both runs. Budget is 10,809 µs with zero XRUNs. These are first demonstrably jointly audible measurements for this cue, not a release pass or a controlled speed delta versus previous differently activated runs. The cue/observer compile in diagnostic only.
