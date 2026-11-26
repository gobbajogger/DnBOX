# OTA history layout experiment with binary24 storage

Same 52-byte physical Main leaf with separate arithmetic and one shared
normalization per frame. At each first-order all-pass history store, pack
three bytes representing sign, full binary32 exponent, and the retained
15 fraction bits, rounded nearest/even. Do not change the calculation
from Float32, gain, phase, coefficients, physical conditioner, or its
four nonlinear pole words. No native half-float capability assumed.
This finite-input laboratory packing is not yet an admitted receiver.

The independently executable run.sh reproduces two layouts:
packing the four expansion words only 52→48 B; packing all eight
expansion and contraction words 52→44 B. On the same 128 synthetic
profile/input combinations (1,048,576 words per arm), the worst
signed output was −107.09 and −103.75 dBr, respectively, relative
to the unpacked 52 B separate-arithmetic model. The narrower IEEE
binary16 comparison (16-bit storage on only two states) loses much
more on quiet inputs (−20.86 dBr; −62.29 with fixed 2⁸ state scaling).
This is not a claim about half-float or every imaginable packed layout.

A literal 44 B ×6 fits the sum of the existing 264 B reserved gaps in
one owner, but those gaps are noncontiguous. A real layout needs
explicit disjoint state fields and allocator/lifecycle proof. No pool,
flash, active motion guarantees, target time, original-cue sound
result, or product enablement follows from this guest host test.
