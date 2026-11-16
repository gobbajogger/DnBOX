# Exact 2d86 native and map evidence

Source:2d86f59491d3ec33c1ab586a62ae0f4e9ff0a490. Independent defaultPADCAL1 Elite/Pro build results:2548/2556 bytes XIP free,1096 bytes ordinary RAM headroom,836 bytes pool margin above8192 reserve. Producer explicitPADCAL0 pair:4500/4512 XIP free,1528 RAM headroom,836 pool reserve margin. Disabled configuration is separate evidence, not adopted savings.

Map-only rebuilds use the same authenticated SDK214/toolchain20260730.1 and exact source, append only ld -Map, and reproduce both independent default1 ELFs byte-for-byte. The before binaries were copied from independent outputs, not claimed newly compiled in this map worktree. All5507 tracked blobs were reverified unchanged after each pair. Maps and logs here have paths normalized only; files.json binds original and public bytes.

The wrapper now checks build.main return status before issuing a receipt. The synthetic regression reproduces old return1-after-link false success, rejects it with the new guard, and preserves return0 success. These mocks are not native qualification. Existing real before builds had successful logs and matching ELFs; no historical native failure is inferred from the wrapper finding.

Full Local, all final engines, affected managed-host completion, target hardware/timing and endpoint admission remain open. Package/ELF bytes are retained privately and are not firmware releases or device-write authorization.
