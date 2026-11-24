# Synth One stage costs and active voice experiment

## Samples from the pinned fixture

The host-only source shadow tags six regions in `s1c_render`. A Linux process CPU timer samples the current region at nominal 1.0 ms and 1.7 ms periods. The render thread writes stage tags and counts frames. Clock reads and histogram updates occur outside the sample loop. Timed candidate and control were built without tags or that sampler.

All 32 sampled runs processed 196,608 frames each and matched the independent uninstrumented control's PCM and MIDI across all 384 halves. The fixture loads the exact saved program-63 Synth One and Miami Sound with the shared arp. It also drives both drum kits and the same input and transport sequence as the earlier serial fixture. Of 3,041 timer samples, 1,440 landed inside the Synth One core.

| Synth One source region | 1.0 ms arm | 1.7 ms arm | Pooled |
|---|---:|---:|---:|
| Smoothing, controls and LFOs | 2.31% | 5.64% | 4.24% |
| Oscillators, filters and voice mix | 52.22% | 48.74% | 50.21% |
| Bitcrush, tremolo, pan and phaser | 21.09% | 21.49% | 21.32% |
| Delay | 0.00% | 0.96% | 0.56% |
| Reverb and its input and wet compressors | 18.29% | 17.77% | 17.99% |
| Master compressor and widening | 6.10% | 5.40% | 5.69% |

These percentages locate work in the tagged x86 host build. Timer granularity and compiler placement around the tags limit low-count estimates. The zero count for delay in one period is below useful resolution. These are host-attribution figures, and native callback timing remains unverified.

## Active voice experiment rejected

The candidate forms a polyphonic voice mask at the beginning of each render span after the existing release cleanup. The configured shared arp splits at external note edges. The per-voice renderer preserves slot ownership and a mono transition clears the mask alongside the existing voice clear. The mono path keeps its direct assignment. Polyphonic accumulation still adds audible voices in ascending slot order with the same arithmetic and histories. The candidate adds no persistent state or effect arena bytes.

The separate DSP oracle compared all 694 factory programs plus INIT across 27,800 spans and 3,094,140 frames. It checked exact stereo PCM, complete part state with pointer offsets normalized, and each byte of the effects arena. It included zero-length spans, six-note chords, release and reattack, mono and poly transitions, and reverb feedback changes. Both host timing arms also matched the independent control PCM and MIDI, including two runs with span timing switched off.

The whole-span CPU timer only brackets 1,072 Synth One calls per run. The active mask was faster in 8 of 16 unpinned pairs with a median delta of −0.42%. With both arms pinned to one CPU it was faster in 11 of 24 pairs with a median delta of +0.70%. Both pinned run-order groups had positive median deltas. The exact state result passed, but the speedup was not repeatable. The generator remains diagnostic evidence only.

## Reproduction

Start from a verified late-host archive and the independent control created by `run-late-host-profile.sh`. Use a fresh evidence scratch directory after the focused storage check.

```sh
sh "$PROOF/run-s1-stage-sampler.sh" "$ROOT" "$LATE_HOST" "$SCRATCH" > "$PROOF/s1-stage-samples.log"
sh "$PROOF/run-s1-active-voices.sh" "$ROOT" "$LATE_HOST" "$SCRATCH" > "$PROOF/s1-active-voices.log"
sh "$PROOF/recheck-s1-active-voices.sh" "$ROOT" "$LATE_HOST" "$SCRATCH" "$CPU" > "$PROOF/s1-active-voices-affinity.log"
python3 "$PROOF/summarize-s1-stage.py" "$PROOF"
```

The receipts are `s1-stage-voice-results.json` and `s1-active-voices-state.log`. No native slot was used.
