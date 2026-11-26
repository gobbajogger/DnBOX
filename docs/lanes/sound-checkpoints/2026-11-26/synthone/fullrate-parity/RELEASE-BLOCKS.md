# Synth One: source arithmetic and a false release mismatch
The exact-source host candidate uses both original note events and common project tuning.
No fitted correction or per-preset audio edit is applied. Match the original float
overloads for bitcrusher rate and double-intermediate ADSR arithmetic before measuring
the stereo bank. The fixed bitcrusher alone took the two largest affected cases from
about 7% RMS residual to below 0.004%.
 
The original sends the entire 66,150-frame tail to its 128-frame chunking loop.
The compact test driver sent two separate 33,075-frame spans. The extra split fell
53 frames into a block, shifting voice retirement checks. In Digital Ocean (527) and
Sci Fi Drama (90), matched voice states diverged exactly at original release retirement
on frames 243634 and 232754 (including 128 lead-in frames). Matching the single
tail span fixes test timing without changing engine release behavior.
 
A fresh independent 906-row comparison preserves every reference stereo hash.
All 906 meet -40 dBr on both channels; 891 meet -60 dBr and 41 have byte-identical
whole PCM. The worst is Arcade Lead (-47.564 dBr, about 0.42% RMS). The maximum
absolute whole-render level delta is 0.001702 dB. This is six-note stereo host parity
at matching settings, not firmware fit, live arp parity, or release qualification.
See bank-releasephase-summary.json and summarize-source-arithmetic.py. Reproduce
with exact pitch, exact float crusher and exact envelope together (see build flags).
