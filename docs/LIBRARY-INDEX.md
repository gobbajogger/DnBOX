# Library index (files kept only in the User's ChatGPT Library)
Filled by the backup (`backup/dot-<date>/_manifest`, rows of kind `library`). One row per file. Restore = download from the Library by name, check SHA256, place at the path shown.
| Library name | Size (bytes) | SHA256 | Restore to | Why not in Git |
|---|---|---|---|---|
| SuperJ6-0.999.8.ipa | | | `/workspace/sj6-private/originals/` | third-party app installer (RULES 13) |
| SuperJ6 AU extension zip | | | `/workspace/sj6-private/originals/` | third-party app installer |

## Sound ownership and native-build recovery — 2026-11-18

The user's latest direction explicitly keeps the Sound checkpoint and recovery inputs in Library. These rows locate the bytes independently verified by the new Sound owner. Download by the exact linked Library item, verify SHA-256 before extraction, and keep all original-engine material comparison-only. No app installer or original engine becomes a new production dependency.

| Library name | Size (bytes) | SHA256 | Recovery |
| --- | ---: | --- | --- |
| [sound-classic-handoff-delta.tar.gz](https://chatgpt.com/library?file_id=libfile_fde3_f-5d323842039c8191b9949ddef4116279_2021479872048191acbfdca1f870a539&account_id=personal) | 30,979 | `4cc1b62ece0685eaa29b2c7ff86e7b58a4760a50b829bf27a18c38f6ef1bd773` | Extract to a new private reference directory; consume `CLASSIC-HANDOFF.md` and its exact source/PCM/reference pointers. Handoff branch head is `b90faafd2de71063448beea67aa5d22938a5c64b`. |
| [jieli-linux-toolchains-20260730.1.tar.xz](https://chatgpt.com/library?file_id=libfile_fde3_f-3f3b62014a388191b884a74f861fd091_2021479872048191acbfdca1f870a539&account_id=personal) | 26,027,508 | `a1545cddbc451a06bac43eb5cd96d831470c42a97c5c0b1f98879a20fc7647bd` | Extract separately; pass the resulting toolchain root to `experiments/sound-hardware-v1/reproduce_native.py --toolchain`. |
| [AC79 SDK V1.2.14 recovery archive](https://chatgpt.com/library?file_id=libfile_fde3_f-bec3af011f588191aa5fe724ba82f4bd_2021479872048191acbfdca1f870a539&account_id=personal) | 309,800,121 | `929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6` | Extract separately; pass the SDK root to the same native replay helper with `--sdk`. The actual source tree is `fw-AC79_AIoT_SDK-AC79NN_SDK_V1.2.14_2026-10-13`. |

Current source, tests, numeric behavior packets and measured candidate results are in [the existing Sound research package](../experiments/sound-hardware-v1/README.md). The [existing Sound Space](https://chatgpt.com/space/page_1d69bfdd65788191a98d1fe6e9c370b3) carries the current delivery report and gate state. Native diagnostic images and duplicate SDK/toolchain trees are rebuildable inputs and are not firmware payloads in this package.
