# Compact SuperJ6 host regression

The candidate passed 82 assertions on the two DnBOX bass owners. Its four published Juno presets all produce different stereo samples from the unmodified Sound PR head. The original AudioKit AU did not participate in this experiment.

## Published DnBOX Juno presets

Each arm initialized a fresh preset through `original_sound_init` and `original_runtime_init`, using 120 BPM and 44.1 kHz. MIDI note 60, velocity 100, started at frame zero and released at frame 12,288. Each stereo recording contains 32,768 frames in 256-frame blocks. The capture point is the normal instrument runtime output after its chorus and master gain. There was no host gain adjustment. The `shipping` arm in the receipt is the unmodified PR head at `dea023ce632538621b59bb307ebcaf440805b032`.

| DnBOX preset | Different stereo words | RMS level change L / R, dB | Log-spectral RMS L / R, dB |
| --- | ---: | ---: | ---: |
| SUPERJ6 POLY | 65,536 / 65,536 | +0.265 / −0.025 | 2.867 / 2.888 |
| SUPERJ6 PAD | 65,536 / 65,536 | +4.819 / +4.572 | 5.594 / 5.536 |
| SUPERJ6 PLUCK | 65,536 / 65,536 | −0.955 / −1.239 | 4.334 / 4.355 |
| SUPERJ6 BRASS | 65,536 / 65,536 | +2.760 / +2.582 | 2.836 / 2.900 |

All eight captures were finite. Frame zero was the first differing frame on all four. POLY falls inside the 1 dB level and 3 dB spectral thresholds on both channels. The comparison uses the sibling scorer's existing channel metrics on DnBOX controls. It contributes zero original factory-program pairs to S4. Exact metrics and all eight PCM digests are in [observed.json](observed.json).

## Audio-boundary and ownership checks

`lifecycle.c` uses the published selector, queue, render and project-hold functions. Its linker wrapper observes the addresses of the two initialized synths. Both owners passed the following checks.

- A typed waveform edit coalesces with a subsequent edit before the next block. Zero-frame and held-project renders retain the staged value. The first nonempty block installs it before processing a queued note.
- A held or releasing note refuses a pre-note edit. A subsequent idle edit succeeds. The edited layer, second layer, peer owner, serialized sound and stored raw words are checked independently.
- Ordinary Juno macro 0 regains control of the waveform. Replacing the selected owner with another published preset restores the stock oscillator mode and clears the previous typed PWM word.

The firmware linker retains the typed entries. The four published factory cells exercise the stock macro route. The 640-bank selector and omitted-PWM main-pass tests are separate host fixtures.

## Replay

Use little-endian Linux, GCC, Python 3.10 or newer, NumPy, and the host build prerequisites of both DnBOX checkouts. The recorded run used GCC 14.2.0 and NumPy 2.3.5. From the directory containing this file, prepare fresh checkouts of the pinned base and apply the compact patch to one. The packet and scorer can reside in a newer checkout containing this experiment.

```sh
packet_dir=$PWD
base=dea023ce632538621b59bb307ebcaf440805b032
git worktree add --detach /tmp/sj6-head "$base"
git worktree add --detach /tmp/sj6-compact "$base"
gzip -dc ../native/current-head.patch.gz | git -C /tmp/sj6-compact apply -
(cd /tmp/sj6-compact && sha256sum -c "$packet_dir/candidate-sources.sha256")
python3 "$packet_dir/replay.py" /tmp/sj6-head /tmp/sj6-compact /tmp/sj6-host-run
```

The output directory must be new. The runner builds each renderer through that tree's production sources, records both stereo arms, runs the 82 candidate assertions and compares the fresh PCM hashes and metrics with the receipt. It retains generated PCM, host binaries and build logs in the output directory. The existing `tests/host/juno6_runtime_test.sh`, `juno6-dsp` gate and `juno_slot_integration_test.c` exercise additional behavior. No board or PI32v2 executable is involved in this host packet.
