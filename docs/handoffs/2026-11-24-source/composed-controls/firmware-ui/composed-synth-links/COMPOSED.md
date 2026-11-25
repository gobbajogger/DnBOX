# Composed LINK / native typed Remote evidence

Start with the guarded native DNR3, exact Chicago table compression and four
played-note DSP savings from Nightly 91103fa4. Apply the three scoped
Synth One LINK patches from DnBOX 449557ac. The narrow port applies at zero
fuzz to the old DNR3 donor. The first native link triggered the ROM guard:
the PI32 compiler emitted 0xffc000f8 as a scalar forbidden-bit mask. It was
not used as an address. scalar-mask.patch checks exactly the same forbidden
bits in the serialized bytes so the machine code does not materialize the
ROM-range immediate. No audit/guard was weakened. Verified by disjoint
bit decomposition and 327,680 representative exhaustive-byte fixtures.

Both links pass the original checks with 3,628 / 3,640 B free XIP and 512 B
pool margin. The combined host test confirms both S1 slots and unlink/save
audio equivalence. Both guarded native USB played-note cutoff tests passed
with 0 underruns and 0 late; 10,887 and 10,877 us still miss the new bound
(10,809) on this narrow source. The loaded gate is the main remaining
performance blocker, not this 68–78 us smoke difference.

SUPERJ6: The earlier authenticated ADSR panel port and approved shared glyph
passed 322 host assertions on both slots (see RECEIPT.md). It also applies
to the combined native source without fuzz, but enabling the historical
full SuperJ6 provider on the combined firmware exceeds APP by 4,380 B Elite
and 4,368 B Pro. Neither the port nor the current full bank/controls/UI have
a production pass. No assets, binary firmware or raw recordings are
included and nothing was deployed.
