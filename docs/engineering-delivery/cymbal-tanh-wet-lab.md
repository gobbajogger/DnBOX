# Cymbal helper and RAM lookup: scheduler lab

Development only. Set `X0X_UI_SMALL_CANVAS=1 X0X_CHICAGO_FOLD_RESIDUAL_MIN=1 DNBOX_LAB_D9_SUFFIX=1 X0X_D9_TANH_BOOT=1 MODEL=elite|pro bash build.sh` with the usual JieLi dependencies.

C0 calculates private cymbal stems before the early Synth One render. C1 keeps canonical noise, rim/clap, tonal percussion, and sampler state. It either adopts the helper's finished state and sums in original voice order, or computes the cymbals locally without waiting. This is limited to the remapped mask and spans of at least 128 frames. Host differential: 23,280 complete state and four-bus comparisons, including finished, delayed and unchanged stock paths. The wider helper and the cymbal-only build without lookup relocation both regressed; do not substitute them.

The tanh table moves exact storage, not the effect. All 1,026 float words matched the stock table in all ten native cold, boot, and loaded RAM snapshots. All six loaded runs used 528 helper results, zero fallbacks. Test counters are present; no production qualification.

| Offset | Elite | Pro | Late / dropouts |
| ---: | ---: | ---: | ---: |
| 0 ms | 10,863 µs | 10,986 µs | 0 / 0 on each |
| 2 ms | 10,558 µs | 10,980 µs | 0 / 0 on each |
| 5 ms | 10,559 µs | 10,878 µs | 0 / 0 on each |

Every run is strict 360 MHz with the same 922-event source, no skipped cycles, unchanged simulated NOR and the same wet preset/arp/owners checked. Worst 10,986 µs is still 177 µs above the target. Both fit; 22,732 B shared pool remains after reserve. The helper and lookup consume 8,764 B relative to e5 while preserving 18,116 B of the canvas RAM saving. This is the smaller-display scheduler lab, not the combined Part Sound branch or installer.

Package SHA-256: Elite `74482d784e3dc5eb742da3f4fa01b2f914c3c8f79071b759a22015e2ae27ac6c`; Pro `c94b51ad76a8e3c90c92e86c8ce1b3a7701b15b7cfa67a10af88238d1dd4a0c6`.
