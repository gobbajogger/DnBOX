# Full 52B nonlinear physical equation, paired dependency schedule
Run bash sj6-ota-bare-physical-pair/run.sh from the pinned workspace. Source pin 703d2534814e4169b1f801cf1dc026317bffcbdda209ad9c0647d239f2b97ea0, extracted leaves verified against archive. Unused host wrapper removed; both mapped to same donor fm_tanhf inline and compiled pi32v2 r3 -Os -ffp-contract=off. 512 frames, independent pregenerated lanes, synthetic static g=.17, 52 B owner, full per-sample nonlinear correction in both arms, no prep/tangent substitution. Output hash after measured section. No device or firmware build.

Active voices | serial physical ticks | paired physical ticks | same hash
---|---:|---:|---
1 | 40605 | 33089 | 08e73264
4 | 158764 | 129771 | ecfc96f4
6 | 237539 | 194227 | c9f4b548

TIMER4 24 MHz, strict emulation no skips. Reductions 18.51%, 18.26%, 18.23%; six voices 19.331 ->15.806 us/frame. Previous separately measured compact stock 5.230 us/frame, remaining 10.576 us/frame above stock (about 5.415 ms per 512 measured frames). Small baseline timing change vs prior exact source fixture (~30 ticks) due different bare layout; same hashes. Relevant bare symbols: baseline 304 B caller +402 B clock_2x; candidate 1044 B caller with paired clock inlined, +338 B. No signed cue, real Sub, load, DMA, scheduling, product APP or fit claim.
