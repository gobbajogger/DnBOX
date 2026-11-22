# Exact consolidated source and pinned-input delivery

All assigned prep work is verified. The reusable bundle and packaging manifest are saved in the actual [LoopyX0X37 Project Sources](https://chatgpt.com/g/g-p-ad9d6d4ab04c8191af6aa22fd752707e/project?tab=sources). An independent fresh worker downloaded both there, unpacked them into a disjoint directory, and completed the offline verifier with exit 0. No original parent bundle or checkout was used for that readback.

| Source | Commit | Reconstructed tree | Blobs / bytes |
|---|---|---|---|
| gobbajogger/DnBOX | f1786ebec2616477163b62b029c7e77405a9c3de | a4876790880cae769298f6701e8ab5db1d970b30 | 6,865 / 145,411,116 |
| gobbajogger/DnBOX-nightly | 583f622961fc9975fac746592ce6c483073c4170 | 7c10299ae8ccccb72771cd75b490b67bf2238eac | 1,117 / 37,805,424 |

Both source heads were acquired with authorized public Git transport and checked through the GitHub connector; both consolidated refs were rechecked unchanged before completion. The prep branch derives from the latest observed lane/prep-shared-inputs-delivery commit 6f6d1ba272fb553085b45879154623eb6cf66233. Only manifest and verification evidence are added to that base.

The bundle is LoopyX0X37-exact-consolidated-f1786ebe-583f6229.tar.gz, 127,006,815 bytes, SHA-256 96c3a29b328048b94f9553de8ae0ff1ce6cc474c2ee3bc39fc2f7e950fd0f115. Its packaging manifest is LoopyX0X37-exact-consolidated-f1786ebe-583f6229-manifest.json, 4,408 bytes, SHA-256 107e7d4bb26f2819bc4bfde7c7e3d9390bf90872653af9f8d29d661fbe74e7d1. That immutable packaging manifest predates the readback and retains its build-time publication-pending status; [the final manifest](manifest.json), [download receipt](project-sources-readback.json), and [fresh-worker result](fresh-worker-restore-result.json) record completion.

The existing SDK ZIP was reused, not uploaded again: 309,800,121 bytes, SHA-256 929ec0b652c1e246aa066c5ff4bc6068e1fd145501adce30187b8497b93276b6. Exactly its three inputs consumed by tools/fm1pkg_make.py are included under SDK/cpu/wl82/tools. Each actual byte digest matches tools/build.py: uboot.boot, cfg_tool.bin, and cfg/eq_cfg_hw.bin. The real tests/sdk_pin_test.py ran with AC79_SDK set both locally and on the independent downloaded restore: four PASS checks, no SKIP. [SDK input evidence](sdk-package-inputs.json) · [original test log](sdk-pin-test.log).

The existing compiler tar.xz was reused at SHA-256 a1545cddbc451a06bac43eb5cd96d831470c42a97c5c0b1f98879a20fc7647bd. The actual existing Project Sources ZIP was downloaded and compared: all 599 regular files and 44 symlink targets match; its only ten additions are macOS metadata. Compiler binary SHA-256 is c45189ee624de0430deb0db02e6dce171fb5538e7fe323e3964dc8ac6236240f, clang 4.0.1. Both restores produced valid pi32v2 ELF smoke objects. [Independent input evidence](independent-input-verification.json) · [local restore](local-restore-result.json) · [fresh-worker log](fresh-worker-verify.log).

Restore into a new directory on Linux x86_64 with Python 3.12:

~~~sh
mkdir fresh
tar -xzf LoopyX0X37-exact-consolidated-f1786ebe-583f6229.tar.gz -C fresh
cd fresh
python3 verify.py
export AC79_SDK="$PWD/SDK"
export JIELI_TOOLCHAIN="$PWD/restored-toolchain/jieli-linux-toolchains-20260730.1"
~~~

The verifier reconstructs both Git trees from all file bytes and executable modes, checks each SDK package input, restores compiler symlinks/modes, runs the real SDK test, and compiles a target object. Source history is not embedded; exact source-tree identities and their corresponding commit hashes are recorded.

Public Git transport recovered both trees successfully. Plain CLI push lacked write credentials, so the authorized GitHub connector published evidence with expected-head checks. Library download calls returned Unknown tool; the supported signed-in Project Sources UI provided downloads and publication instead. These routes resolved the access dependencies; none remain unresolved.

No firmware sources were edited. Full Elite/Pro firmware builds, product lane qualification, device operations, releases and public-site deployments are outside this prep assignment and are not asserted.
