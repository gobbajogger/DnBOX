# Synth One: exact sine lookup experiments

Diagnostic only. Based on the pinned lazy overlap build documented one directory up. No changes to production, Nightly, controls, voice count, effects, drum voices, memory reserve or timing gate. SuperJ6 is out of scope.

## Native Pro, strict six held Synth One voices with both kits sequenced

| Variant | Peak (µs) | Completed halves | Stale PCM spans | App flash free (B) |
|---|---:|---:|---:|---:|
| Rebuilt unchanged lazy control | 16,047 | 340 | 49 | 4,980 |
| Skip sine work when AutoPan depth is zero | 16,230 | 340 | 49 | 4,868 |
| Skip the muted sub waveform while preserving phase and signed zero | 16,203 | 340 | 49 | 4,772 |
| Merge equal LFO phase/smoother ticks (keep separate amounts) | 16,245 | 340 | 49 | 4,644 |
| Combine zero-depth, muted sub and paired LFO | 16,044 | 340 | 49 | 4,452 |
| Direct full-cycle sine lookup, alone | **15,430** | **341** | **49** | **4,372** |

The exact Pro full-cycle run repeated identically, including raw PCM hash. Elite went from **16,234 to 15,431 µs**, also still 49 stale spans, 341 completed halves and 4,364 B app flash free. Both variants keep the same **27,392 B usable pool margin** above the unchanged 8,192 B reserve. The 10,809 µs engineering target is still missed by 4,621/4,622 µs with both kits. One extra completed half is not a dropout pass.

The same full-cycle model binaries finished all **391** expected halves with **zero stale spans and zero late parity** on the six-voice solo fixture: Pro **10,426 µs** (329 µs faster), Elite **10,558 µs** (249 µs faster). That is 383/251 µs inside the 10,809 µs engineering target. This is one scene; wet, arpeggiator, automation and longer load qualification remain distinct.

A fresh unchanged Pro solo rerun matched the old receipt. Against that capture the direct-table solo output has 14,317 different PCM frames confined to an early region (first 77,730, last 97,313, zero-based). The final **136,509 contiguous stereo frames are bit-identical without alignment**, including the entire six held MIDI note passage. Both runs finish the same cadence. Early PCM is not identical, and its cause was not isolated. See `native-solo-pcm-delta.json`. Candidate Pro and Elite solo captures have the same whole-file PCM hash.

## What changed

The existing interpolator reconstructed the signed quarter-wave endpoints twice on each evaluation. The direct lookup pre-expands the identical 257 endpoint literals, including negative zero at phase 0x80000000 and positive zero at wrap, without changing the interpolation or phase. It works at any amount or preset, not only INIT. It uses 608 B more linked app flash and no extra pool RAM. The host renderer compared **8,573,419 exact stereo frames and complete DSP state** across 694 factory programs plus INIT. This proves host equivalence to the existing compact implementation, not original app parity or complete native proof.

The three guarded trials separately passed host arithmetic/state checks, and the combination passed that same full renderer comparison. The AutoPan leaf also passed 1,048,576 phase/stereo comparisons including live panning. The sub oracle passed 92,609,024 comparisons, including signed-zero boundaries. The paired LFO preserves independent amounts and tested live rate/shape/amount edits; it can pair in INIT (96/96 unedited host spans). None improved real audio cadence. Retain these recipes as negatives, not product code. Do not assume the speed of independently tested branches adds up.

## Reproduce

Use a fresh checkout at the parent pinned source, with the diagnostics from this branch available in a separate checkout or extracted alongside it. Use the existing checked SDK and JieLi toolchain. Run `bash build-variants.sh <source> <pro|elite> <base|pan|sub|lfo|combined|table>`. Each invocation needs a clean independent source tree. Use the parent `seed.sh`, `build-project.sh` with owner B (kit 2 for both; omit it for solo), the exact matched controller, strict replay helpers, and `receipt.py <elf> <run> 1`. Inject six held channel-3 notes at 4400/4403/4406/4409/4412/4415 ms: 48, 52, 55, 59, 62, 67, each with velocity 88. Files ending in `-inputs.txt` pin build, seed, scene, emulator and controller inputs.
