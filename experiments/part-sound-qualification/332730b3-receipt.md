# 332730b3 full Part Sound qualification

Source: 332730b32a41e16578154c7d2e2da47a5ecf5cd8. Lab only. No device writes or installer promotion.

- Strict emulator at 360 MHz, 922 wet heavy-scene events, six phase offsets per model. Synth One 518 with ARP, Delay and Room on, and Chicago B. Zero skipped cycles, late blocks, audio underruns or virtual NOR changes. Worst Elite 10,646 us and Pro 10,669 us against 10,809 us.
- Reclaimed 16,064 B shared RAM. Both models retain 20,680 B after the 8,192 B reserve. XIP free 920 B Elite, 932 B Pro.
- Unmodified lab package hashes from the clean checkout with the six flags in WET-HEADROOM.md: Elite 9138844c5992003d48b27c2a922cd29cf8aadd818faa47f850f479ccb8bf5811, Pro 49592bb5140635204c04bf60a85bdfde28c64c35e27dbced24ac27bf4a1a6886. Replay packages separately insert the pinned emulator controller.
- All 1,026 Tanh and 513 Miami table words exact in cold/boot/loaded dumps. 528 private cymbal adoptions and zero fallbacks per loaded run. Lean variant saving another 6,712 B missed Pro shift 2 timing and was not accepted.
- Broader gate is not cleared: it found project fixture, held-note and sanitizer/environment failures. This commit updates only the Project fixtures; targeted scope passes all 47 tests, including opaque format 2 stream 16 archive/recovery and pre-write downgrade refusal. No runtime behavior or release admission changed.
