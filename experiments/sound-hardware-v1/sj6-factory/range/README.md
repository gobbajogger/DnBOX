# SuperJ6 factory oscillator range

The November 21 catalog found `neonoscRange`, app ID 86, in all 640 programs. The integrated loader at `63278e3322aefa89b25f129ca2fccc35ef3b3b3d` drops it. Feeding the 4-foot value alone to that decoder left its presence mask at zero. The build-9 Conductor contract places it at main-pass position 11. Its inverse-parameter default maps physical 1 over 0–2 with taper 1, giving normalized 0.5. The published register stops are 16-foot at 0, 8-foot at 0.5 and 4-foot at 1. [Control catalog and register stops](../../mapping/results/implementer_brief/numeric_behavior_targets.json) · [Archived build-9 order and defaults](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/control_contract.json)

The experimental [factory-range.patch.gz](factory-range.patch.gz) stages the first authored range word with the other factory controls and applies the register to both DnBOX oscillator layers at the audio boundary. The next selection restores 8-foot when the field is missing. The prepared pitch combines the register with the current performance bend; later bend changes preserve the register. Authored negative zero survives raw readback. The prototype rejects an unqualified intermediate range before changing the owner. The catalog gives the three named stops and occurrence count. The actual histogram of the 640 values and the original app’s off-stop quantization, complete callback side effects and final routed pitch still require the original inputs.

The Linux application fixture exercised both owners, three registers and three subsequent performance bends (−2, 0 and +2 semitones). At the default tuning, all **18,432 stereo frames exactly matched** neutral-register renders of the octave-transposed notes. All 96 non-neutral 128-frame blocks differed from the unshifted musical control. Both layers’ increments matched the nine corresponding MIDI notes rendered by the independent comparison-only CoreSampler at 44.1 kHz. The largest difference was zero binary32 ULP. This validates the conventional register-to-pitch mapping under the neutral fixture and changing bend. The reference component received MIDI notes; it did not execute the original app’s ID 86 callback. The application test rechecked 32 existing synthetic selections, 288 authored words, 64 PWM defaults, invalid inputs, held notes and transaction ownership. Native links, on-device cost and the complete program bank were not measured for this proposal.

To reproduce, apply the patch in a separate checkout at the firmware revision above and run:

```sh
gzip -dc experiments/sound-hardware-v1/sj6-factory/range/factory-range.patch.gz | git apply -
sh tests/host/superj6_factory_load_test.sh
```

The test contains the nine measured comparison pitch words and a two-ULP ceiling. To collect them again, build the pinned Linux sampler comparison using the [oscillator replay](../oscillator/README.md#linux-coresampler-voice-comparison). Call `sampler_reference(3, .5, note, ...)` at MIDI 46, 48, 50, 58, 60, 62, 70, 72 and 74. The original sampler and factory export are separate qualification inputs.
