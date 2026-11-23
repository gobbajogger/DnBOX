# Firmware optimization bench
Compare existing scored strict-emulator receipts before spending time on a full qualification. This is a **comparison tool**, not an emulator launcher or a gate change.

1. Build both candidates with the same stock model and profile. Run the existing short strict scene for each, using the same scene, emulator, clock and project data. Keep each FWSC, ELF, scored receipt and matching flashmap/budget log together. Start with WIDE + Synth One; use the scene relevant to the code change. Keep original-engine audio references separate.
2. Compare the matching receipts:

```sh
python3 tests/host/firmware_bench_compare.py --model pro \
  --baseline base/pro-wide.json --candidate candidate/pro-wide.json \
  --baseline-budget base/flashmap-and-budget.log \
  --candidate-budget candidate/flashmap-and-budget.log
```

For repacked seeds with different firmware bytes, supply both `--baseline-user-state-sha256` and `--candidate-user-state-sha256` from the replay's independently verified, identical user-data region. Do not hash the whole flash in place of that region or reuse one digest without checking both inputs. Exact-model metadata is required from the surrounding run when a short legacy receipt does not carry it.

The output reports baseline, candidate and deltas in peak time, deadline headroom, late transfers, xruns, XIP free space and pool margin. Identical audio is required by default. Use `--report-audio-change` only for intentional sound changes and separately score level/spectrum against the [engine reference benches](reference-testbenches.md). That option does not qualify fidelity. Compare like-for-like durations; a short capture does not prove long-term timing or execute the full release gate.

Tests: `python3 tests/host/test_firmware_bench_compare.py`. Actual receipt self-pairs reproduced the four archived long timing peaks (10,893 / 11,561 / 9,321 / 9,024 µs) and both archived eight-second WIDE cases. The improvement and negative cases in the unit tests use synthetic changes to receipts, not measured new firmware.
