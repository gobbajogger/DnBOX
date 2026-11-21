# Project Sources publication and fresh-worker read-back — 2026-11-21
Source: `lane/prep-shared-inputs-restore` at `7e47aadaf95051a7e93f2e46b2005b2be06574b4`.
Delivery branch: `lane/prep-shared-inputs-delivery`.

## Personal Library (confirmed uploaded, downloaded, and checked)

- **INPUTS ONLY, not full source tree:** [delivery ZIP](https://chatgpt.com/library?file_id=libfile_fde3_f-f2cafb6006d48191b39e51f448348085_2021479872048191acbfdca1f870a539&account_id=personal), `LoopyX0X37-shared-inputs-delivery-f1786ebe-583f6229-inputs-only.zip`, size **49,461,313 B**, SHA-256 `0dc74706f2ac6d254d57941a296ab5b47dddb0f90afe76f7e2d4ed5ae410307b`.
- [standalone JSON manifest](https://chatgpt.com/library?file_id=libfile_fde3_f-b16412eb9e708191878052701e18163b_2021479872048191acbfdca1f870a539&account_id=personal), SHA-256 `f3dec4bf5e745c354b6cb05862fba3f4502a16e5e89362c2d9d4f124f03a09c9`.
- [verification evidence](https://chatgpt.com/library?file_id=libfile_fde3_f-fbc4f3dbce848191973560501c3babdf_2021479872048191acbfdca1f870a539&account_id=personal), SHA-256 `9cbcdd5c376e0348f63ccc6fa58a19f994d7bc07de103a74530ed6739f0e91fd`.

All three files were uploaded into personal ChatGPT Library folder `/LoopyX0X37/Project Sources`, then re-materialized from that Library into a new working directory. `cmp` returned exact equality for all three. Fresh unzipped **Library read-back** passed `sha256sum -c SHA256SUMS` for eight members; `python3 scripts/verify.py` passed; `bash scripts/restore.sh ./restored --inputs-only` passed and executed `pi32v2/bin/clang` version 4.0.1. Earlier package attempt failed due missing `manifest.json` and was corrected *before* publication verification.

## Actual Project Files status

A `files.list(surface=conversation)` response shows the project-backed SDK verification note as `source_kind=project`. ZIPs and manifests appear only as `source_kind=imported` after Library operations. There is **no proof of an independent attachment to the LoopyX0X37 ChatGPT Project Files**, nor evidence a fresh worker can receive an offline full-source + SDK package. Personal Library folder names are not equivalent to Project Files membership.

## Completion blockers

1. GitHub immutable commits/trees are verified through authenticated API, but source bytes were not materialized: direct Git fetch failed DNS resolution of `github.com`; the connector exposes metadata/text, not binary Git archives.
2. The SDK reference note records full archive SHA and three build pins, but actual SDK ZIP raw-byte materialization is denied, and `gitee.com` DNS is unavailable. Production SDK hashes **not rerun in this runtime**.
3. Hence **no complete combined source+SDK ZIP exists** and no independent worker offline-complete restore was demonstrated. The posted ZIP is an explicitly labeled partial inputs-only recovery checkpoint.

Status: **WORKING**, not DONE.