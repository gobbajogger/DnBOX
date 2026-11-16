# Conditional original-loader static audit

This is a fixed-input inspection, not original software execution or a generic Mach-O parser. Read SOURCE-FINDING.md for the conditional result and unresolved factory-path proof. No alias or runtime implementation follows from this audit.

## Inputs and recovery

- Original AU ARM64 binary SHA256: `a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1`. Restore it from the preserved original app material in the project's private Library recovery inventory. Do not publish the original binary or generated assembly.
- Analysis dependency: [official Capstone 5.0.9 distribution](https://pypi.org/project/capstone/5.0.9/), Linux x86-64 wheel `capstone-5.0.9-py3-none-manylinux_2_17_x86_64.manylinux2014_x86_64.whl`, SHA256 `273fd8d747d2e35c88f91450be51a603ecfaafb00d96d9f315dcb8689c86193e`. Hash the complete downloaded wheel before extracting its `capstone/` members to an isolated empty directory. No installation or global settings change is required. The verifier checks all 58 package members against capstone-pin.json before import.
- Distribution metadata reports 5.0.9; this exact wheel's Python binding reports 5.0.7 and native API reports (5, 0, 1280). Record these separately, not as an inferred version upgrade. The former temporary import directory disappeared during the investigation; the reviewed replay used a fresh isolated extraction from the hash-verified wheel, not that ambient directory.

From the repository, supply explicit paths:

```
SJ6_CAPSTONE_ROOT=/absolute/verified-package-root \
SJ6_ORIGINAL_ARM64=/absolute/original-arm64.bin \
SJ6_AUDIT_OUTPUT=/absolute/private-analysis-output \
python3 tools/sj6_loader_audit/receipt.py
python3 tests/test_sj6_loader_dependencies.py
```

Keep the dependency root free of extra files and Python bytecode. The process disables bytecode writes and rejects extras, symlinks, native-core overrides, wrong hashes and unexpected imported paths. Do not weaken a pin to make an unrelated installation pass.

The result has 39 fixed-address/import/layout/instruction checks. Six dependency-admission tests and four intentionally false assertion smoke checks do not execute or test original loader behavior. Control-flow interpretation still requires independent source review. Output assembly stays private; only the payload-free source finding and receipts are suitable for this branch.

The earlier exploratory whole-text reference scanner is deliberately not part of this reusable tool. The original text section contains an embedded table at 0x1000641e4 where sequential Capstone decoding stops; its empty reference result did not establish absence. Fixed-address receipt windows and raw-word caller scans do not rely on that scanner. Any future full-text reference scanner must report skipped regions and demonstrate coverage on known cross-references before use.
