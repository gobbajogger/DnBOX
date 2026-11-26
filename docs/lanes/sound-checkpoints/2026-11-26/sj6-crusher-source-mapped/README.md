# Source-mapped SuperJ6 Crusher calibration

The old measurements are valid signed audio, but the installed control
labels were wrong. This test corrects them and physically matches the
Apple stage. No firmware was changed.

| AU public address | Authenticated identifier and behavior |
| --- | --- |
| 107 | `neoncrushEnable`: sets Final Mix to 30% and the separate Booster to 1.3× when on; zero Mix and 1× when off |
| 108 | `neoncrushAmount`: Apple Decimation `6.06% * raw^1.23`; writes Rounding to zero |
| 109 | `neoncrushBitRate`: displayed as Lo-Fi; current dispatch has no DSP setter and the held-note test found no sustained effect |
| 110 | `crushMixControl` exists internally but is not registered in the current AU parameter tree |

Current installed signed Mac arm64 tree registers Enable/Amount/BitRate
at original image offsets `0x25edbc...0x25ede8`. The Amount callback
`0x27387c→0x0f1500` writes physical 3/4; the BitRate callback at
`0x273898` only fetches snapshot/metadata; internal Mix path
`0x273970→0x0f1598` writes physical 15. Hidden ID 110 is constructed
at `0x262100`. Booster DSP `0x39a008` multiplies each channel precisely
once by its own scalar, verified bit-for-bit on unequal stereo values.
There is no second per-channel multiplier in that owner.

The standalone Apple AU used real restored defaults (including Decimation
Mix 100%, Soft Clip Gain -0.9 dB), 44.1 kHz stereo and 128-frame callbacks.
Amount compares physical Decimation 0 vs 6.06% while keeping **both**
arms at Apple Final Mix 30%. Enable is scored independently, comparing
bare Mix 30% plus the verified external 1.3× gain vs bare Mix 0 at zero
Amount. Neither comparison measures a private installed PCM tap.

| Original OSC2 fader | Single-tone Apple input-equivalent peak | Magnitude residual after phase-based calibration |
| --- | ---: | ---: |
| .25 | .06784 | -.204 dB |
| .50 | .13951 | +.096 dB |
| 1.0 | .27617 | +.093 dB |

A separate mid-fader Enable comparison gives .150 held and .157 static;
its independent amplitude evidence is within .036 dB. The held note
Enable B→A returns to baseline, and public Amount responds to live
edits, so these observations do not depend only on cold fixture setup.
Full original Jupiter Sine contains small prefilter harmonics. This
sinusoidal equivalent is stage-local and approximate, not directly
measured input PCM nor parity across waveforms or presets. The previous
compressor-based estimate is at a different stage, a materially higher
inlet; do not add a gain based on the ratio without locating the
intervening gain/routing or another calibration assumption.

Recompute using the two older signed capture folders from this same
parent path:

```sh
python3 score.py native/bare ../sj6-crusher-gain-probe/native/installed \
  ../sj6-crusher-control-matrix/native/installed native/live
shasum -a 256 -c SHA256SUMS
```

## Cross-check against independently signed Crusher-off dry output

`python3 reconcile-dry.py ../sj6-auv3-compressor-inlet/native` reuses the
three matching fader/identity cases from the earlier independent capture.
It compares their output fundamental to our calibrated input equivalent,
with compressor, delay, reverb, AutoPan, widen and both choruses disabled.
The observed net transfer is approximately 0.427–0.450, close to the
authored raw master .44618. This is an end-to-end sanity check on the
inferred Crusher-input scale, **not** a decomposition of the intervening
owners. The old analytic stem RMS .19569 is not the latest authenticated
populated waveform; do not derive precise oscillator amplitude parity
from its numerical proximity. The genuine later pre-Normal sine
fundamental was about .3422; its comparison requires the downstream
owner and envelope convention rather than an assumed unity route.
