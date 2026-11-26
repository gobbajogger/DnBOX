# Source-derived smaller physical dry suffix

Compared against the pinned native Apple HP10 and connected-zero
Crusher signed PCM. This is a leaf host proof, not integrated firmware
or an assertion that both filters already belong at the same place
through every active effect.

Map the previous validated increment HP10 denominator back through the
bilinear transform and use two state-variable integrator words instead
of four delay histories per channel. Compute state advances as small
stable increments, rather than subtracting nearly equal long histories.
For each DC H(z)=(1-z^-1)/(1-p*z^-1), a single low-pass estimate gives
immediate output x-s; its state update is s+=(1-p)*(x-s). No fitted
gain, inputs, phase, or preset constants.

The physical HP + connected Crusher pair matches the two signed
authenticated complete datasets at −112.68/−112.70 dBr. Isolated
Crusher is about −124 to −128 dBr, verified PCM hashes and exact
regular/irregular partition equivalence. Final .999 uses the same
mathematical form but no independent native phase/library claim beyond
the direct terminal physics receipts. Layout uses 16 B/channel,
32 B/stereo owner, 64 B for two independent owners, before any
control metadata. No history overlay and no native XIP/cycle/fit claim.
Actual production admission requires exact mode/lifecycle, enable/disable
reset scope, instrument controls, and paired memory and timing proof.

```sh
python3 coeff.py numeric.h
cc -std=c11 -O2 -ffp-contract=off -shared -fPIC mini.c -o mini.dylib
python3 score.py ./mini.dylib ../sj6-portable-hp-zero-crusher
```
