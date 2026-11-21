# SuperJ6 factory oscillator comparison

The compact factory oscillator failed every initial level/spectrum comparison to the archived analytic fallback component. The factory-only correction passed all 298 tested layer, shape, width and pitch combinations. At MIDI 60 the initial level errors were +6.02 dB sine, +6.11 dB triangle, +9.34 dB saw and +10.53 dB square at 50% duty. Narrow square pulses also had a large DC offset. The comparison uses the compact candidate with the typed waveform, AMP2 and master-volume patches.

| Suite | Initial passes | Corrected passes | Worst absolute level difference | Worst log-spectral distance |
|---|---:|---:|---:|---:|
| Four S4 pitches, six waveform/width settings, both layers | 0/48 | 48/48 | 0.00068 dB | 0.265 dB |
| Pitch and width checks used during development | 0/154 | 154/154 | 0.00077 dB | 0.533 dB |
| Eight additional pitches and new PWM settings after choosing the correction | 0/96 | 96/96 | 0.00850 dB | 0.519 dB |

Each result compares 8,192 samples at 44.1 kHz, starting with an idle oscillator. Both arms receive the same phase increment so pitch preparation is outside this comparison. The candidate uses its pre-note application helper, confirms exact raw readback, starts an actual DnBOX voice, and observes the running voice after the oscillator, before its sub, filter, gain, modulation effects and mixer. The second run changes the opposite layer's shape and adds another note on a separate voice. The fixture confirms that both layers of both voices sound and compares the selected samples as binary32 words. The command also enforces the two legacy compact quality modes’ 1,024-frame stereo hashes. A deliberate change to stock waveform interpolation triggered the legacy hash check before scoring the factory oscillator. The comparison reuses the S4 scoring limits of 1 dB absolute RMS level and 3 dB log-spectral distance between 20 Hz and 16 kHz. Individual arm hashes, signed level, log-spectral distance, correlation, DC difference, and source hashes are in `results.json.gz`.

