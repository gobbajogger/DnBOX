# Split six dry voices between cores (isolated fixture)

This is an experiment, not an installable engine. It activates only in the dry six-note fixture. Apply `dry-six-parallel.patch` to the exact optimized six-voice dry source (same base and patch order as `dry-six-drums-result.md`, omitting the drum-load patch). It does not alter preset values or the number of voices. It keeps ordered per-voice summation, steps the noise LCG in original slot order on both lanes, and gives the worker its own mutable shadow in app-owned pool RAM. Failed worker work isn't mixed; after the HAL holds the failed core, the wrapper restores the pre-render seed and replays serially. This fallback was implemented, not fault-injected. Any Core 1 timeout, owner transition, or other live workload still needs qualification before production.

| Strict 8-second native emulator run | Held peak (µs) | Held halves over 10,809 µs | Full-run XRUN / late | App free | Pool margin |
|---|---:|---:|---:|---:|---:|
| Prior six dry Elite | 12,140.79 | 128/128 | 147 / 147 | 3,828 B | 26,492 B |
| Six dry Elite, split through release | 7,853.83 | 0/128 | 0 / 0 | 2,932 B | 16,836 B |
| Six dry Pro, split through release | 7,835.29 | 0/128 | 0 / 0 | 2,940 B | 16,836 B |

Both passing dry arms had 625 audio halves and zero skipped emulator cycles, with all six actual note presses/releases witnessed. Elite committed 294 worker jobs, 75,264 frames, no worker errors or RNG state mismatches. Splitting only the sustain left 19 full-run XRUNs, so the candidate keeps the split until the active release voices drop below four. Allocating the shadows in XPOOL overflowed that separate region; the passing links use app-owned guarded pool RAM, not an address or guard override. Cost is 9,656 B of pool and ~896 B of app flash on Elite.

Separate Elite correctness builds use `dry-six-base-fingerprints.patch` on the uninstrumented baseline and `dry-six-dry-release-fingerprints.patch` on the candidate. Run `compare-six-float.py BASE CANDIDATE`: all 320 stereo float-output fingerprints matched (256 held spans and 64 release spans, 81,920 frames before mixing/DMA). No timing claim from hash builds, and no all-program/wet/audio-device equivalence claim.

## Heavy-load status: unresolved

The original six-dry plus both sequenced kits run peaks at 15,646.83 µs / 152 XRUNs. A separate diagnostic profile (not a timing candidate) attributed mean normalized work per 512 generated frames to S1 on Core 0, ~10.90 ms; 909 and 808 on Core 1, ~5.16 ms and ~7.35 ms. Their stage ordering means waiting for both kits to finish before handing synth voices to Core 1 does not create capacity. Forcing both kits to Core 1 alone was 15,638.00 µs / 152; opportunistically trying to reclaim the same core after both drums was 15,636.29 µs / 152 and actually executed zero synth worker jobs. Plain O3 of the synth or both drum translation units likewise did not solve the loaded case. No second synth, BREAK playback, wet engine, or installer promotion is part of these tests.
