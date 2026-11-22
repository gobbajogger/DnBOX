# Library index (files kept only in the User's ChatGPT Library)
Filled by the backup (`backup/dot-<date>/_manifest`, rows of kind `library`). One row per file. Restore = download from the Library by name, check SHA256, place at the path shown.
| Library name | Size (bytes) | SHA256 | Restore to | Why not in Git |
|---|---|---|---|---|
| SuperJ6-0.999.8.ipa | | | `/workspace/sj6-private/originals/` | third-party app installer (RULES 13) |
| SuperJ6 AU extension zip | | | `/workspace/sj6-private/originals/` | third-party app installer |

## Consolidated firmware baseline evidence

These archives preserve generated test outputs from the consolidated baseline at `76040daa5b52d686a8b7b544296ba5d79610a95d`. [Terminal gate and recovery receipt](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7056756805)

| Private file (version 0) | Bytes | SHA-256 |
|---|---:|---|
| [76040daa5b52-verified-positive-outputs.tar.zst](https://chatgpt.com/library?file_id=libfile_fde3_f-42ae122e9a2881919c3f5f69d3add887_2021479872048191acbfdca1f870a539&account_id=personal) | 113,353,279 | `fa4b55dab947f4e6b9f6d72a0487e52da40f2247c1cdd37b4974581cc212fa93` |
| [76040daa5b52-verified-positive-outputs.json](https://chatgpt.com/library?file_id=libfile_fde3_f-678d4dff57608191b28e02ecc141b4d2_2021479872048191acbfdca1f870a539&account_id=personal) | 630,306 | `0d22e3047bf09aa99575cb5c6e0014a5aae051983ae8d1e79f7fb1e1a8f39c46` |
| [76040daa5b52-stock-replayed-ui-output.tar.zst](https://chatgpt.com/library?file_id=libfile_fde3_f-e31af57818088191bb298829f3e62c06_2021479872048191acbfdca1f870a539&account_id=personal) | 20,384,391 | `e917e39d9ea24a5a8f57ff67752b91010e00b5c86fc18ef4c5d11a2188a88410` |
| [76040daa5b52-stock-replayed-ui-output.json](https://chatgpt.com/library?file_id=libfile_fde3_f-eade8c297b788191849bbabd2d71f14b_2021479872048191acbfdca1f870a539&account_id=personal) | 241,592 | `3a2fbc336bec6d7358a5bfa7a155cf35505fe541799a05e42abd8fc020be047a` |

Download an archive and its matching JSON manifest. Check both byte sizes with `wc -c` and both digests with `sha256sum`. Extract each archive into its own empty scratch directory with `tar --zstd -xf ARCHIVE -C DIRECTORY`. Verify the extracted paths, byte sizes, SHA-256 digests and modes against the manifest.

The first manifest maps archive paths to 1,871 passing-test outputs. The second lists 747 members under `members`, comprising 370 normal/sanitized PNG pairs and seven host executables from the [stock Linux replay](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058333600).

## Combined firmware cloud qualification

| Recovery filename | Bytes | SHA-256 |
|---|---:|---|
| `combined-3c6a5a04.zip` | 3,179,654 | `c5d6af67efa4b26b2ae538314f28ecbfdcc200643d681d1feaf85708d4b82aa1` |

Download the named ZIP into an empty directory. Check its byte count and SHA-256 against the table, then use `unzip -t` to verify its contents. The archive contains the two stock model packages and their application and link artifacts. The latest three-lane rebuild reproduced the application, ELF and package hashes for each model. [Exact model hashes](../evidence/integ-cloud-3c6a5a04/refreshed-build-identity.json)

## Sound ownership and native-build recovery — 2026-11-18

The user's latest direction explicitly keeps the Sound checkpoint and recovery inputs in Library. These rows locate the bytes independently verified by the new Sound owner. Download by the exact linked Library item, verify SHA-256 before extraction, and keep all original-engine material comparison-only. No app installer or original engine becomes a new production dependency.

| Library name | Size (bytes) | SHA256 | Recovery |
| --- | ---: | --- | --- |
| [sound-classic-handoff-delta.tar.gz](https://chatgpt.com/library?file_id=libfile_fde3_f-5d323842039c8191b9949ddef4116279_2021479872048191acbfdca1f870a539&account_id=personal) | 30,979 | `4cc1b62ece0685eaa29b2c7ff86e7b58a4760a50b829bf27a18c38f6ef1bd773` | Extract to a new private reference directory; consume `CLASSIC-HANDOFF.md` and its exact source/PCM/reference pointers. Handoff branch head is `b90faafd2de71063448beea67aa5d22938a5c64b`. |
| [jieli-linux-toolchains-20260730.1.tar.xz](https://chatgpt.com/library?file_id=libfile_fde3_f-3f3b62014a388191b884a74f861fd091_2021479872048191acbfdca1f870a539&account_id=personal) | 26,027,508 | `a1545cddbc451a06bac43eb5cd96d831470c42a97c5c0b1f98879a20fc7647bd` | Extract separately; pass the resulting toolchain root to `experiments/sound-hardware-v1/reproduce_native.py --toolchain`. |
| [AC79 SDK V1.2.14 recovery archive](https://chatgpt.com/library?file_id=libfile_fde3_f-bec3af011f588191aa5fe724ba82f4bd_2021479872048191acbfdca1f870a539&account_id=personal) | 309,800,121 | `929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6` | Extract separately; pass the SDK root to the same native replay helper with `--sdk`. The actual source tree is `fw-AC79_AIoT_SDK-AC79NN_SDK_V1.2.14_2026-10-13`. |

Current source, tests, numeric behavior packets and measured candidate results are in [the existing Sound research package](../experiments/sound-hardware-v1/README.md). The [existing Sound Space](https://chatgpt.com/space/page_1d69bfdd65788191a98d1fe6e9c370b3) carries the current delivery report and gate state. Native diagnostic images and duplicate SDK/toolchain trees are rebuildable inputs and are not firmware payloads in this package.

## Sound exact recovery and executed Elite pair — 2026-11-18

| Library name | Size (bytes) | SHA256 | Recovery |
| --- | ---: | --- | --- |
| [DnBOX-Sound-recovered-74fdcc42-20261118.tar.gz](https://chatgpt.com/library?file_id=libfile_fde3_f-034be894b64881919da2c5dc7122a285_2021479872048191acbfdca1f870a539&account_id=personal) | 81,171,985 | `a33c1054a6ff8f9d10e3218388dfa00c90e1f5f9eeeae520d9927c2fac8a517b` | Exact reviewed staged tree, publication plan and recovery index. Published without deletion as commit `9be11840b11d256f22a9af19d290cfe96f31a429`. |
| [DnBOX-Sound-local-Elite-fit-20261118.tar.gz](https://chatgpt.com/library?file_id=libfile_fde3_f-4abf329e18488191bda4aef75d18e025_2021479872048191acbfdca1f870a539&account_id=personal) | 5,292,068 | `5ee9b04d5d875d2d5f0e743f33491a0700f051ef247938298cc31930ed564b61` | Both allocated serial native links: exact inputs, maps, logs, terminals, physical-link failure and successful shared image, independent audit. No Pro or production admission inferred. |

## Recovered original dry PCM — 2026-11-19

| Library name | Size (bytes) | SHA256 | Recovery |
| --- | ---: | --- | --- |
| [sound-pcm-preservation.tar](https://chatgpt.com/library?file_id=libfile_fde3_f-63d0512781708191a4f53e3346480783_2021479872048191acbfdca1f870a539&account_id=personal) | 211,988,480 | `d5d11628a390f19d99783b1fb805979d371f5971efa01737c00bbc1e359b5403` | Download exactly, then run `experiments/sound-hardware-v1/dry_voice_followup/recover_references.py --archive PATH --output NEW_DIRECTORY`. It verifies all nested hashes and exports only the original mono column of60 physical recordings/180 notes. No original engine is executed or incorporated into the independent implementation. |

## Complete Sound recovery and SuperJ6 field addendum — 2026-11-19

The two parts contain exact canonical source at `358085d49a6b575dd6bfa79dd6c01c688741ea50` (tree `9520a215d07b7e054188fb4adf91acb78938638c`) and the frozen inputs, actual native maps/ELFs/objects/logs, numerical references, candidate/paired PCM and review records. All 10,666 archive entries were verified by size and SHA-256. Concatenate part01 then part02; the complete gzip tar is **737,000,061 bytes**, SHA-256 `e969679925e16bbd89ebf3192082578af597ae496815fe3dfd14a91df855c69e`. Extract into a new private directory and run its `VERIFY.py`. Original algorithms remain comparison-only. Consumed native plans are evidence, not a new execution allocation.

The separate small SuperJ6 addendum adds the independently qualified pre-note layer 1 attack binding and retains the source boundary. It does not replace the larger archive or change the historical fit/audio findings.

| Library item | Bytes | SHA-256 | Use |
| --- | ---: | --- | --- |
| [DnBOX-Sound-final-recovery-358085d49a6b.tar.gz.part01](https://chatgpt.com/library?file_id=libfile_fde3_f-0529d1417b7c819190a3af4a97867fad_2021479872048191acbfdca1f870a539&account_id=personal) | 402,653,184 | `11712b6da50b5f1d2e2858e2cd33590853b2ec6896bba89cf609a75f9184d926` | First consecutive byte range of the complete S1/stock-fit recovery archive. |
| [DnBOX-Sound-final-recovery-358085d49a6b.tar.gz.part02](https://chatgpt.com/library?file_id=libfile_fde3_f-988366e82f808191a7cd8621ccdd2bae_2021479872048191acbfdca1f870a539&account_id=personal) | 334,346,877 | `09d37cff2269d5c89081ae0460436413f4574959cb187864cff90bdbfb7e886f` | Second and final consecutive byte range of the same archive. |
| [SPLIT-MANIFEST.json](https://chatgpt.com/library?file_id=libfile_fde3_f-71bff0b76e1c8191a595b2bd70119511_2021479872048191acbfdca1f870a539&account_id=personal) | 2,179 | `5eac9577021debd27e9fcf6d3be76032929a9cadf0ad3de33e0da6a53e825763` | Ordered byte lengths and hashes for both parts and the reconstructed archive. |
| [DnBOX-Sound-SuperJ6-ID36-addendum.tar.gz](https://chatgpt.com/library?file_id=libfile_fde3_f-ffbba022a2e88191976d92efd3966bd4_2021479872048191acbfdca1f870a539&account_id=personal) | 14,888 | `76ea897b4788a7e7f2d510dfa4fe70b93879c4cbe6e0f150048fa33852437041` | Exact SuperJ6 ID 36 field-adapter source, focused tests, original failures, guarded replay, review and source manifest. Applies after the main archive. |
| [SPLIT-README.md](https://chatgpt.com/library?file_id=libfile_fde3_f-5a0c03f708048191b8e6bee00c115d6f_2021479872048191acbfdca1f870a539&account_id=personal) | 4,633 | `efbd9490645cfb463a0f3a82aecfcd2579935f90d154a925e7223d062ccbb3bb` | Restore, verification and content-boundary instructions for the main archive. |
