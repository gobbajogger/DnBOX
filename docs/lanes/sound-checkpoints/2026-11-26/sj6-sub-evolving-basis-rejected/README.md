# SuperJ6 Sub — diagnostic source-time carrier, rejected
Run from the scratch root:
```sh
OPENBLAS_NUM_THREADS=1 PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench python3 sj6-sub-source-time/probe.py
```

Needs the existing authorized old MySub asset container and SFZ, the hash-pinned current actual ARM64 receiver, the proven same receiver test driver at recovered-sound/sj6-sub-handoff/probe.py, the preserved original fan-in test archive, and the exact compact core donor (path and pin in probe.py). The driver substitutes amplitudes only at the sample-loading seam and keeps source map, attack/held/release, exact monotone switch/commit, and receiver/filter intact. This is neither the latest complete native AU graph nor a connected firmware variant.

Seven stationary nominal harmonics plus DC, with independently regressed amplitude and unwrapped phase on non-overlapping 100 ms source windows starting at 75 ms then every 250 ms. Only the SOURCE samples in these windows train. Source time, not preset or elapsed MIDI time, selects and interpolates envelopes; no reset trick, source-to-output time alignment, global gain fit, or per-note tune. Earliest/last time offsets clamp to the nearest timed fit outside center-to-center interpolation; this is a limitation at onset. The diagnostic checks a completely absent compiled-receiver key (played key 73, sample root 72), as well as raw source slices strictly outside each root's training windows. Timing/cutoff fixture in prior handoff unchanged. HP/Crusher/master untouched. All fitted phase/amplitude values and raw media remain in local RAM. The archive contains no coefficients or sample audio.

Blind key73 markedly improves signed held/release to −39.27/−36.85 dBr, but onset only −12.75. Main Brainwave root36/54 onset and most signed held/release scores only ~−7 to −18 dBr. Across the independent raw intervals for root36/54/72, no better than −20.44 dBr. This is evidence that the source-clock behavior matters, not a parity pass. Logical descriptor bytes are calculated for all trained zones at 30 bytes per anchor, excluding synthesis code/state. Do not link or adopt.
