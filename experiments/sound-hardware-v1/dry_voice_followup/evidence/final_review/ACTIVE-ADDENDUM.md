# Active-filter comparison addendum

**PASS: all three notes in one new D active-filter diagnostic.** For `filter-modulation/1.2_2000_0.9` (44,100 Hz, velocity 100), the saved metric report, its logged stdout and the terminal summary agree. Gain was 1 with no alignment or normalization; the limits were 1 dB RMS, 3 dB spectral, and zero nonfinite samples. All three notes were finite and within both limits.

| Note | RMS error (dB) | Spectral error (dB) | Result |
| ---: | ---: | ---: | --- |
| 36 | 0.2322267 | 0.6821656 | PASS |
| 60 | 0.2391121 | 0.7671472 | PASS |
| 84 | 0.1961054 | 2.1061539 | PASS |

Four serial, zero-exit receipts record exactly one host build, one static replay, one active candidate capture, and one metric execution. Static and active receipts name the same host binary. The static replay's hash matches the selected D baseline (`7aded201280455c1ddaee86abab4d82b6d791953e81d0d9cb40e17946cbfa380`). The active physical controls, notes, frame counts and sample rate match the numeric original manifest.

The new original mono capture contains 1,058,304 bytes (three notes × 88,192 binary32 samples); its manifest SHA-256 is `3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74`. The paired SHA-256 `338458922ef3861ec77898dfe386328a2ad6fa19897d265a010d106d96269bce` agrees across the terminal summary, metric report and metric stdout. These are cross-checks of saved evidence. The reviewer did not reread raw audio, open original source, rebuild, rerender, or execute the metric.

This closes the active comparison marked pending in the earlier review. It does not recover a historical nonzero-depth paired file, qualify the 54 historical nonzero-depth notes or the full 252-note matrix, or establish production or target sound. The separate 138/180 static result and its failed full static gate remain unchanged. No blocking reporting discrepancy was found.
