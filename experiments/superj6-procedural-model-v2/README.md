# Isolated procedural model recovery

Generic GPL-3.0-only code and synthetic tests recovered from the retained math-renderer-r2 source. No factory banks, fitted coefficients, PCM, SDK or toolchain binaries are included. Original recovery archive SHA256: 2bf91cf7a85cb2a0fd1322acd67f07cd2a63b4fd17127c8070d36ae94b1437f6.

The admission changes reject model-open output overlap with borrowed input, misalignment and wrapping intervals before metadata reads. The later build-scratch guard also rejects scratch overlap with the admitted view or borrowed input before writes. Numerical build failure may leave partial scratch, which the caller must discard. Borrowed input remains immutable for the model lifetime. These checks cannot prove OS mapping or make forged model handles safe.

Run `python3 run_qualify.py --output <fresh-directory>` for five staged synthetic suites. Optional `--recovery-root` uses preserved local model fixtures; these are not bundled here. The render-state guard additionally rejects optional state overlapping the admitted view, borrowed input or built cycle, and rejects misaligned/wrapping state or cycle spans before state access. It preserves the existing valid-input math and requires live storage and an unchanged admitted view.

Current candidate evidence: 1,204 + 522 + 9 + 9 + 1,642 synthetic checks pass, and all 24 recovered vectors retain their frozen output hashes. The state-overlap regression first failed five of nine checks on the preceding source. Native object only: view 64 bytes/alignment 4, executable 3,426 bytes (+110), readonly 8,197 bytes, writable/BSS zero. Not a combined application link or timing qualification.

Independent review and managed default ASan/UBSan/LSan receipts for the preceding open/build source remain historical evidence only. The render-state candidate has independent source review and a fresh captured-runner replay of all five synthetic suites. Managed default sanitizer completion remains NOT_RUN; the reviewer did not repeat the 24-vector run. Its local default sanitizer exits 1 because LeakSanitizer reports ptrace; it is not sanitizer PASS, and no suppression was substituted. The shared managed environment is occupied by firmware qualification, so a new exact-source replay must wait for its coordinated release.

This historical approximation is not complete SuperJ6, an approved sound/design, a full control graph or factory corpus. S3/S4 and all lane completion gates remain FAIL. No engine activation. Prior poor onset/null results remain unresolved.
