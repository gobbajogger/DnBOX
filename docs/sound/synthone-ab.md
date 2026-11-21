# Synth One factory A/B

The acceptance limits are an absolute RMS-level difference of at most **1 dB** and a log-spectral RMS difference of at most **3 dB** from 20 Hz through 16 kHz on each channel. Both arms use the same note schedule at unity gain. The production selector currently exposes INIT. Complete production-firmware factory comparisons stand at **0/694**.

## Harmonic oscillator in the full factory fixture

A fresh build of the pack-free harmonic oscillator ran against the original raw-table oracle with the original downstream effects. Both arms applied 144 ordered commands for each of 694 programs, played the six-note factory stimulus at 48 kHz, and generated 96,000 stereo frames per program. The candidate ran without opening a packed wavetable file. All 1,388 stereo PCM digests matched the recovered independent comparison. Left and right were scored individually with 2,048-sample Hann windows, 512-sample hops and a reference-relative −80 dB floor.

| Result | Value |
| --- | ---: |
| Programs inside both thresholds on both channels | 694 / 694 |
| Programs with byte-identical complete PCM | 492 |
| Largest absolute level difference | 0.000635 dB, right, *Soi ARP 7 Tcrushme* |
| Largest spectral RMS difference | 0.064061 dB, right, *The Pad* |
| Finite stereo frames per arm | 66,624,000 |

The host comparison qualifies the oscillator substitution in the shared original effects graph. The native INIT provider still needs the other factory voices, original control behaviors and a combined effects implementation within the stock memory budget.

From the root of the repository, use the restored factory build and the historical pack-free provider directory. The output directory must be new.

```sh
python3 experiments/sound-hardware-v1/factory-admission/replay_harmonic.py \
  "$FACTORY_BUILD" "$PACKFREE_PROVIDER" /tmp/synthone-harmonic-results
```

[Per-program stereo scores and PCM digests](../../experiments/sound-hardware-v1/factory-admission/packfree-stereo-694.jsonl.gz) · [Production program admission](../../experiments/sound-hardware-v1/factory-admission/README.md) · [Final widening-stage comparison](../../experiments/sound-hardware-v1/synthone-widen/README.md)
