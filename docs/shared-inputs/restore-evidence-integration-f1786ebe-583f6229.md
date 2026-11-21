# Shared build-input restore evidence — 2026-11-21 17:11 UTC

Committed on `lane/prep-shared-inputs-restore` (no other branch written). Source of record: `docs/shared-inputs/manifest-integration-f1786ebe-583f6229.json` on original checkpoint `3e06faac0aaba6569055b28b2cdee51d4f3bfe31`.

## Exact source identities (authenticated GitHub)

| Repo | Commit | Tree | Recursive inventory |
| --- | --- | --- | --- |
| `gobbajogger/DnBOX` | `f1786ebec2616477163b62b029c7e77405a9c3de` | `a4876790880cae769298f6701e8ab5db1d970b30` | 7,548 objects, 6,865 blobs, 145,411,116 uncompressed blob bytes, not truncated |
| `gobbajogger/DnBOX-nightly` | `583f622961fc9975fac746592ce6c483073c4170` | `7c10299ae8ccccb72771cd75b490b67bf2238eac` | 1,255 objects, 1,117 blobs, 37,805,424 uncompressed blob bytes, not truncated |

Commit and recursively enumerated Git tree object IDs match the original pins. **Neither source checkout was materialized into this runtime**. `restore.sh --fetch` exit 128: Git cannot resolve `github.com`. GitHub object checks are not proof that the actual source bytes were bundled.

## Published artifacts: actual Library status

- ZIP: [personal Library bundle](https://chatgpt.com/library?file_id=libfile_fde3_f-10d0d482801c819181a139adde0637ed_2021479872048191acbfdca1f870a539&account_id=personal), `LoopyX0X37-shared-inputs-restore-integration-dnbox-consolidated-f1786ebe-583f6229.zip`, 49,459,065 bytes, SHA-256 `387bea14003ab5637ea794a87a7b77d542775880bc3e0d66d831e0a08a05ef05`.
- Manifest: [personal Library JSON](https://chatgpt.com/library?file_id=libfile_fde3_f-58c73d3cb4408191a7434c5eb62b3982_2021479872048191acbfdca1f870a539&account_id=personal), SHA-256 `62e6f02dc72b51a1032af775d78d8006206bc6f6518a8ae38aa3ba66ccdf2bd7`.
- JieLi SDK V1.2.14 ZIP: [separate personal Library asset](https://chatgpt.com/library?file_id=libfile_fde3_f-884bd07411348191a96937009c5e02a3_2021479872048191acbfdca1f870a539&account_id=personal), 309,800,121 bytes according to Library metadata; ZIP bytes **not** independently verified because materialization is denied.
- Destination: `/LoopyX0X37/Project Sources` within **personal ChatGPT Library**. **ChatGPT Project Files attachment has not been verified**; Project Files/Conversation list is empty in this task context. Do not conflate the personal Library folder name with an actual Project upload.

## Tests performed

1. ZIP upload and standalone manifest upload both succeeded. Both downloaded from Library to a second clean working directory. `cmp` exact equality: PASS for ZIP and JSON. Verified the SHA-256 values above for both copies.
2. `unzip -tq` on the uploaded-and-read-back ZIP: PASS. `sha256sum -c SHA256SUMS`: PASS for all six listed embedded files. `python3 scripts/verify.py`: PASS for toolchain archive input.
3. `bash -n scripts/restore.sh` and `python3 -m py_compile scripts/verify.py scripts/verify_sdk.py`: PASS. Fresh `bash scripts/restore.sh ../kit-restore --inputs-only`: PASS with executable `clang version 4.0.1`.
4. SDK validation tool tests: synthetic ZIP with test-specific SHA pins, three-file extraction, and revalidation: PASS; synthetic ZIP rejected against production SHA pins: PASS. **Actual production SDK hashes: NOT RUN**, because five source archive copies plus the personal-Library copy refuse byte materialization.
5. Git source restore: FAIL (network/DNS), not a matching checkout. No firmware build, device action, release, or full fresh-worker test.

## Remaining completion gates

- Retrieve source bytes for both immutable commit/tree pairs, generate and verify an archive that contains their actual trees, republish and read back that full archive.
- Obtain actual SDK V1.2.14 bytes, run `scripts/verify_sdk.py --sdk-zip ... --extract ...` and verify all three `tools/build.py` checksums.
- Attach verified source+SDK bundle to **LoopyX0X37 ChatGPT Project Files**, and demonstrate that an independent fresh project worker can retrieve, unpack and verify matching sources and SDK.

**Lane remains WORKING; publication of an inputs-only ZIP is not completion of the requested full reusable source bundle.**
