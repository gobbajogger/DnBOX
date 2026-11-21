# Shared-input first checkpoint — 2026-11-21

Source of record: `integration/dnbox-consolidated` at firmware `f1786ebec2616477163b62b029c7e77405a9c3de` / Git tree `a4876790880cae769298f6701e8ab5db1d970b30`; nightly `583f622961fc9975fac746592ce6c483073c4170` / tree `7c10299ae8ccccb72771cd75b490b67bf2238eac`.

- GitHub refs and commit tree IDs: PASS (authenticated connector, live integration refs).
- Supplied JieLi ZIP SHA-256: `ed379c483468c44f10cf3be7db32356b9623e16320a021cf2a4e06ec96ab5f19` (49,449,905 bytes): PASS; extracted pi32v2 compiler reports `clang version 4.0.1` on Linux x86_64.
- Supplied older bootstrap SHA-256: `ea173ec3afd6adea9219622652ed0dcafe48f8a6cba43395c4dab62895928319` (6,323,764 bytes): PASS for integrity, **not compatible as DnBOX source** (historical `gobbajogger/X0X37-for-Loopy-Pro` tree).
- Current firmware `tools/build.py` pins AC79 SDK `AC79NN_SDK_V1.2.14_2026-10-13` and validates three file hashes; archive bytes are not in current runtime: NOT RUN for SDK.
- Current DnBOX and nightly source checkouts: GitHub object IDs verified, **not yet materialized into an unpackable bundle**. Local `git ls-remote` fails (no `github.com` DNS); no claim of complete checkout or full build.
- Target Project Source upload and fresh-worker restore: PENDING.

This document records the first checkpoint only, not a final successful reusable bundle. See matching JSON manifest in this directory; update evidence after actual bundle publication and unpack verification.
