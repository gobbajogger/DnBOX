# Exact guarded Pro heavy: per-owner trigger ablation

Same previously byte-identified guarded Pro ELF, unchanged app/companion
data and 922-event timeline, 360 MHz FM1_STRICT=1/NO_SKIP=1. Each synthetic
settings tail differs only in production PART OFF bits (and required
record length/CRC); native target layout pins seq.part_off at 30, A=4
and B=8. Actual diagnostic harness tests exact adopted 2712-byte Sound
(including both enabled arps and maximum 909 Tune) and native part-off
adoption and final retention. Original sequence and MIDI/physical
events input identical, 909 always enabled; no production loader,
firmware, feature, package, release, hardware or real sample changes.

| Pro arm, 8 s offset0 | Peak µs | Raw late = XRUN | Halves | Late fraction |
|---|---:|---:|---:|---:|
| Both trigger normally, part_off=0 | 27684 | 92 | 396 | 23.23% |
| Synth One A new triggers blocked, part_off=4 | 21563 | 193 | 430 | 44.88% |
| SuperJ6 B new triggers blocked, part_off=8 | 19312 | 193 | 430 | 44.88% |

The baseline exactly reproduces the previous run; both trigger-suppressed
arms still fail independently as workloads with 909 and the other
active synth. Off B reduces peak by 8372 us and Off A by 6121 us in this
pair. These are whole-run deltas, not a decomposition into DSP call time.
A dormant FX/tail is not removed by stopping owner new triggers. More
late blocks at a lower maximum and different half counts show a different
schedule; final in-RAM Sound hashes also diverged, even though all adopted
Sound bytes and frozen physical/MIDI stimulus started identical. No claim
that live effective trajectories of the untouched owner or knob paths
were equal across runs; cannot assign causality to individual controls
or to relocation. All emulated warm flash files unchanged.

Replay uses run.sh in disposable exact guarded Pro link workspace; copy
native-layout.c as off.c and apply solo-arp.patch to a copy of the existing
init-fdata-id3-heavy/run-synthetic.py as paired.py. No binaries or private
sample/project records retained.
