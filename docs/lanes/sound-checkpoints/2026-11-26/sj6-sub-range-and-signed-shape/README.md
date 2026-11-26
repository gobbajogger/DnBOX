# Sub range correction and aligned signed source diagnosis

The current typed SuperJ6 candidate used the untransposed note when
selecting the Sub sample-zone surrogate and calculating its octave-down
pitch. The verified original Brainwave Low-Range cue sends incoming MIDI
48 to 36 before selecting the Sub zone. The narrow host candidate does
the same; the measured terminal held pitch moves from about 66 Hz to
32.761 Hz, versus 32.690 Hz on the original. Center, both isolated Main
paths, and Solidify (Sub off) are bit-identical to the compact baseline.
No master or per-preset adjustments. Host high-range is only a synthetic
guard; out-of-bounds policy and native link/commit remain unqualified.

Together with the original cue and repeat, the corrected signed samples
show separate reasons this does **not** fix the Sub's sound. The H1 is
25.80 dB lower; H3/H5/H7, relative to H1, are −14.09/−19.26/−26.75 dBc
rather than −10.24/−16.16/−19.69 dBc. The odd spectrum and its relative
phases disagree even after allowing for a uniform gain or delay. The
first 128 samples have signed correlation +.9997, then the 128–512 window
goes −.554 and established held 8,192–22,050 goes −.832, no shifted
or polarity-adjusted comparisons. The original's first measured
zero crossing is near frame 234 from note-on, compact near 724; the
original already has active onset-specific sample behavior. The signed
original repeat residual is −50.37 dBr, much smaller than the compact
difference. This is a terminal-output witness, not the private in-app
sampler tap; capture other signed notes before choosing any per-zone
analytic representation. Do not copy installed sample arrays.

A 12-zone fresh-note direct AUv3 capture is compiled and queued but its
reference output is not included because the Mac was locked. The new
fixture source is here. Everything in compact-generated is original
synthesis from the compact host, not a proprietary asset.

```sh
python3 native-sub-shape.py ../sj6-direct-auv3-fanin \
  --compact compact-generated/range-corrected-sub.f32le
```
