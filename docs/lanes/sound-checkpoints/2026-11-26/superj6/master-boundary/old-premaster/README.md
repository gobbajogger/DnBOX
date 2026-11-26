# Older authorized SuperJ6 shared gain, two independent measured seams

```sh
export PYTHONPATH=/tmp/sj6-normal/packages:$PWD/ab-effects-readonly/sj6-fullbench/bench
export SJ6_BIN=/tmp/sj6-normal/old.private
python sj6-old-premaster/probe.py
python sj6-old-premaster/sender.py
```

The scripts pin the original old ARM64 binary SHA. No private binary, authored
preset library, or audio is in this pack. `probe.py` drives the actual old
AKBooster DSP ctor and actual process with 514 signed stereo samples including
values above unity and asymmetric gains. Each fixed arm is bit-exact against
one Float32 multiply, including the stopped identity and muted zero controls.
The live asymmetric 31/86/140 callback arm also reaches the exact settled
multiply after preserving its nontrivial initial ramp. No factor of four,
headroom normalization, crossfeed, or clipping at this old DSP leaf.

`sender.py` executes the actual old app master callback at `0x1000ecef8` and the
real Swift AKMixer setter `0x1004667d4`; only the final AVAudioMixer public API
is intercepted. It receives exactly `Float32(Double(raw)*.404)` throughout,
including Brainwave .3150142729282379 -> .12726576626300812, mute, and >1 raw.
No current Mac PCM or real Apple mixer graph is synthesized by this capture.

Together these falsify a hidden divide/multiply *inside these exact old
shared stages*. They do not prove the current Mac graph, its full level
routing, mixer behavior under all state, or explain the measured absolute
discrepancy. A physical internal newer AU seam remains necessary to claim a
specific shared mismatch. Do not add gain to match the final level ratios.
