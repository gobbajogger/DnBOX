# Keep the reversible two-tom split through the expensive release tail

**Diagnostic only.** Base is [the combined 48-row diagnostic](owner48-combined-drum-result.md) at `a54125b655d0aa245c89fd16ede63077c6fa9d33`. The patch [owner48-release-prefix.patch](owner48-release-prefix.patch) keeps exactly the same contiguous 808 prefix on Core 0 and the 909 plus remaining 808 on Core 1 during the first 16,384 generated frames after key-up, instead of dropping immediately back to the automatic planner. No preset, sound algorithm, voice count, DMA cadence, stack reserve, or display-capacity change. The generated-frame threshold applies **only** to the laboratory fixture; it is not proposed as a production scheduler.

## Uninstrumented strict replay

| Same six dry voices and two sequenced kits | Elite, before → after | Pro, before → after |
|---|---:|---:|
| Full-run dropouts | 79 → **65** | 78 → **67** |
| Worst full-run block, µs | 15,147 → **12,852** | 15,129 → **12,829** |
| Worst held block, µs | 12,843.46 → 12,852.13 | 12,880.67 → 12,829.46 |
| Held blocks above physical deadline | 57/128 → 55/128 | 57/128 → 56/128 |
| Free app flash | 1,392 → 1,376 B | 1,400 → 1,384 B |
| Audio pool free beyond retained 8,192 B reserve | 43,340 B | 43,340 B |

The 48-row canvas is still 23,040 B, saving 10,080 B versus the old 69-row compositor. Both replays checked actual six-note acceptance and release, 132 actual hits per kit, zero oversized display requests and zero emulator skips. The deadline remains 11,609.98 µs, and the margin target is 10,809 µs. This eliminates the isolated 15-ms release spike, not the held-note bottleneck; there is still no zero-dropout pass, wet-effects qualification, device test, installer change, or release.

Timing: [Elite](owner48-release-prefix-elite.json), [Pro](owner48-release-prefix-pro.json). In a separate native Elite correctness run, [all 160 pre-DMA fingerprints matched](owner48-release-prefix-fingerprints.txt), including 128 held and 32 release blocks. Do not infer uninstrumented timing or broader patch parity from that run.

## Why it works and what didn't

The [separate canonical Elite profile](owner48-release-barriers-elite.json) recorded the exact release spike at generated frame 171,520. Core 1 spent 5,481.33 µs on 909 then 7,440.71 µs on all 808, before the final serial processing. Synth One on Core 0 spent 7,428.13 µs actively rendering plus 5,670.04 µs waiting on the 808 stage. The observed block was 15,178.62 µs with instrumentation. Moving all of 909 onto Core 0 during that same interval [instead](owner48-release-balance.patch) merely serialized heavy drums with the still-expensive synth tail: 77 dropouts and 15,513 µs worst, worse peak than the baseline. It is not a recommended change.

At worst sustained frame 129,024 the two compute paths were already close: Core 0 spent 2,483.71 µs on the 808 prefix and 7,348.54 µs active in Synth One, before about 984 µs in the serial post stage; Core 1 spent 5,269.46 µs on 909 and 4,872.00 µs on the remaining 808. Saving compute on both cores or in the serial tail matters more than rearranging whole jobs during sustain.

The reverse path furthermore needs an explicit active-state snapshot before it is suitable for general use. The worker presently re-reads the kit's active state after Core 0 has started rendering the prefix. If the prefix owns the only voice and it becomes silent within a span, the worker could observe silence and discard already-produced samples. A separate active-voice end-of-tail and interrupted-worker test are required prior to changing production scheduling. The present snapshots cover the real two-kit stress fixture, not those corner cases.

## Recreate from scratch

Use the sibling `owner48-rebuild.sh`, `owner48-build.sh` and `owner48-replay.sh` with a checkout containing both pinned objects. The scripts use a new folder and do not require the original scratch disk contents. Apply this patch after rebuilding the source, then link both models before running the pinned Linux emulator in fresh per-model output folders. The pinned model-specific controller template supplies only its controller to the freshly built diagnostic. That matters: starting from a bare app package without that controller changed boot time and yielded a different count (76, not 79, on unchanged Elite). Both canonical baseline timing JSON files reproduced byte for byte; an independent run through the checked-in shell recipe also reproduced the 65-dropout Elite candidate.

## Independent experiments, not included in the paired result

- A voice-count shortcut retaining the split only while at least three release voices are active did not match the fixed-window result: 69 Elite dropouts and 13,933 µs max, with 1,136 B app flash. Keep it as evidence rather than inferring the route solely from a synth voice count.
- Removing two hot-loop diagnostic counters alone saves 80 B app flash on each board and leaves the audio pool unchanged, with 76/77 dropouts on the full canonical reference. Paired with the release overlap on Elite it leaves 1,456 B app flash and 65 dropouts at 12,788.38 µs worst. This combination is separate from the reported paired release-overlap result.
