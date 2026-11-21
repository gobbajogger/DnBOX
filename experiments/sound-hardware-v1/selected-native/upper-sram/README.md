# SMK high SRAM allocation on 42ae56e1

The unchanged framebuffer uses 66,240 bytes of RGB565. Both render bands and their capture-before-input order stay in place. The four-file high-only patch moves both existing synth parts to previously declared application SRAM and places the existing 808 in the audio cross-core pool. Each owner retains its complete size and lifetime.

The original Felucca linker in 5684e187b0ef25b3b22fdcd88034398878ecbff3 declares the complete 0x01C7C000–0x01C7FD50 region as application NOINIT. Its later 29eca82b BLE extension assigned 0x01C7C400–0x01C7D400 to the BLE host. The current image still bounds its separate controller to 0x01C01800–0x01C07F00 through firmware/hal/x0x_ble_abi.h and ble/ctl/ctl.ld. firmware/app.ld reserves CPU0's stacks below 0x01C7C000. Its core1/DMA debug write limit ends at that same address. The high audio section is explicitly zeroed at cold boot.

The vendor AC79NN SDK V1.2.14 XIP linker fixes 52 bytes of boot_info at 0x01C7FD4C, before its separate 128-byte update block and vectors. The DnBOX debug write guard starts at 0x01C7FD50. The new linker region ends at 0x01C7FD00 and the actual payload ends at 0x01C7FC80. DnBOX BUILDING.md identifies the same AC791N and SPL for SMK Pro and the Elite variant. No cache-way or peripheral configuration changes enter this patch.

The two linked firmware packages retained 16,140 bytes above the fixed 8,192-byte pool reserve. All four strict populated 8-second scene receipts had zero late callbacks and zero audio xruns. verify_high.py checks ELF sections and objects, firmware/package identities, all 922 input events, audio timings and both unused adjacent SRAM gaps in the final memory captures. Run it as python verify_high.py <builds/s4-upperram42> <shots/s4-upperram42>. high.json records the result.

The full host UI/panel test passed 8,713 checks on both branch and trial. The 22 deterministic 240×240 color screenshots were byte-identical. The final physical-event emulation captures also include animated live measurements and can differ by the sampling instant. No physical hardware run was available for this experiment.

Vendor primary linker source: https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK/blob/AC79NN_SDK_V1.2.14_2026-10-13/cpu/wl82/sdk_ld_sfc.c
