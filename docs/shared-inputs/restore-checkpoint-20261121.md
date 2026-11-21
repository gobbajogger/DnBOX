# Exact-source restore checkpoint — 2026-11-21 17:05 UTC

## Source of record

Started from `lane/prep-shared-inputs` at `3e06faac0aaba6569055b28b2cdee51d4f3bfe31`. The immutable source identifiers are from `docs/shared-inputs/manifest-integration-f1786ebe-583f6229.json`.

| Repository | Exact commit | Git commit object's tree | GitHub object check |
| --- | --- | --- | --- |
| `gobbajogger/DnBOX` | `f1786ebec2616477163b62b029c7e77405a9c3de` | `a4876790880cae769298f6701e8ab5db1d970b30` | PASS (authenticated GitHub /git/commits/<SHA>) |
| `gobbajogger/DnBOX-nightly` | `583f622961fc9975fac746592ce6c483073c4170` | `7c10299ae8ccccb72771cd75b490b67bf2238eac` | PASS (authenticated GitHub /git/commits/<SHA>) |

**Boundary:** GitHub confirms commit-to-tree identities, but neither tree has been materialized locally. This is NOT a successful complete source bundle.

## Executed checks

- User-supplied `jieli-linux-toolchains-20260730.1.zip`: SHA-256 `ed379c483468c44f10cf3be7db32356b9623e16320a021cf2a4e06ec96ab5f19`, 49,449,905 bytes — PASS.
- Existing personal-Library ZIP `LoopyX0X37-shared-inputs-integration-dnbox-consolidated-f1786ebe-583f6229.zip`: 49,455,924 bytes and SHA-256 `378050a7148466014dcfbae1e916ee94160f78629016d47a6b62cc1b3117b8c6` — PASS against checkpoint. Downloaded its Library bytes back into fresh runtime; `unzip -tq` PASS, 6 members.
- Existing kit `python3 scripts/verify.py`, `bash -n scripts/restore.sh`, `python3 -m py_compile scripts/verify.py` — PASS.
- `bash scripts/restore.sh ../fresh-toolchain --inputs-only` — PASS; extracted `pi32v2/bin/clang --version` reports `clang version 4.0.1` on Linux x86_64.
- `bash scripts/restore.sh ../fresh-network --fetch` — FAIL, exit 128, `Could not resolve host: github.com`. Therefore both source folders are unverified and missing.
- Historical `LoopyX0X37-Project-Sources-Bootstrap-20261023(1)(1).zip` — SHA-256 `ea173ec3afd6adea9219622652ed0dcafe48f8a6cba43395c4dab62895928319`, 6,323,764 bytes; contains `source-at-7537a46.tar` for a DIFFERENT repository, not a substitute for DnBOX.
- `tools/build.py` from the checkpoint branch fixes SDK tag `AC79NN_SDK_V1.2.14_2026-10-13` and hashes: `uboot.boot` `64ccc63e4df3238046fadb200f1b134311d8fe6ec6e3ccb8c8b81379231b867e`; `cfg_tool.bin` `276579954f076886a6a7694f65dc71c034a63a2c204b76749065c0ac7b010d1b`; `cfg/eq_cfg_hw.bin` `179ac9f4501d2cf8c3bdbc06fcda1e46228b7ace7e339342c767bb6568de57d8`.
- Library metadata lists `/LoopyX0X37/fw-AC79_AIoT_SDK-AC79NN_SDK_V1.2.14_2026-10-13.zip`, size 309,800,121 bytes. All five detected copies refused raw-byte materialization with `This Project file does not have an authorized raw-byte materialization path.` Neither SDK bytes nor the three hashes are verified in this runtime.

## Actual persistence and fresh-worker status

- Existing ZIP and JSON manifest are retrievable from **personal ChatGPT Library** `/LoopyX0X37/Project Sources`; ZIP integrity and inputs-only restore were executed here.
- No evidence of their attachment to the **LoopyX0X37 ChatGPT Project Files**. This session's Project/conversation Files listing is empty even though two supplied input archives are mounted in the executor.
- Existing bundle contains only toolchain ZIP, manifest and scripts; it does **not** embed the two pinned sources or the SDK. A fresh worker with only the existing bundle needs GitHub network and SDK bytes before it can build.

**Status: WORKING; milestone checkpoint committed before 2026-11-21T17:30:00Z.** Subsequent evidence must not upgrade the complete-source or SDK checks without byte-level proof.
