# Synth One: shorten the compact Room codec and retain the same history
Base e00f689dcab38f396dc23325e0f8eef515925489. Source experiments only; no firmware release.

- The original Q11 staging is unchanged. In the commit branch, use the integer peak and round-to-nearest-away quantizer rather than 16 float divides and float exponent discovery per block. The reader scales a binary32 reciprocal by its exact power of two. Stored mantissas/exponents and wrap stay unchanged. Both halves remove work without narrowing audio or disabling an effect.
- `room-int.patch`: encode only. `room-joined.patch`: encode + decode. `combined-room-voice.patch`: codec plus shared identical Room ingress and held-envelope shortcut; tested together, not shipped. Alternative patches are relative to the same base, not cumulative.
- `run.sh BASE CANDIDATE` checks every signed staged value for each legal exponent, every peak, every legal decode pair, then a million Room ticks including state and compressed memory. Both GCC host and renderer use non-fused binary32 semantics.
- `check-render.sh BASE CANDIDATE`: 694 packed factory programs plus INIT, variable spans, six notes, overlap/release and wet changes; compared 8,573,419 stereo samples and otherwise identical full compact state / arena. This is equivalence to the existing firmware engine, not a new A/B claim against the original app.

## Strict Pro emulator (six held INIT voices, stock clean source)

| Variant | Peak completed block | Last completed block | Completed halves | XIP spare |
|---|---:|---:|---:|---:|
| Baseline | 24,309 us | 24,171 us | 340 | 2,608 B |
| Encode only | 24,217 us | 24,090 us | 340 | 2,512 B |
| Encode + decode | 23,823 us | 23,705 us | 340 | 2,608 B |
| Codec + stereo ingress + envelope | 23,907 us | 21,604 us | 346 | 2,336 B |
| Idle baseline (no MIDI) | 11,481 us | 9,057 us | 391 | 2,608 B |

Pool margin stayed 512 B above the same 8,192 B reserve. The companion XDATA, FDATA and FITTAB are unchanged. `receipt.py` checks strict output and reads model-matched ELF symbols and raw stereo. Physical half-buffer period ~11,610 us. Matching input sends all six exact notes. All loaded arms miss the deadline, with 45+ fewer completions than unloaded; the parity xrun register misses double toggles. Raw consecutive duplicate spans are supporting evidence, not an exact lost-half counter. Differences in time-domain capture are possible on an emulator with late buffers; don't use raw hash across differing block schedules as an audio-equivalence gate.

A separate **fixture-only** pair forces delay and Room on for the first six-held span; it does not change stock preset support or day-to-day product settings. Codec-only 23,777 → 23,120 us and 340 → 352 completed halves, still too slow. Neither firmware is publishable. Exact physical eligibility requires both models, real control/kit loads and zero dropped halves, not these single-scene probes.
