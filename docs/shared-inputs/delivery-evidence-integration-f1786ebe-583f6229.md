# Delivery checkpoint: 2026-11-21 — evidence and remaining constraints

Base: `lane/prep-shared-inputs-restore` at `7e47aadaf95051a7e93f2e46b2005b2be06574b4`.
Destination: `lane/prep-shared-inputs-delivery` (manifest and test evidence only).

## Verified exact Git identities

| Repository | commit | Git tree | API recursive tree |
|---|---|---|---|
| gobbajogger/DnBOX | `f1786ebec2616477163b62b029c7e77405a9c3de` | `a4876790880cae769298f6701e8ab5db1d970b30` | 7,548 objects, 6,865 blobs, not truncated |
| gobbajogger/DnBOX-nightly | `583f622961fc9975fac746592ce6c483073c4170` | `7c10299ae8ccccb72771cd75b490b67bf2238eac` | 1,255 objects, 1,117 blobs, not truncated |

The connector confirmed commit-to-tree OIDs and complete recursive trees; **neither actual tree was materialized**. `git ls-remote https://github.com/gobbajogger/DnBOX.git` failed: `Could not resolve host: github.com`. Vendor `curl https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK` also failed: `Could not resolve host: gitee.com`. GitHub connector can read metadata/text but does not expose binary source archives to this cloud container. The old bootstrap ZIP is a different historical repository.

## SDK evidence boundary

The provided Project SDK verification note says `AC79NN_SDK_V1.2.14_2026-10-13` archive 309,800,121 bytes, SHA-256 `929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6`. Its three member digests match the `SDK_SHA256` constants in pinned `tools/build.py`:

- `cpu/wl82/tools/uboot.boot` — `64ccc63e4df3238046fadb200f1b134311d8fe6ec6e3ccb8c8b81379231b867e`
- `cpu/wl82/tools/cfg_tool.bin` — `276579954f076886a6a7694f65dc71c034a63a2c204b76749065c0ac7b010d1b`
- `cpu/wl82/tools/cfg/eq_cfg_hw.bin` — `179ac9f4501d2cf8c3bdbc06fcda1e46228b7ace7e339342c767bb6568de57d8`

The SDK ZIP appears in personal Library `/LoopyX0X37/Project Sources` but `files.materialize` denied raw bytes: `This Project file does not have an authorized raw-byte materialization path.` **No production SDK byte digest was rerun here; no SDK is embedded.**

## Independently executed local checks

- Supplied toolchain ZIP: 49,449,905 bytes, SHA-256 `ed379c483468c44f10cf3be7db32356b9623e16320a021cf2a4e06ec96ab5f19` — PASS.
- Previous restore ZIP re-materialized from Library: 49,459,065 bytes, SHA-256 `387bea14003ab5637ea794a87a7b77d542775880bc3e0d66d831e0a08a05ef05` — PASS.
- Original restore ZIP integrity and all six internal SHA256SUMS — PASS.
- Original kit `scripts/verify.py`, shell syntax, Python compile — PASS.
- Fresh independent `restore.sh --inputs-only` — PASS; `pi32v2/bin/clang --version` = `clang version 4.0.1`.
- This delivery ZIP `zipfile.testzip()`, all nine embedded member byte comparisons — PASS.
- This delivery ZIP: 49,461,313 bytes; SHA-256 `0dc74706f2ac6d254d57941a296ab5b47dddb0f90afe76f7e2d4ed5ae410307b`.
- Standalone delivery manifest SHA-256 `f3dec4bf5e745c354b6cb05862fba3f4502a16e5e89362c2d9d4f124f03a09c9`.

## Persistence, acceptance and open gates

The `/LoopyX0X37/Project Sources` name refers to **personal ChatGPT Library**, not confirmed ChatGPT **Project Files**. Current Project Files listing exposes only the SDK verification note. The requested full source/SDK bundle and fresh-worker full restore are **NOT DONE**. This ZIP is explicitly `inputs-only` and must not be treated as a verified full source snapshot. Subsequent work requires authorized source Git archive bytes, authorized SDK archive bytes, verified local Git tree OIDs and SDK digests, plus a real Project Files upload and fresh-worker retrieval test.