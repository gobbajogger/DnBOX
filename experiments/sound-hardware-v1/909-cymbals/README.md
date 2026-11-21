# 909 cymbal comparison

The firmware and the host simulator play the same three checked-in IMA streams. A 32-path offline encoder selects the sequence of 4-bit codes. The ride has little energy near Nyquist, so its search also penalizes error near that frequency. Downward tuning brings that part of the source spectrum into the audible midrange. Playback still starts at predictor/index zero and reads the low nibble first.

| Source | Historical SNR (dB) | Shipping SNR (dB) | Encoded bytes |
| --- | ---: | ---: | ---: |
| Shared closed/open hat | 16.534 | 18.958 | 10,125 |
| Crash | 17.615 | 20.245 | 21,650 |
| Ride | 18.270 | 19.470 | 23,500 |

The direct 9W9 comparison uses commit `10cbe5c1e1c9e65b0818bb34644ab1a13092082c` of `athousanddetails/schwung-9W9`. Its three original WAV hashes match the manifest. We trigger both engines via the repository’s host comparator and compare one second of mono master output at 44.1 kHz. Each cymbal gets velocities 80 and 127, plus low/mid/high continuous and discrete settings for controls exposed by both engines. The [complete table](results.json) contains signed RMS level error and RMS log-spectrum difference from 20 Hz to 16 kHz. It uses the sound lane’s 2,048-sample Hann windows, 512-sample hop and −80 dB floor without alignment or gain adjustment.

All 108 production cases meet the 1 dB level and 3 dB spectral bounds. The historical ride at minimum tune had 3.847 dB spectral error, now 1.703 dB. The largest production errors are 0.347 dB level and 2.478 dB spectral. Lossy sample differences remain measurable under the stricter pre-existing pointwise test. The reference also runs separately with the production streams decoded into its WAV loader to isolate the engine from the codec.

## Reproduce

Run from repository root with a C++17 compiler, Python 3, NumPy and the pinned original checkout. The first two commands regenerate the three streams and manifest byte for byte. The player writes its logs, 108 case measurements for each arm, and the mono float samples in a new directory.

```sh
python3 tools/regen_909_cymbals.py /tmp/909-regenerated
diff -r assets/909/encoded /tmp/909-regenerated
sh tests/host/run_drum909_adpcm.sh
python3 experiments/sound-hardware-v1/909-cymbals/measure.py /tmp/909-codec-scores
python3 experiments/sound-hardware-v1/909-cymbals/player.py /path/to/schwung-9W9 /tmp/909-player
```

The decoder test compiles and runs the shipping decoder against both the production streams and the historical, digest-pinned streams. Set `CYM_AB` to the dated study directory to verify its external stream bytes and decoded WAVs as well. This replay used the pinned historical streams and their independent decode. The external study directory and physical analog recordings were unavailable.
