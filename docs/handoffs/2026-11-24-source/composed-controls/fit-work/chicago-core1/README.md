# Guarded single-Chicago two-core scheduling trial
 
Apply `single-chicago-on-core1.patch` to the pinned all-controls-fit-s1-wheel-noise
diagnostic source. This patch only changes the jobs that the existing planner can
assign to the second core. A single Chicago part is eligible; two Chicago parts
are kept serial because their current filter cache is not safe for simultaneous
writers. DSP arithmetic, transport policy, BLE, stack boundaries, and flash
geometry were not changed.
 
Native fit: Elite 216 B APP free, Pro 224 B, both 464 B above the RAM pool
reserve. Audio and independent virtual cold guards pass on both; idle is not
included below. Both fitted before loaded replay. Native stack canaries show
3,348 B used on each at offsets 0 and 1, or 748 B above the conservative 4,096 B
usable floor. BLE remained active. A separate profile witnessed Chicago on the
second core; do not substitute profiled PCM for the strict gate.
 
| Physical event offset (ms) | Elite µs | Pro µs | Dropouts, Elite / Pro |
|---|---:|---:|---:|
| 0 | 10,815 | 10,812 | 0 / 0 |
| 1 | 11,080 | 11,086 | 0 / 0 |
| 2 | 11,190 | 11,072 | 0 / 0 |
| 3 | 10,757 | 10,803 | 0 / 0 |
| 4 | 10,811 | 11,195 | 0 / 0 |
| 5 | 10,839 | 11,072 | 0 / 0 |
 
All 12 loaded runs had active PCM and clear guards. Worst block 11,195 us
remains 286 us above target 10,909. The slower comparator lost DMA halves,
and final project/hit states can differ under the physical control script;
this does not prove PCM equivalence of a fixed musical command stream. Not a
release pass. Still qualify event/input-response equivalence, dual-Chicago and
the storage combination before promotion.
 
See ROOT-CAUSE.md for matched passive profiling on the previous image.
The patch is standalone. Use the same existing checked paired fit / seed projection
and `heavy_pinned_board.py` with `SHIFTS=-1,0,1,2,3,4,5` for the strict replay.
