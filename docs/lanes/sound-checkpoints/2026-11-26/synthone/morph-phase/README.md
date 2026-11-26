# Target-friendly ties-to-even phase rounding
Remove the standalone lrintf link dependency with exact binary32 integer
rounding of finite inputs to the low 24 phase bits, for the existing
round-to-nearest/ties-to-even law. No clamping or phase-fit adjustment.
55,816,544 signed binary32 samples and explicit quarter-grid ties match
lrintf exactly; 270,063 original morph ticks preserve phase and sound
receipts. The optional identical lower-bound search keeps inclusive band
edges and falls back as before for non-comparable frequencies.
