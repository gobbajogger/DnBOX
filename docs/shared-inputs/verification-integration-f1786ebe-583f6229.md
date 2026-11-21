# Shared-input first checkpoint — 2026-11-21

Source of record: `integration/dnbox-consolidated` at firmware `f1786ebec2616477163b62b029c7e77405a9c3de` / Git tree `a4876790880cae769298f6701e8ab5db1d970b30`; nightly `583f622961fc9975fac746592ce6c483073c4170` / tree `7c10299ae8ccccb72771cd75b490b67bf2238eac`.

- GitHub refs and commit tree IDs: PASS (authenticated connector, live integration refs).
- Supplied JieLi ZIP SHA-256: `ed379c483468c44f10cf3be7db32356b9623e16320a021cf2a4e06ec96ab5f19` (49,449,905 bytes): PASS; extracted pi32v2 compiler reports `clang version 4.0.1` on Linux x86_64.
- Supplied older bootstrap SHA-256: `ea173ec3afd6adea9219622652ed0dcafe48f8a6cba43395c4dab62895928319` (6,323,764 bytes): PASS for integrity, **not compatible as DnBOX source** (historical `gobbajogger/X0X37-for-Loopy-Pro` tree).
- Current firmware `tools/build.py` pins AC79 SDK `AC79NN_SDK_V1.2.14_2026-10-13` and validates three file hashes; archive bytes are not in current runtime: NOT RUN for SDK.
- Current DnBOX and nightly source checkouts: GitHub object IDs verified, **not yet materialized into an unpackable bundle**. Local `git ls-remote` fails (no `github.com` DNS); no claim of complete checkout or full build.
- Target Project Source upload and fresh-worker restore: PENDING.

This document records the first checkpoint only, not a final successful reusable bundle. See matching JSON manifest in this directory; update evidence after actual bundle publication and unpack verification.

## Persistent Library package and readback evidence

- Library folder: `/LoopyX0X37/Project Sources` (personal Library, **not** verified as Project Files).
- Bundle: [download via Library](https://chatgpt.com/library?file_id=libfile_fde3_f-3873fcb84bd48191b77f0c3c8455fc5b_2021479872048191acbfdca1f870a539&account_id=personal); 49,455,924 bytes, SHA-256 `378050a7148466014dcfbae1e916ee94160f78629016d47a6b62cc1b3117b8c6`.
- Standalone JSON manifest: [download via Library](https://chatgpt.com/library?file_id=libfile_fde3_f-ebe5260065d081918b6a6e9ad51be043_2021479872048191acbfdca1f870a539&account_id=personal); 4,531 bytes, SHA-256 `b1cc488c9729383fb70c8bc6530eeda9c226a2d1205d5d1f78697dc9e377de22`.
- **Readback proof:** both uploaded Library items were materialized into a fresh container directory and were `cmp`-identical to the local originals. Bundle `unzip -t`: PASS. Restored `scripts/verify.py`: PASS. `bash -n scripts/restore.sh` and `python -m py_compile scripts/verify.py`: PASS. `restore.sh ... --inputs-only`: PASS; restored Clang reports `clang version 4.0.1`.
- **Git fetch negative proof:** `restore.sh ... --fetch` exits `128` with `Could not resolve host: github.com`. This is an environment limitation, not a source identity pass.
- This ZIP is a **pinned, network-assisted source retrieval kit**, not a byte-complete offline checkout. All source files remain in their respective GitHub repositories; the bundle contains the exact commit + tree validations, not those source bytes. DnBOX source tree 6,865 Git blobs (145,411,116 uncompressed bytes) and nightly tree 1,117 blobs (37,805,424 bytes) are not embedded.
- Current AC79NN SDK V1.2.14 files remain outside the kit. SHA-256 pins in firmware `tools/build.py` are recorded; no SDK binary check or firmware build was run.

**Final status: WORKING.** Package available and roundtrip-restorable through Library. Cannot represent it as a fully offline matched source bundle or an attached ChatGPT Project Source.
