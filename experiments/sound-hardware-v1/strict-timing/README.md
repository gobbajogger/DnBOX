# Strict 360 MHz audio timing

[Measurements](measurements.json) identify the originating firmware package and ELF, emulator, scene, disposable flash, audio hash and guest counters per shot. Current stock firmware corresponds to `ca0c00a4e5863132de4a66f5d1c477c14d699be5`. Historical `d4088e60` uses the matched stock build with pad calibration disabled. Its published unshifted maxima were 11,406 µs Elite and 11,410 µs Pro. [Historical run](https://github.com/gobbajogger/DnBOX/pull/869#issuecomment-6846592540)

Run once for each board with its normal application package and matching ELF. The prebuilt emu360 `fm1-live` used here has SHA-256 `04c795b1961298cbd10d4bd9881ab97fde5eef7ff179f49a6b953dab78d43dc8`. Run in a fresh output directory. Native GNU `nm` must be able to read the supplied ELF. The driver uses an emulator-only flash file. It retains the full stereo DAC capture, RAM snapshot, guest core-load CSV and console output. The ELF supplies the locations of the guest counters.

```sh
python3 replay.py --emulator /path/to/fm1-live --elf /path/to/x0x.elf \
    --fwsc /path/to/board.fwsc --output /tmp/emu360-elite-sweep
```

Each board first creates its own factory flash image. A second 2,000 ms shot measures warm boot before the first input at 2,220 ms. Each 8,000 ms scene starts from a separate copy of that model's initialized flash. Offset 0 is the control. Offsets +1 through +5 shift all 922 event timestamps intact. All shots run with strict enforcement, default dual-core scheduling and the timed display. The table in [the lane report](../../../docs/lanes/sound.md) cites the firmware's `x0x_dbg.max_us`. Every stock scene processed exactly 648 halves. Both the raw late count and the increment over warm boot are reported. Production qualification also requires matching behavior and both native memory receipts.

The unshifted Elite observer reproduced the uninstrumented max, RAM snapshot and core-load CSV. Its worst half spanned 28, 228 and 256 engine frames. The separate PC sampler changed that result from 11,402 to 11,391 µs and is used only for source attribution. The sample window identified the 909 drum render and Chicago voice span as the largest compute regions. The stock scene produced zero nonzero BREAK halves with the optional bank absent.

The isolated [mixer/planner patch](mixer-planner.patch.gz) retains ascending scheduler tie order and eliminates settled-unity multiplication. Its measured Elite application was 546,468 B, with 6,204 B free and the same 8,972 B raw pool. The unshifted run saved 111 µs but missed the target by 382 µs. Its recorded stereo output differs at 122,151 channel words, beginning at frame 160,324. Its engine outputs matched all 50 deterministic files across 17 scenarios. In the native scene, the same K22 press triggered the same closed-hat voice one DMA period later. The branch carries only its evidence.


## Full sweep and DSP experiments

- [Trace and sweep archive](current-sweep-proof.tar.gz) contains reproducible control comparisons, exact input identities and all 12 stock results. [Historical reproduction](historical-shift-proof.tar.gz) includes all six Elite shots of d4088e60.
- [909 source and differential](909-render-proof.tar.gz) includes complete 7,648-byte state and four-bus comparisons, the sampler replay, the triangle-diode test and the real physical image sizes.
- [Equal-occupancy source and cold links](occupied-parity-proof.tar.gz) includes the complete experimental patch against ca0c00a4 and the linker and host audio receipts. The raw firmware payloads are reconstructed using the normal stock build.
