# Normal Pro WIDE typed square profile

The [archived Integration source and replay](https://github.com/gobbajogger/DnBOX/blob/0afa1f1c8ee6fb45d62121f37d89bae2f44ce743/evidence/integ-resume/normal-wide-wet-1c46c6fc/README.md) links the source `1c46c6fc8d7a550b3308515a3415ea63d86e7486` to the exact package, ELF and saved flash. The unmodified scene scored 14,380 µs and 43 late halves. Dropping the Wave detent scored 10,691 µs with zero late halves. See the [published detent replays](../normal-wide-detents/README.md).

The official Linux emulator v0.5.1 sampled both cores from 5,620 to 5,670 ms. In the unmodified square scene, the worker recorded 134,188 weighted samples in the generic shape dispatcher and 116,073 in BLEP. Those functions had no samples on either core when Wave stayed on triangle. The worker recorded 99,895 samples in the four allpass updates. The planner distributed the triangle workload differently. The time samples identify hot guest instruction addresses and are separate from deadline qualification. [Profile attribution](pro-profile.json)

Use the same official emulator, package and seed as the detent replay. Copy the seed before each shot. Run the two existing scene-key files with `FM1_STRICT=1 FM1_NO_SKIP=1`, `FM1_BENCH_KEYS=@SCENE_KEYS`, `FM1_TPROF=OUTPUT,5620,5670`, and `--fwsc PACKAGE --mhz 360 --shot 5740 --flash SEED_COPY`. Pass the two raw profile outputs and the exact Pro ELF to `summarize.py`. The official emulator source for this interface is [v0.5.1](https://github.com/joakimeriksson/fm1-emu/blob/c7e47c4fcbdaafb4beffa40493da1b2b07ffa263/crates/fm1-live/src/main.rs#L915).

Each stock trial used the same Integration source and generated inputs, with only the listed typed DSP change or its compile option. The build used the pinned JieLi toolchain and AC79NN 1.2.14 SDK, `--board smk37 --model pro`, and these flags.

```sh
export X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_STOCK_PRESET=1
export X0X_DEV_PADCAL=0 X0X_COMPOSE_S1_EFFECTS=1 X0X_DNR2_SCRATCH=1 X0X_DNR2_NATIVE=1
export X0X_TANH_BOOT_TABLE=1 X0X_INTEG_DRUM_SIZE=1 X0X_INTEG_LATEST_SOUND_FIT=1
```

For timed candidates, the stock package's protected 643,072-byte prefix replaced only the matching baseline package prefix. The saved project, sample and other user-flash bytes remained identical. The replay used the original unshifted 922-event scene including Wave and Width. The test rejects any late half, any xrun, a peak at or above 11,609 µs, or less than 1,024 free XIP bytes.

The [deterministic oscillator probe](pulse-check.c) uses one million changing phase, pitch and duty inputs. Compile with C11, `-O2 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -DDNBX_DIAGNOSTIC_COUNTERS=1`, include the tested tree's `firmware/src/dsp` directory and link with `-Wl,--gc-sections -lm`. The pulse sample stream and final counter block matched the original at SHA-256 `ca610579e922c838a9d5fad64f2e21110c74bcbec4fa4de8cd99c0e530f39640` for the focused BLEP changes.

The smallest fitted Pro trial skipped calls for individual inactive BLEP edges. It improved the unchanged scene to 12,677 µs and five late halves with 1,052 B of XIP and 1,244 B above the pool reserve. The fastest measured scalar trial hoisted the waveform-stop conversion and reached 11,729 µs with one late half, 120 µs above the deadline and 20 B below the XIP floor. The isolated compile optimization that produced the same FWSC hash was scored by its existing equivalent run. [Exact source, package and timing hashes](pro-trials.json)

Guest SRAM at boot and 8 s showed byte-identical layer parameters, LFO phases and six pairs of voice states for the archived Pro WIDE patch. Sharing only the equal oscillators retained bit-exact PCM and full state in 1,800 host blocks, including unequal states, live Wave/Width changes, note release and resets. The unchanged Pro scene measured 12,059 µs and six late halves with 956 free XIP bytes. Moving the reused oscillator dispatch ahead of the per-layer state updates measured 12,214 µs and six late halves with 1,004 free XIP bytes. Both are diagnostic builds. The [complete layer-state probe](layer-check.c) compiles with `-O2 -ffp-contract=off -fno-fast-math -DDNBX_DIAGNOSTIC_COUNTERS=0` and links the tested tree's `dnbx_core.c`. The exact concatenated test output and state bytes have SHA-256 `f968eca7d7a24a65822fbd930c9c7fa30228dab7e09b36b2ac22d4733c4441c0`.

Full equal-layer sharing reduced the unchanged Pro replay to 11,585 µs and zero late halves or xruns. It required complete byte equality of both layer configurations and all six voice states before reusing a layer. The complete host PCM and state comparison was bit-exact. This diagnostic package retained 876 B of free XIP and failed the 1,024 B floor by 148 B. Its pool margin remained 1,244 B. The unmodified source rebuilt to byte-identical archived Pro FWSC and ELF hashes with the same flags and pinned toolchain. [Prototype patch](timing-only-full-layer.patch)

Pro timing remains open under the pair of stock constraints. The archived stock production package is unchanged. Stock promotion and Elite qualification remain pending.

To reproduce the full-layer prototype, use `git apply --unidiff-zero timing-only-full-layer.patch` on the exact Integration source named above before building.
