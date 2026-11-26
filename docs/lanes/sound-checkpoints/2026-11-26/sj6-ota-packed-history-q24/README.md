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

The follow-up run-layout.sh implements a split layout without traversing
the boundaries of its arrays. A has six 28 B AP+condition records, three
4-word pole arrays and the last voice's pole zero. B has two 4-word pole
arrays. C holds only the last voice's three remaining pole words. The
last four-word recurrence uses a local array, with explicit copyback.
Layout sizes are exactly 220/32/12 B by C static assertion. In 180,000
host transactions across two independent owners, all outputs and active
state words match the contiguous 44 B packed baseline. 86,865 frozen
valid arms and 5,951 out-of-range owner/voice arms preserve data and return
signed negative zero. This verifies the layout logic only, not that the
actual native linked offsets, all owners, controls, extra coefficient
descriptors or real target timing qualify. No pitch state borrowing.

The two signed original-receiver input profiles were also exercised by
a scalar translation of both packing forms; the independent original
receiver was actual compiled code. In the four Raw/Main pitches at two
cutoffs, packing both directions gave a worst in-cue −104.98 dBr vs
translated unpacked, and −100.23 dBr on a separate 4,096-zero-frame
continuation. That continuation is not a claim of indefinite tail
stability. Authenticated replay requires the same existing private
receiver benches; auth-scalar.py contains the exact scoped replay
recipe, not compiled candidate C or target evidence.

An optional aligned-storage variant (run-aligned.sh) packs four Q24
words in three named aligned integers and updates each independent
group together, with no unaligned or crossing accesses. It reproduces
the earlier contiguous three-byte implementation on all 1,048,576
signed synthetic output samples. It is a source-only option if the
current independently benchmarked shared helper later fails the fit;
no native savings claimed and it is not required by the layout proof.
