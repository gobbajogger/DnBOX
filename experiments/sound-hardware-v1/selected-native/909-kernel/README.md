# Native 909 kernel comparison

The probe calls the shipping `drum909_init`, `drum909_trigger` and `drum909_render_st_cleared` symbols in each linked PI32v2 Pro firmware. It triggers one set of voices at velocity 0.875, renders four 256-frame blocks, and rolls a zero-seeded hash over the binary32 words of all four buses. The instruction count includes initialization, triggers, clearing, and hashing. The depth covers the measured wrapper and the firmware calls it makes.

The masks select no voices (0), the three toms (28), four sampled voices (1920), and all eleven voices (2047). Both kernels produced identical hashes for every mask. The real eight-second heavy scenes also recorded stack canaries. The per-mask counters, scene measurements and host regression results are in `results.json`.

To reproduce the CPU measurements, run `replay.py --repo REPO --elf ELF --compiler PI32V2_TOOLCHAIN --gen REPO/build/gen --out OUT --rustc RUSTC --core-rlib FM1_CORE_RLIB` for each image. The compiler directory contains `bin/clang` and `bin/ld`. The matching emulator core is from the pinned native qualification toolchain. A built copy of `cpu.rs` may be supplied with `--cpu PROGRAM`.

The repository's independent host comparisons read the unmodified 909 from the base commit. Run both commands after generating the ordinary firmware headers.

```sh
export BASE=0ab227e2f727a9240e7df0ef972c062b5512357e
SKIP_TANH_EXHAUSTIVE=1 sh tests/host/run_drum909_struct_equivalence.sh
sh tests/host/run_drum909_integral.sh
```
