# Stock normal WIDE qualification

Both stock packages passed the same unchanged 922-event software scene at 360 MHz in strict mode. The emulator was the pinned official Linux v0.5.1. Each package was run with its existing populated project and sample data. Seed rebasing replaced only the package-protected flash prefix.

| Model and WIDE owner | Peak | Late halves | Xruns | Free XIP | Pool margin above 8,192 B reserve |
| --- | ---: | ---: | ---: | ---: | ---: |
| Pro B | 11,561 µs | 0 | 0 | 1,036 B | 1,244 B |
| Elite A | 10,893 µs | 0 | 0 | 1,028 B | 1,244 B |

The 11,609 µs deadline and the 1,024 B XIP floor are unchanged. Both shots completed 645 halves. The guest reported all five active parts, 11+11 drum triggers, valid BREAK playback, and zero worker give-ups. Readback confirmed live square/square on Pro, triangle/triangle on Elite, the typed mode-2 wet chorus, and high-pass/full-sub Synth One. See the [Pro readback](pro-readback.json), [Elite readback](elite-readback.json), and [package and seed hashes](integration-pin.json).

Exact-equal sharing requires equal typed-square layer configuration and every voice layer's state to match byte for byte. The renderer advances the equivalent layer once, mirrors the resulting state, and retains the stereo accumulation order. The part ABI and persisted state layout are unchanged. The separate effects mixer alone uses `-Os` instead of `-O2` in the stock WIDE build. The intermediate setting reclaimed 160 B. A whole-object `-Oz` trial reclaimed 352 B but retained one late half. The guard emits the measured compiler commands on Pro and Elite and the existing commands for other configurations. [Build plan control](build-plan.log)

The exact compiled implementation passed the [1,800-block PCM and state comparison](layer-host.log). The [effects mixer check](fxbus-host.log) reused the existing silence, tail and restart fixture with the same source built at `-O2` and `-Os`. It compared the output channels, mixer state and delay lines byte for byte. The [391 WIDE checks](superj6_stock_wide_bridge_test.log), [factory load checks](superj6_factory_load_test.log), [held-chord storage and admission](held_chord_integration_test.log), and [panic](panic_chord_test.log) passed.

## Integration source pin

Use exact base `1c46c6fc8d7a550b3308515a3415ea63d86e7486` with [the two-file overlay](integration-overlay.patch). Apply with `git apply --unidiff-zero integration-overlay.patch`. The included program is byte-identical to the measured compiled input. The published build guard was verified to emit the same commands as the measured mixer recipe on both stock targets.

Use the toolchain, SDK and stock flags in the [preceding profile receipt](../normal-wide-typed-profile/README.md), then compare both FWSC and ELF hashes to [integration-pin.json](integration-pin.json). The [Pro timing](pro-timing.json), [Elite timing](elite-timing.json), [Pro stock link](pro-stock.log), and [Elite stock link](elite-stock.log) identify the bounded native run. Longer combined qualification remains separate.
