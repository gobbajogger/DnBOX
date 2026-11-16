# Isolated procedural model recovery

Generic GPL-3.0-only code and synthetic tests recovered from the retained math-renderer-r2 source. No factory banks, fitted coefficients, PCM, SDK or toolchain binaries are included. Original recovery archive SHA256: 2bf91cf7a85cb2a0fd1322acd67f07cd2a63b4fd17127c8070d36ae94b1437f6.

The only production change rejects model-open output overlap with borrowed input, misalignment and wrapping intervals before metadata reads. Borrowed input remains immutable for the model lifetime. These checks cannot prove OS mapping or make forged model handles safe.

Run `python3 run_qualify.py --output <fresh-directory>` for three staged synthetic suites. Optional `--recovery-root` uses preserved local model fixtures; these are not bundled here. Fresh 1,204 + 522 + 9 checks pass, and all 24 recovered vectors retain frozen output hashes. Independent reviewer replayed the synthetic suites and accepted the narrow source change. Native object only: view 64 bytes/alignment 4, executable 3,262 bytes, readonly 8,197 bytes, writable/BSS zero. Not a combined application link.

This historical approximation is not complete SuperJ6, an approved sound/design, a full control graph or factory corpus. S3/S4 and all lane completion gates remain FAIL. No engine activation. Prior poor onset/null results remain unresolved.
