# Original SuperJ6 Crusher Mix / Amount matrix

Original installed AUv3 and standalone Apple effect, both 44.1 kHz with
128-frame callbacks over the held analysis window. The installed arm
sets only public controls, verifies complete readback, and holds the
Jupiter Sine MIDI cue and oscillator fader (.5) fixed. Each captured
output is signed with SHA-256. No firmware was changed.

| Measured change at 440 Hz | Original AUv3 | Bare Apple Distortion |
| --- | --- | --- |
| Full Mix at zero Amount, relative to same dry arm | -15.791 degrees | +4.570 degrees |
| Full Mix at maximum Amount, relative to same dry arm | -15.782 degrees | -47.07 to -47.55 degrees |
| Mid Mix at either Amount | about -7.75 degrees | Amount and input amplitude dependent |

The full-Mix maximum-Amount repeat was -15.806 degrees. The bare
phase span covers input peaks .1, .25, .5 and .8. The installed
full-Mix amplitude ratios at the two Amount extremes were -1.211
and -1.014 dB respectively; maximum-Amount repeat -1.013 dB.
The change at zero Mix when Amount is toggled was -0.098 dB and
+0.001 degrees. Zero Mix and zero Amount still give **+4.435 dB**
and +1.576 degrees when Crusher is enabled, relative to disabled
at otherwise identical controls. That does not match the standalone
expected 1.3x Booster control (+2.279 dB). The measured excess is
not proof of a second Booster: compare the topology and live setup
before assigning it to any internal owner.

Neither the bare wet response, a fixed parallel dry mix, nor only
a different input level has been shown to explain the original.
No gain correction inferred and no release parity claimed.

Reproduce the analysis with:
```sh
python3 score.py native/bare native/installed
shasum -a 256 -c SHA256SUMS
```
