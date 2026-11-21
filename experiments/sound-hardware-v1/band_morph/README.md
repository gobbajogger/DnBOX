# Synth One oscillator comparison

The DnBOX oscillator computes each of the four waveform endpoints from its Fourier series. It uses the stock float math library and carries no reference tables or mutable global state. The earlier direct projection used up to 2,047 terms per sum. This implementation sums at most eight directly. For larger harmonic counts it integrates the Dirichlet kernel, evaluates the sine integral and applies two terms of the smooth endpoint correction. The cosine series needed for the triangle follows from a second integral. The coefficients approximate mathematical functions independent of the comparison recordings.

The disabled-band version uses the raw triangle, square, narrow pulse and ramp. It interpolates the sharp edges across the last grid interval before a transition. With bands enabled, the bottom positive band extends above the Nyquist bin of the phase grid. Correcting the first trial's 2,047-harmonic ceiling and sampling both neighbors in that band fixed 36 edge comparisons and 12 stepped comparisons. Other bands use direct phase evaluation. The two raw inputs to a morph share the oscillator's phase.

| Original comparison at 44.1 kHz | Passing | Largest RMS / spectral error, dB |
| --- | ---: | ---: |
| Earlier grid | 133/133 | 0.161 / 0.161 |
| Exact band boundaries and adjacent binary32 frequencies | 520/520 | 0.161 / 0.161 |
| Earlier four stepped oscillators | 4/4 | 0.161 / 0.228 |
| Additional stepped audio, including lowest band | 91/91 | 0.162 / 1.606 |
| Oscillator fields at two pitches over 694 factory programs | 1,388/1,388 | 0.161 / 0.160 |

Upper bounds above are rounded up. The numeric report is `result.json`. The limits were RMS ≤1 dB and spectral distance ≤3 dB at gain 1. The original waveforms are comparison inputs. The field check honors each preset's actual oscillator-band flag. Of 694 programs, 214 enable bands and 480 use raw waveforms. All four ownership tests passed for mid-note waveform and width edits and peer muting. These checks cover individual controls and isolated oscillator audio. Full-program A/B coverage remains open.

From the repo root, use a fresh writable output directory for the two generated inputs.

```sh
python3 tools/restore-synthone-test-inputs.py --bundle assets/test-inputs/synthone --output /tmp/s1-morph-original
python3 tools/compile-synthone-factory.py --out /tmp/s1-morph-commands
python3 experiments/sound-hardware-v1/band_morph/check.py \
  --original /tmp/s1-morph-original/public-morphs.f32 \
  --commands /tmp/s1-morph-commands/ORIGINAL-PRESET-COMMANDS.json \
  --dsp firmware/src/dsp
```

## Stock two-oscillator measurement

The separate patch in `stock/` applies to the measured stock Synth One tree. Its original adapter comes from the November 19 private replay at #973's `0cddaeb8` reference. `stock/BASE.sha256` gives all modified preimage hashes. The patch adds a dedicated typed setter for physical fields 0–6 and 146. A call validates the entire group before applying any change. Performance IDs 0 and 1 still mean bend and damper on their existing entry. The two oscillator phases belong to their individual voice slots. The original dry INIT path remains available to existing callers.

The runtime test accepted each of the 694 authentic eight-field groups (5,552 words), preserved a held pedal and bend through every update, rejected invalid updates atomically, and retained the patch on panic. All eight fields changed the output in the deliberately active scene. Switching the muted oscillator's frequency, level and waveform left the other one bit-identical. Before and after, the original INIT stream hash was `8934eb02ab5e80a9`.

The compact stock ABI held 1,124 bytes per Synth One part on the host (previously 1,048); the larger engine union kept its original size. The target link used the real stock SDK/toolchain. A single linker root (`-u original_runtime_s1_oscillators`) makes the public setter reachable during qualification. Each ELF retained that entry, the setter and the Fourier evaluator.

| Stock model | Image | XIP free | Pool free | Ordinary RAM free |
| --- | ---: | ---: | ---: | ---: |
| Elite | 550,800 | 1,872 | 8,972 | 1,100 |
| Pro | 550,792 | 1,880 | 8,972 | 1,100 |

The anchored image grew 2,288 bytes on each model. These are stock S1 links of this isolated patch. Other simultaneous candidate changes require their own link. The factory importer still needs a real caller for the complete preset stream.

The two-part host test played six voices on each part and both oscillators on each voice. For 128 consecutive 512-frame blocks per scene, enabled bands averaged 1.57–3.22 ms of thread CPU. The largest observed block was 10.94 ms CPU and 10.95 ms wall at the lowest pitches. With raw oscillators the largest observed block was 0.68 ms CPU. The final run recorded zero wall deadline misses; an earlier contended run measured misses in both the base and experimental paths. No target callback timing has been established.

The controlled original dry comparison used the measured filter under test and six scenes (one frozen baseline, oscillator 2 alone, and the first authentic raw and banded oscillator groups with matching and differing semitone offsets). All 18 notes passed the unchanged 1/3 dB limits, max 0.130 dB RMS and 0.375 dB spectral error. `stock/evidence/controlled-dual.json` retains physical inputs, original and candidate hashes and each measurement. The integrated stock image uses the stock S1 filter. Sound equivalence of that entire image and complete presets remains open.

To replay the actual runtime test, check the preimages, apply the patch to an isolated stock tree, then run its host fixture. The build script has abort traps for unrelated Chicago/Miami references in this Synth One-only executable.

```sh
osc=/path/to/experiments/sound-hardware-v1/band_morph/stock
(cd /path/to/stock-tree && sha256sum -c "$osc/BASE.sha256" && gzip -dc "$osc/typed-oscillators.patch.gz" | git apply -)
bash "$osc/build.sh" /path/to/stock-tree /tmp/s1-morph-runtime
/tmp/s1-morph-runtime/witness /tmp/s1-morph-commands/preset-commands.bin
```
