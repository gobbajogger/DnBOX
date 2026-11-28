# Synth One serial stage-split proof

Host-only experiment. This does not enable or add a native scheduler and
does not change the product renderer or qualify target timing.

Over each 32-sample tile, the control/voice stage emits one dry sample,
17 smoothed effects parameters, three LFO factors and an ordered delay
reset flag. The effects and output stage then consumes the tile in original
sample order. It holds the original intra-span frame index, including the
existing compressor and filter coefficient cadence. It neither changes
effect quality nor reads the voice stage's evolving smoothed state.
The tested compact profile disables high-frequency regeneration, so the
effects stage does not share the voice stage's random-number stream.

The harness compares each span's float stereo output, the complete owner
state, and its entire effect arena across changes of phase, modulation,
Mono, held notes, releases, and smoothing. Run:

```sh
bash experiments/s1-split-replay/run.sh
```

The frame payload is 88 bytes, so two 32-frame payload buffers would use
5,632 bytes before metadata. A future parallel version must prove the
ownership and timeout behavior, cost of the transfers, fit, and exact
audio before comparing native deadlines. This serial proof does not
establish that moving work to the second core would meet the deadline.
