# SuperJ6 oscillator selection and repeated notes

The ordinary Sound firmware exposes four DnBOX programs, SUPERJ6 POLY, PAD, PLUCK and BRASS. The optional build also exports an entry for decoded rows containing eleven original controls. The test suite and experimental bank reader exercise this entry. The ordinary preset selector loads the four stock programs.

## Factory waveform after the stock PAD

The stock PAD's LFO rate and depth are 12 and 15 on their seven-bit controls. Starting from these values caused both factory square oscillators to inherit the old modulation even when the new PWM switch was off. All 24 square-wave probes at the four S4 pitches exceeded the 3 dB spectral limit. In one probe the distance reached 21.134 dB. The other 24 waveform probes passed. Selecting a factory row now clears the previous program's PWM modulation on both layers. Individual wave edits preserve the current modulation.

The corrected path passed all 48 basic probes, 154 range probes and 96 holdout probes. Worst absolute level error was 0.00850 dB and worst spectral distance was 0.533 dB. The two authenticated oscillator settings for PWM String Pad passed all eight pitch comparisons, with worst spectral distance 0.067 dB. Removing the reset gave zero of eight and 8.217 dB. All comparisons measure the pre-filter oscillator against the pinned public analytic component. The full stereo S4 matrix still lacks fourteen original-program captures.

Use the sibling oscillator bench with the pinned Git references described in its README. Run each of its three suites with the additional argument `--after-stock-pad`. The same argument works with `--remote-repo` for PWM String Pad. The real audio-boundary regression is `sh tests/host/superj6_factory_load_test.sh`.

## Repeating a pitch during release or sustain

Two keys play together. MIDI 67 remains held while MIDI 60 is released and struck again 64 frames later. The test repeats with the sustain pedal held. The public original-derived polyphonic sampler uses one voice for the repeated pitch in both cases. The previous shipping and factory renderers used two. Both now reuse the sounding pitch, while the unrelated key keeps its voice. This check measures allocation. The source comparator uses a synthetic looping sample at 44.1 kHz.

Run `sh experiments/sound-hardware-v1/sj6-factory/retrigger/replay.sh GIT_CHECKOUT FRESH_OUTPUT_DIRECTORY` with a checkout containing `4ea86b9e869011cae761709d13569b7bc32980a8`. The script compiles only the archived sampler for the comparison. The shipping test runs with `sh tests/host/juno6_runtime_test.sh` on all four stock programs, with both pedal settings. The factory renderer uses the existing `juno6-factory` host suite entry.

Each stock program also produced 132,300 finite stereo frames with the S4 note/chord/release schedule and an internal 120 BPM clock. Ordinary selection produced identical channel words with the factory feature disabled or enabled. There is no authenticated complete original AU preset paired with those four stock patches. The per-program PCM hashes and levels and the scoped component measurements are in [the results](oscillator-and-retrigger-results.json).
