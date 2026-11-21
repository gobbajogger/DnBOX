# Stock Synth One dry voice and part

The firmware plays the INIT saw through its own amplitude envelope, resonant four-pole filter and six-voice summing path. Chicago, Miami and the shipped Juno remain available. Two parts can use Synth One simultaneously. Only the INIT patch is selectable. The existing performance parameters are pitch bend and damper.

The original reference ran separately from the firmware. Twelve cutoff settings supplied isolated impulse observations at resonance 0 and 0.5. The DnBOX model holds the input for two filter substeps, averages the previous two feedback values, and averages the two output values. The cutoff polynomial uses four degrees. For feedback the quadratic terms retained from a fourth-degree fit were sufficient on the note measurements. The unbandlimited saw wraps through one interpolated sample interval. The release test observed retirement at the first render boundary after the amplitude dropped below 0.01. See the numeric captures in `impulse-calibration.json`, `releases.json` and `quantum.json`.

The original dry-note protocol fixed gain at 1, absolute level error at most 1 dB, and the average spectral error at most 3 dB. The three MIDI pitches are 36, 60 and 84. All 180 static notes passed, with worst level/spectral errors of 0.470/0.415 dB. All 72 filter-envelope modulation notes passed, with worst errors of 0.019/1.111 dB. All samples were finite. The 24 modulation scenes include six zero-depth controls. Sixty static references have pinned PCM hashes. Two independent source-rendered controls byte-match the saved original PCM before the source host produces the modulation scenes. Each comparison records both PCM hashes, gain and note metrics in `results.json.gz`.

Restore the two-part Sound archive from the repository's [input index](../../../docs/LIBRARY-INDEX.md). Use a fresh output directory for each command. The first command needs Python, NumPy, C/C++, GNU patch and the pinned comparison objects. Calibration also needs SciPy.

```sh
python3 qualify.py /path/to/DnBOX-Sound-final-recovery /tmp/sound-notes
python3 calibrate.py /path/to/DnBOX-Sound-final-recovery /tmp/sound-impulses
tar -xzf part-gain-proof.tar.gz -C /tmp
```

The part-level packet contains the original chord producer, a verifier against the actual firmware part, and eight measured note/chord scenes. Two chords are bit-identical to the original singles summed at unity. The fixed one-sixth multiplier failed all eight scenes. The correctly summed part passed eight. Full application FX, factory program binding and the device callback depend on additional work.


## Immediate note retriggers

A synth can receive note-off and note-on for the same pitch before rendering the next sample. At velocity 100, the original plays a new attack after this pair. The shipping INIT previously continued sustaining. The production voice now advances its amplitude and filter envelopes once when the gate closes. Both retain their levels, and the oscillator phase and filter delay retain their histories.

The comparison uses the original dry scheduler and the actual firmware part at 44.1 kHz. For each pitch it renders two takes, one held continuously and one with both events at frame 19,200. The three original held-note PCM hashes match the earlier part-level recordings. Subtracting the held take isolates the sound of the retrigger. Before the change all three firmware difference waves were silent. Errors below compare the firmware and original difference waves during the next 8,192 samples, at their original gain and timing.

| MIDI note | Peak change in original | Peak change in firmware | Relative waveform error |
| ---: | ---: | ---: | ---: |
| 36 | 0.031088 | 0.031088 | −45.99 dB |
| 60 | 0.025957 | 0.025952 | −26.69 dB |
| 84 | 0.032912 | 0.032941 | −41.21 dB |

All eight established INIT note/chord scenes also pass their original 1 dB level and 3 dB spectral limits. Their worst absolute level error is 0.0202 dB and their worst spectral error is 0.3302 dB. These tests cover dry INIT. The factory graph and effects use separate comparisons.

Run `python3 experiments/sound-hardware-v1/stock-dry/compare_retrigger.py /tmp/s1-retrigger` with a fresh output directory. It restores the pinned source, verifies the held-note references and writes the full PCM captures and per-pitch results. To check the negative control, add `--candidate-tree` pointing to a checkout at `63278e3322aefa89b25f129ca2fccc35ef3b3b3d`.
