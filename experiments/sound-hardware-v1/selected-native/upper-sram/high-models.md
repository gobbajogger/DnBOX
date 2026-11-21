# Upper SRAM on SMK Elite and Pro

On source `42ae56e1d83127fd9855fb8e86f8907af216b63a`, the layout frees 14,076 pool bytes. The 344,064-byte pool then holds 319,732 bytes and retains an 8,192-byte reserve, leaving 16,140 additional bytes for effects. The 240 × 138 RGB565 canvas still occupies 66,240 bytes. Both 69-row main render bands retain their original capture and input ordering.

| Section | Address range | Bytes used | Allocation |
|---|---|---:|---|
| `.audiohigh` | 0x01C7D400–0x01C7FD00 | 10,368 | Two independent `original_bass` owners |
| `.xpool` | 0x01C75200–0x01C77600 | 8,828 | Existing 5,120-byte workspace and 3,708-byte 808 |
| `.ble_ram` | 0x01C7C400–0x01C7D400 | 3,888 | BLE mailbox and host |

The former application linker at `5684e187b0ef25b3b22fdcd88034398878ecbff3` included 0x01C7C000–0x01C7FD50 in application NOINIT. Commit `29eca82b` assigned 0x01C7C400–0x01C7D400 to the BLE host. The pinned WL82 SDK reserves its first boot-info byte at 0x01C7FD4C. The tested high-SRAM allocation ends at 0x01C7FC80. The controller occupies low SRAM at 0x01C01800–0x01C07EE0. Both new relocations stay clear of the CPU stacks and guards. `tools/build.py` selects the allocation for the two SMK models. The startup path explicitly zeros the high section on both.

The four native 360-MHz emulator runs used 922 events each over eight seconds. Each owner stayed active and both the late and xrun counters stayed at zero.

| Populated scene | Elite maximum | Pro maximum |
|---|---:|---:|
| Chicago + Miami | 11,502 µs | 11,286 µs |
| Juno + Synth One | 10,553 µs | 10,887 µs |

`verify_high.py <build-dir> <shot-dir>` checks the linked sections, selected objects, exact firmware identities, captured boundary bytes and the four receipts. The 22 deterministic host panel images match and the baseline and candidate each passed 8,713 host UI checks. `check_models.py` compares both SMK binaries against the original passing allocation. It also verifies the legacy MKE board's 76 object checksums and the lower-level FM-1 link. The unchanged MKE image already exceeds its XIP region by 10,060 bytes. Both MKE link attempts produce that same overflow. Run `sha_products.py <fm1-baseline-build> <fm1-candidate-build>` to compare the FM-1 app, loader and installable package. All three are byte-identical.

`ble_boot.py` uses the repository's real dual-role controller and a disposable flash image. With the saved BLE setting enabled, both baseline and relocated apps reached READY, BLE up and host on. The emulated runs cover startup and native audio timing. Physical wireless links and hardware audio deadlines remain unmeasured.

Vendor linker source is at https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK/blob/AC79NN_SDK_V1.2.14_2026-10-13/cpu/wl82/sdk_ld_sfc.c