**Reference and scope.** The comparison reference is the standalone analytic oscillator in the archived source port at `4ea86b9e869011cae761709d13569b7bc32980a8`. The historical qualification compared 42 sampler-component histories in total, spanning sample playback, envelope, filter and analytic fallback modes. That total does not identify a count of isolated oscillator captures. The public archive retains the comparison source and aggregate results. The original per-case PCM and control/event plan needed for direct replay are absent. This replay compiles the pinned header in a separate comparison library and runs it on Linux. It does not execute the original AU binary. The 298 controls are isolated probes, and zero complete presets were scored here. Runtime selection of an analytic fallback or a sample asset, effective LFO routes, instrument tuning, simultaneous sources, effects, retrigger and the complete three-second S4 graph need their own comparisons. [Archived original component and scope](https://github.com/gobbajogger/DnBOX/blob/4ea86b9e869011cae761709d13569b7bc32980a8/experiments/superj6-original-reference-20261115/README.md)

## Independent candidate

`factory-oscillator.patch` selects the new quality mode only when an original pre-note shape is applied. The candidate retains DnBOX sine and polynomial BLEP primitives. A unit-gain reference probe at increment 1/1024 measured a sine peak near 0.500, an inverted triangle approaching 0.495, a saw ramp slope of 0.68 per cycle and a 0.594 change between PWM plateaus. The positive pulse ends at wrap. At width 0.30, measured plateau levels were −0.1782 and +0.4158. The candidate uses these scalar observations, centers the pulse by its duty and corrects triangle slope reversals using the integral of DnBOX's polynomial BLEP.

The triangle, saw and pulse paths run at twice the output rate. A new two-branch filter removes the out-of-band components before decimation. Its four independent coefficients were designed against the halfband phase condition. For a normalized angular frequency w, each branch cascades two allpasses with response (a + z)/(1 + a z), z = exp(−2 i w). Bounded least-squares minimized the imaginary part of A/(B exp(−i w)) on 200 frequencies from 0.001 through 0.43π, starting the four interleaved coefficients at equally spaced values from 0.02 to 0.9. This produces 0.09110601, 0.31684510, 0.58723765 and 0.85663281. The signal path uses four additional floats per voice/layer, 192 bytes across one six-voice, two-layer part. The public static-allocation ABI advances to version 2. Only the comparison harness imports the archived reference.

An intermediate amplitude/DC/polarity correction passed 22/48 probes. The initial smaller two-state decimator then passed 48/48, but failed 22/154 range cases. The selected version also passed the 96 additional combinations. These measurements qualify the component comparison only. No native timing or image capacity follows from the host measurements.

## Reproduction

Run from the Sound checkout. The setup below uses a fresh temporary directory and the compact recipe already present on this lane.

```sh
repo=$(pwd)
sj6="$repo/experiments/sound-hardware-v1"
bench="$sj6/sj6-factory/oscillator"
tmp=$(mktemp -d)
git fetch --no-tags origin 0cddaeb822458fdf4ed3b0b9035fcf22248fda10 4ea86b9e869011cae761709d13569b7bc32980a8
git worktree add --detach "$tmp/compact" 0cddaeb822458fdf4ed3b0b9035fcf22248fda10
tar xzf "$sj6/stock-fit-20261121/native-receipts.tar.gz" -C "$tmp"
tar xzf "$sj6/sj6-amp2/typed-entry.tar.gz" -C "$tmp"
tar xzf "$sj6/sj6-amp2/oscillator-prenote.tar.gz" -C "$tmp"
for patch in 01-s1-with-shipped-juno 02-from-shipped-to-compact 03b-compact-amp2 04-amp2-range-dictionary
do
    git -C "$tmp/compact" apply --index "$tmp/stock-fit/$patch.patch" || exit 1
done
git -C "$tmp/compact" apply --index "$tmp/typed-amp2/typed-edits.patch"
git -C "$tmp/compact" apply --index "$tmp/oscillator-prenote/pre-note-wave.patch"
git -C "$tmp/compact" apply --index "$sj6/sj6-factory/master/compact-master.patch"
test "$(git -C "$tmp/compact" write-tree)" = d371377ada1f15a75ce33868c7d04172b273f8bb
```

With Python, NumPy, cc and c++, the same command evaluates either source tree. Exit 1 denotes out-of-tolerance audio.

```sh
python3 "$bench/replay.py" --compact "$tmp/compact" --repo "$repo" --out "$tmp/baseline"
git -C "$tmp/compact" apply "$bench/factory-oscillator.patch"
for suite in controls range holdout
do
    python3 "$bench/replay.py" --compact "$tmp/compact" --repo "$repo" --suite "$suite" --out "$tmp/$suite" || exit 1
done
```

For the 14 complete S4 programs, use either the selected records or the original archive. The archive is `J6presets.zip`, 451,262 bytes, SHA-256 `ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8`. Archive mode authenticates all 12 bank files. Both input modes verify all 14 canonical preset hashes against the checked-in S4 manifest and take the actual authored waveform and PWM fields for each layer. Missing amount and enable each default independently to positive zero at the main preset pass. It performs 112 isolated oscillator comparisons over 28 layer settings at the four S4 pitches. The exported archive was unavailable for this run. Receipts keep missing authored PWM values as JSON null and preserve present signed zeroes. The `amount` and `enable` columns report the main-pass words. The postpass needs the four live destinations resolved after the main pass. For captured routes 78/79 the original forwards commands to the corresponding LFO base controls 184/186/188/190 in order. The reference delegates those commands to a backend and does not establish the resulting physical PWM setting. Its per-tick LFO adapter also reads live destination, base, amount and waveform, and earlier callbacks can change the second route. No mapping from the four serialized selector values alone to a final duty is qualified. [Original postpass API and preconditions](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/preset_executor.hpp) · [Original live LFO adapter](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/lfo_apply.cpp) [Original conductor field contract](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/control_contract.json)

```sh
python3 "$bench/replay.py" --compact "$tmp/compact" --repo "$repo" --banks /path/to/J6presets.zip --out "$tmp/factory-oscillators"
```

With only the 14 selected records, place one UTF-8 JSON file named `<program_uid>.json` for each UID in the [S4 manifest](../s4-inputs.json) in one directory. Each file can be the unmodified `{"bank":…,"preset":…}` emitted by `factory.py matrix`, including its final newline, or the complete extracted `preset` object. Wrapped files must also match the released file digest and supplied bank UUID. Bare objects must match the pinned UID and complete canonical record digest, which binds them to their original bank position. Preserve JSON number types, parameter ordering and all fields when extracting. Each record is checked before any render begins.

```sh
python3 "$bench/replay.py" --compact "$tmp/compact" --repo "$repo" --selected-records /path/to/sj6-selected --out "$tmp/selected-oscillators"
```

## PWM String Pad from the Remote source

The Remote preserves 13 original binary32 control words for *PWM String Pad*, including all six base oscillator fields. Its archived comparison checked those 13 words on both owners against the exact original Intro Bank. The original member SHA-256 is `b7130d817c26809aed3b10acbda07a2a6d39ed5487355d07a0debd8c666c1c78`. This replay verifies the pinned Remote module, original verification script and receipt, then evaluates the module to obtain the two layer settings.

| Layer | Square selector and enable | Authored PWM amount | Initial passes | Corrected passes | Worst corrected absolute level / spectrum |
|---|---|---:|---:|---:|---:|
| 1 | 1 / 1 | 0.12632030248641968 | 0/4 | 4/4 | 0.000064 / 0.047695 dB |
| 2 | 1 / 1 | 0.5043136477470398 | 0/4 | 4/4 | 0.000099 / 0.066956 dB |

Each layer was tested at all four S4 pitches. The Remote fixture supplies two of the 28 selected layer settings. It supplies no complete S4 preset record. The eight measurements overlap the previously tested oscillator controls. All eight remained bit-identical with another selected layer and sounding voice. [Per-note measurements and source hashes](pwm-string-pad.json) · [Remote source](https://github.com/gobbajogger/DnBOX-nightly/blob/90cf5429a0aac5929c72d8abf387d70c399488ad/web/product-site/remote/superj6.mjs) · [Original field verification](https://github.com/gobbajogger/DnBOX-nightly/blob/90cf5429a0aac5929c72d8abf387d70c399488ad/evidence/remote-qualification-20261120/superj6-source.json)

Use a Nightly Git checkout containing the pinned commit. This mode additionally requires Node.js. The engine and archive matrix arguments have the same meaning as above.

```sh
git -C "$nightly" fetch --no-tags origin 90cf5429a0aac5929c72d8abf387d70c399488ad
python3 "$bench/replay.py" --compact "$tmp/compact" --repo "$repo" --remote-repo "$nightly" --out "$tmp/pwm-string-pad"
```
