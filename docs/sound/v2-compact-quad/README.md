# Transactional compact quad reader, not activated

The morph runtime still calls the scalar API. New batch API resolves two row descriptors once and returns four exact words transactionally. No persistent cache, owner growth, payload changes, or DSP arithmetic changes.

TDD: missing API fails compilation; naïve forwarding passes behavior but observes 4 row validations; required2 fails; batched implementation observes2. Final staged test compares44,313,152 words against separately compiled unchanged original scalar reader, plus all identical positions, aliases, boundary/overflow/misalignment and late failure no-output-mutation cases. Original C/header and pack hashes are pinned in the runner. Actual compiler depfiles and before/after staged hashes bind tested bytes.

Default sanitizer result remains FAIL: LeakSanitizer reports fatal ptrace limitation. Separate address/undefined run used `ASAN_OPTIONS=detect_leaks=0`, exit0; it does NOT qualify leaks. Its earlier runner receipt predates explicit environment fields; the command environment is recorded here, not silently inferred. Managed default-sanitizer rerun awaits this published source pin and sole ownership.

Native31 units plus layout compile owner7972B/alignment4; unlinked executable+346B, other sections unchanged. Batch unused, not combined app fit or native timing. `native.json` losslessly deduplicates dependencies: expand each unit dependency path into ordered {path,sha256} entries from dependency_hashes and serialize receipt with json.dumps(indent=2)+newline; verify original_bytes/SHA256.

Callback-loop gprof excludes cold setup but includes audio-result/hash/CSV overhead. It samples ~32% moogladder,~10% row lookup,~10% compressors; diagnostic only, not deadline qualification or cause proof. Earlier per-callback profiler accumulated no timing samples; retained rather than interpreted as zero cost. No raw timing trace publication retries.
