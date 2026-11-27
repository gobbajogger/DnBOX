# Eventless ARP headroom checkpoint

Development only; not an installer release or the full Part Sound branch. Build with `X0X_UI_SMALL_CANVAS=1 X0X_CHICAGO_FOLD_RESIDUAL_MIN=1 DNBOX_LAB_D9_SUFFIX=1 X0X_D9_TANH_BOOT=1 DNBOX_LAB_ARP_EVENTLESS=1 MODEL=elite|pro bash build.sh` using the native JieLi dependencies.

The eventless ARP path first preserves the previous transport rephase. It refuses any held-zero transition, rate mismatch, accumulator wrap, or beat/gate crossing at or before the final sample. It advances only the already-equivalent fixed clock, previous held-count and saturating age, and calls the same backend once for the same span. All events stay on the original scalar path. No persistent RAM added.

Host: 500,000 exact state/event comparisons. Running both real compact synths, 233,757 signed stereo frames per engine and 65 note-on/65 note-off events per engine matched exactly, including event times. Native PI32 leaf: a qualifying 256-frame window saved about 192 µs, and the worst measured rejected-path increment was under 0.22 µs per span; this is isolated, not a whole-replay attribution.

| Offset | Elite peak block | Pro peak block | Late / XRUN |
| ---: | ---: | ---: | ---: |
| 0 ms | 10,729 µs | 10,630 µs | 0 / 0 both |
| 2 ms | 10,448 µs | 10,455 µs | 0 / 0 both |
| 5 ms | 10,689 µs | 10,344 µs | 0 / 0 both |

All six pinned initial runs meet 10,809 µs; worst has 80 µs to this stricter target. Exact 922-event strict 360 MHz native emulator, no skipped cycles, unchanged simulated NOR, same preset 518/ARP/delay/Room and Chicago B. Offsets 1, 3 and 4 and full Part Sound composition are separate verification work. Native packages fit with 4,228/4,240 B XIP and 22,732 B shared pool margin after reserve. The previous less RAM-intensive checkpoint remains a separate option.

Package SHA-256:
```
8ec2f3d3724eb91dd6e5cb247969cf767a9b125d55bb2bc548bad9ffd6c61d7a  x0x-smk37-elite.fwsc
54279805bdebc6830befb9cb866cc518459df4c96d9bd1e616a8b5f027886621  x0x-smk37.fwsc
```
