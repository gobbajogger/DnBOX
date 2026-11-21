# Synth One LFO-controlled bitcrush rate

The original has two independently selectable destinations for its LFOs. A positive envelope drives the sample-and-hold clock by up to four octaves. Tremolo applies the inverse envelope to the held sample. The experimental ordinary-firmware implementation advances the same two clocks once per input frame, persists the new four-position route independently for both owners, and keeps the original hold state across control edits. The original filter, delay and reverb are outside this isolated comparison.

On each of the 694 factory programs the source and firmware received the same three-note, one-second dry performance at 44.1 kHz. Each arm produced 30,605,400 frames. When the original used the same available panels, all 694 compared within 1 dB level and 3 dB spectral error. Maximum errors were 0.00237 dB level and 0.06405 dB spectrum. With the original using the unrounded factory settings, 558/694 met both limits, including 107/114 with LFO modulation enabled. The original clock is sensitive to rounding at a hold boundary. All 694 inputs, including both forms of control quantization, are in the [per-program records](factory-rows.jsonl.gz) and [summary](factory-result.json).

The candidate uses previously spare bits in each owner's saved sound. Both stock and optional-patch cell validation passed under native and software FMA with sanitizers. The real panel, project save, switching both owners and reload kept the two routes independent. A separate native image included the upper SRAM allocation. With both LFOs routed to the crush clock and saved stereo phaser and master compression active, Elite took 11,115 µs and Pro 11,367 µs, both with zero late transfers and zero xruns. The standard populated Elite Chicago/Miami scene on this image took 11,690 µs and had one late transfer. [Native measurements](native-integration.json).

## Replay

The [control implementation](stock-controls.patch) applies to ordinary source `42ae56e1`. Apply it once when replaying from that base. The [saved project fixture](wet-project.patch) enables the two LFOs, crush, phaser and master for a selected-native populated Juno/Synth One shot. The projection directory supplies its published 21-control panel observations. The verified factory build supplies the original source and program data. The comparison script verifies the upstream kernel hash, copies the source expressions and uses the original Soundpipe oscillator and smoothing modules. From the repository root run the following once with matched panels and once with unrounded original controls.

```sh
here=experiments/sound-hardware-v1/synthone-crush-mod
OPENBLAS_NUM_THREADS=1 python "$here/replay.py" FACTORY_BUILD OUTPUT/matched
OPENBLAS_NUM_THREADS=1 python "$here/replay.py" FACTORY_BUILD OUTPUT/authored --authored
```
