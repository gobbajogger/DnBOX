# SDK build-pin reconciliation and verifier acceptance/negative tests — 2026-11-21

Pinned DnBOX source: `f1786ebec2616477163b62b029c7e77405a9c3de`. Authenticated `fetch_file(tools/build.py, ref=<source SHA>)` returned blob `8c750473238614bf08b70bd85ca09115b593d41b`. It contains `SDK_TAG = "AC79NN_SDK_V1.2.14_2026-10-13"` and these exact SHA-256 entries, matching the provided Project SDK verification note:

| `SDK_SHA256` relative path | Expected SHA-256 | Source-code reconciliation |
|---|---|---|
| `uboot.boot` | `64ccc63e4df3238046fadb200f1b134311d8fe6ec6e3ccb8c8b81379231b867e` | PASS |
| `cfg_tool.bin` | `276579954f076886a6a7694f65dc71c034a63a2c204b76749065c0ac7b010d1b` | PASS |
| `cfg/eq_cfg_hw.bin` | `179ac9f4501d2cf8c3bdbc06fcda1e46228b7ace7e339342c767bb6568de57d8` | PASS |

Local focused tests on `scripts/verify_sdk.py` from the read-back recovery kit:

1. Synthetic SDK ZIP containing three uniquely selected `cpu/wl82/tools/` files, test-specific SHA-256 pins, validation and `--extract` of all three files — **PASS**.
2. Same synthetic SDK ZIP checked against the real production pins — **PASS: rejected**; first observed mismatch `uboot.boot`.
3. SDK ZIP with duplicate `uboot.boot` members under different prefixes — **PASS: rejected**.

**Verification boundary:** Those are tests of validation logic, **not** successful byte-level verification of the real vendor SDK. The project note says the actual 309,800,121-byte SDK archive was independently verified from a build working copy at SHA-256 `929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6`, but the matching Library ZIP could not be materialized here. `files.materialize` returned `This Project file does not have an authorized raw-byte materialization path.`; public Gitee hostname also failed DNS. Actual production archive SHA and these three contents **NOT RUN** in this runtime.

Source trees are verified as GitHub commit/tree identities only; no actual bytes or local Git tree reconstruction. The personal Library inputs-only artifact and exact read-back are in `delivery-publication-readback-20261121.md`. A full source+SDK bundle and independent fresh-worker Project Files restore are **not yet demonstrated**.
