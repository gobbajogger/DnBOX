# SuperJ6 #203/#204 authentic performance pad assignments

Build9 ARM64 SHA 6d163d79… Swift metadata, actual registration/callback ivars,
RawRepresentable witness, and separately executed original setters match.
These are two performance XY pads, not firmware step pads, note-gate slots,
or arbitrary param IDs. PadController has separate touchPad1/2 and X/Y
instrument parameter slots; PadType cases map raw 0..9:
arp, delay, filter, lfo1, lfo2, lfo3, reverb, oscPitch, tremolo, unison.
RawRepresentable initializer clamps invalid integers to enum no-value
tag 10; raw getter literally returns the case byte. AU setters truncate valid
finite float then independently write pad1Type / pad2Type. Representable
integer cases preserve themselves; out-of-range examples use independent
fallbacks pad1=LFO2 (4), pad2=Filter (2), not shared clamps.
Original non-finite/unsafe huge input traps were not exercised.

#203 registration +c10 → callback .274cf0 → .f160c → independent +.390 ivar.
#204 registration +c18 → callback .274da4 → .f1684 → independent +.398 ivar.
Post-preset load lock branch reads lockPadTypes at +.3a0; lock=1 restores
oldPad1Type (+.3a8) through the first control and oldPad2Type (+.3b0) through
the second. Branch skip on unlocked matches the embedded 'Keep pad
destinations when loading another preset' UI meaning. This lock branch is
statically verified; full live GUI load with lock toggled was not executed.

All 640 actual bank entries have both legal original codes, with 640
distinct within-preset pairs and 630 pairs with both nonzero. Three
archived genuine installed loaded before/after readbacks match the exact
raw bank UID: LFO1/Reverb, LFO1/Filter, LFO1/Filter. Both differing
pairs plus independent test ranges were replayed through original CPU
setters without conflating writes. The PCM checks establish custody
for the real captures, not their response to XY gestures. No source
claim covering destination-specific X/Y physical ranges, live touching,
portable UI or firmware adoption. No Mac or firmware writes.

Replay: python3 check.py PINNED_ARM64 ORIGINAL_FACTORY_ZIP SIGNED_ARCHIVE
with existing macho.py, NumPy, Capstone, Unicorn on the workspace.
