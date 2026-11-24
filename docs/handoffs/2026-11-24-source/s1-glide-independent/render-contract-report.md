# Host timing gate for the combined engine configuration

The corrected harness runs both engine pairs through the complete host `engine_render` path and refuses timing data that lacks workload evidence. It passes the harness regression tests and correctly returns **exit 1** for the measured performance of the combined source. The existing baseline and limits are unchanged.

The isolated source copy matches all 313 firmware files in `full-control-candidate` (source manifest SHA-256 `a9b5ace196e4868e9437ca7e6a9873f2a9ae5a0251c727aa16f1e139b398802c`). The four changed gate files apply cleanly against the historical harness. The normal audio-stress caller keeps its original arguments and CSV format. The stock k8 parity fixture and the product source were untouched.

## Workload checks

- The timing entry point pins compact Synth One, typed SuperJ6 with built-in INIT, and the production shared arp in the source selection and C compiler. The actual stress source rejects a substitution with `X0X_S1C=0` or `X0X_ARP_HOOKS=0`.
- The two configured pairs are Chicago + Miami and Synth One + SuperJ6. The compact pair starts from INIT and enables the arp for both parts. The firmware's persisted arp-block decoder applies the settings. A wrapper counts successful backend note-ons above pitch 71, outside both the incoming MIDI chord (48–63) and the main pattern (48–71). This observes generated notes reaching each real instrument. The gate also checks actual compact `s1c_render` frames and typed SuperJ6 stereo frames, not just labels or commanded engines.
- Every marked block carries explicit engine IDs, a 512-frame length, 44,100 Hz sample rate, and thread CPU timing in nanoseconds. Backend activity is read after the timing bracket. The checker insists on both pairs and validates workload evidence for every repetition. All measured samples after the existing 16-half warm-up retain the 3,869,666 ns limit. Relative limits still use the existing best-of-repetitions policy and 30%/100% tolerances.

## Measured result

Four repetitions per pair ran 256 halves each, with 240 measured halves after warm-up, pinned to host CPU 8. Each repetition rendered 122,880 measured frames per part. Each compact repetition witnessed 32 out-of-direct-range arp note-ons on Synth One and 32 on SuperJ6.

| Pair | Worst measured host half | Late host halves | Ratios |
|---|---:|---:|---|
| Chicago + Miami | 4,732.7 µs | 1 | All within baseline tolerances |
| Synth One + SuperJ6 | 5,062.5 µs | 2 | Synth One 0.543 > 0.134, SuperJ6 0.231 > 0.123, block max 3.846 > 3.577 |

The old checker accepts the supplied timing-only legacy CSV. The revised checker rejects those exact files for missing workload columns. Both dry and arp-off source substitutions fail before linking. Six checker tests pass, including a late block in a nonwinning repetition, a genuine ratio regression, missing or mislabelled workloads, and missing engine or arp evidence. The existing non-timing stress check also passes its lost-note negative control and all three 64-half workloads.

The measurements use the historical host DSP compiler and Chicago decoder policy with the corrected engine selection. They provide no AC79 deadline, raw-late or XRUN result. The native 10,909 µs/zero-dropout gate is unaffected. No native run or release work occurred.

## Reproduce

Apply `render-timing-shipped-harness.patch` to a full checkout with the verified combined firmware and the historical gate files. Those files retain their existing registrations in `tests/run_tests.sh`. Run `python3 tests/host/test_render_timing_repetitions.py`, then `sh tests/host/render_timing_regression_test.sh` with the normal local host prerequisites. The measured gate exits nonzero when the limits are missed. The baseline file is deliberately absent from the patch.

See `render-contract-result.json`, `render-contract-host.log`, `render-contract-source-negative.log`, `render-contract-legacy-substitution.log` and `render-contract-checker-green.log` for the checks.
