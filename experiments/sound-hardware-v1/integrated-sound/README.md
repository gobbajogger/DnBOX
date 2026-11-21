# Combined sound firmware on SMK-37 Elite and Pro

The four development packages recorded in [native.json](native.json) were compiled from the firmware at [5837c73f](https://github.com/gobbajogger/DnBOX/commit/5837c73f754a667c526edd420aefbe7cf3a071bf). Both ordinary and opt-in builds use the pinned 20260730.1 JieLi compiler, AC79 SDK packaging inputs, stock board memory map and default empty built-in BREAK bank. The board qualification uses the Linux build of fm1-live at `ba83317aa55cbcdb2830c5da97d5f64b763499e2`. The separately built original CoreSampler renders run on Linux. No keyboard was flashed.

## Build and repeat

Build each profile in its own clean checkout, saving its ELF, application and package before switching profiles. Select `MODEL=elite` or `MODEL=pro`. Instructions for the toolchain and three SDK packaging inputs are in [BUILDING.md](../../../BUILDING.md).

```sh
BOARD=smk37 MODEL=elite sh build.sh
BOARD=smk37 MODEL=elite X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 X0X_SYNTHONE_WIDEN=1 sh build.sh
```

Run the existing [selected instrument fixture](../selected-native/README.md) for the isolated 808 and 909 and both saved bass/BREAK projects. The populated extension exercises the same 922 physical events after loading both kits, both basses and the 44.1 kHz loop. Every such scene must retain the correct firmware in its protected flash region. [Model measurements and input identities](native-selected-and-heavy.json) · [Isolated 909, ordinary versus opt-in](stock909-comparison.json)

For the calibrated default project, run [strict-timing/replay.py](../strict-timing/replay.py) separately for each model and profile, with all six shifts. Its exit status includes the 10,909 µs target and zero late transfers. It writes the measurement for a failing run. After the sweeps, compare the two profiles of the same board with the following command, supplying the matching actual files. The comparator checks the emulator, ELFs, packages, protected flash, starting user data, input scripts and stereo buffers. It reads the 36,420-byte project from each ELF's own RAM symbol.

```sh
python3 compare.py --stock-dir "$STOCK_SWEEP" --stock-elf "$STOCK_ELF" --stock-fwsc "$STOCK_FWSC" \
  --wide-dir "$WIDE_SWEEP" --wide-elf "$WIDE_ELF" --wide-fwsc "$WIDE_FWSC"
```

The [earlier six-shift pairs](before-engine-fixes.json.gz) use the consolidated FWUI/width source at 063f2306 before the original-engine fixes from #985. The [separate original-bank planner](../strict-timing/original-bank-planner/README.md) uses its own matched stock controls on the original #973 source.

## Host audio and memory

The [host replay instructions](host-replay.md) and [58 command records](all-engines.json.gz) identify both source arms, the two original drum engines and the AU comparison fixtures. The extra [Miami stereo check and mono negative](miami-stereo.json) exercise the active control.

The [sound lane](../../../docs/lanes/sound.md) records the per-engine contracts and known failures. The [flash and memory report](../../../docs/lanes/sound-memory.md) distinguishes shared workspaces from exclusive backend union members. The target ABI probe and range-counting script are in the existing [native measurement packet](../stock-fit-20261121/final-init-native-receipts.tar.gz). The expanded range attribution is specified in native.json.

Run `python3 tests/test_chicago_compact_headers.py` and `sh tests/host/synthone_widen_test.sh` from the source root. The width test restores its independent Soundpipe inputs from the repository. [Full width host results](width-host.json) · [Native display review](width-review-ledger.json) · [Browser shell composites](viewports/composites.json)

The production callback replay extracts the compiled stock render block and checks 2,048 dual-INIT callbacks per scene with and without 808/909. All 12 voices and finite preconversion samples were checked. The host was under concurrent load. Both deadline misses and successful checks are recorded. To reproduce, extract [the callback fixture](../stock-dry/callback-proof.tar.gz), run its `build.py <checkout> <out>`, then `report.py <out>`. The packet's historical replay wrapper checks the older source hashes. [Current callback measurements](callback.json) · [Compiled input manifest](callback-build.json.gz)

## Chicago lookup under concurrent calls

The combined dense reader uses immutable tables. An independent review reproduced an existing ordinary-reader defect. A cold lookup of UINT32_MAX hit the empty-cache sentinel. A controlled interleaving of two writers also produced coefficients from different indices. The shipping planner keeps Chicago on CPU 0. A 1.2-million-call stress run did not observe a spontaneous race.

A candidate moved the cache into each Chicago owner and rejected invalid input before lookup. It preserved all eight widest-profile artifacts across both boards. Ordinary Elite matched the entire +1 ms physical stream and project. Ordinary Pro changed 43,789 of 331,776 stereo frames and three mixer levels (909 115→105, 808 115→112 and bass B 115→105). The candidate remains in the [reproducible experiment](owner-cache-experiment.tar.gz); the ordinary firmware retains its measured baseline.

## All 694 Synth One host programs

The [portable input guide](../../../assets/test-inputs/synthone/README.md) restores the 97 pinned input files and runs the generic original-versus-compact host comparison. Both paths share the original effects. Each program produces 96,000 stereo frames at 48 kHz per arm. The [fresh result and digest comparison](factory-host.json) and [694 per-program rows](factory-host-programs.jsonl.gz) cover the host graph. The production selector returns `NO_FACTORY_BINDING` for every original entry when the [admission CSV](../factory-admission/programs.csv) is regenerated.
