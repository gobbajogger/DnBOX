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
