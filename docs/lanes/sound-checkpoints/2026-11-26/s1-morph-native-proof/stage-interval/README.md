# Two-core critical-path evidence on exact table-share diagnostic
Read-only protected clone replay, not release qualification. Same generated
table/share build as serial909 donor, same 922 events/offset 0, both synths and
arp, synthetic S1 A + Chicago B. No scheduler/store/feature removal; trace
adds 404 B to audio pool and native instrumentation. Guard leaves 116/124 B
APP and 160 B above pool reserve. Instrumentation can change trajectories and
is never a pass binary.

| Worst half | Elite µs | Pro µs |
|---|---:|---:|
| measured trace wall duration | 13,038 | 13,183 |
| combined stage job windows | 10,336 | 10,754 |
| measured Core0 job execution | 10,202 | 9,580 |
| measured Core1 synth job execution | 7,359 | 7,458 |
| already split Core1 808-prefix slot | 777 | 743 |
| ordered stage wait Core0/Core1 (inside jobs) | 5/0 | 1040/216 |
| serial remainder outside job windows | 2,702 | 2,429 |
| perfect-balance total, intentionally excluding already busy prefix | 11,482 | 10,948 |
| estimate including already busy prefix | 11,871 | 11,320 |
| remaining cut vs 10,809 after that ideal rebalance | 1,062 | 511 |

All three spans are plan mask 0x04 (S1 A on the worker), Core1 busy, no skipped
emulation work or persistent NOR writes (byte compare), 4 settled owner
readbacks, 319,488 PCM frames and clean worker mailbox. Standalone trace still
fails: 40 late Elite, 59 late Pro. Source below describes constraints and
does not infer performance from a different uninstrumented phase or model.

Ideal bound: preserve effective per-span barriers, take measured active CPU
demand and split infinitely finely/evenly, no transport or state changes.
Estimate = measured wall outside job windows + half summed job work (incl
prefix proxy). The even split is deliberately unrealizable: inputs/failures/
result access and ordered sums constrain role placement. Measurements have
observer overhead and native XIP/interrupt sensitivity, so this is a model
for observed workload, not a proof every future renderer must take this long.
In particular, the more optimistic bound ignoring 808 activity also misses.

## Source constraints
- Core1 waits for work in discrete 226/1287/718 us Elite and 189/1122/1015 us
  Pro windows (after its jobs, before global FX inputs exist). Can't sum
  these into one contiguous slot or claim its playback timeline is idle
  end-to-end. Chicago whole job 3365/2883 us already contributes to Core0
  and remains rejected by current stack admissibility. The packed dry and
  wet result cannot be started over multiple cores without defining a new
  per-owner ring/clock/commit protocol; the original sample order is
  observable. Earlier 909 split is excluded from this experiment.
- BREAK (324/304 us) is already considered by planner and remains last in
  sample-order mix. Even zero-cost ideal movement recovers far less than
  2.1 ms. Moving the 12 us recorder cleanup overlaps little. Post-join
  MIX and send shape (131/139 us) require all parts/exact canonical sums.
- Current shared FX and master are already pipelined in 32-frame batches
  when prefix permits. Cannot begin whole-state FX or master before the
  same sample's dry+send input is valid. End-to-next-span intervals of
  1277 us Elite /750 us Pro include macro updates, prepare, sequencer
  events, new engine boundaries and IRQ; not established redundant.
- Next precise independent profiling value: attribute those inter-span gaps
  and whole-block final master/foreground service using timestamps around
  original_prepare_all, seq_advance and audio IRQ boundaries, while holding
  the real PCM/event timeline. Removing at least 0.5–1.1 ms after perfect
  redistribution is a necessary working target, not proven feasible.
