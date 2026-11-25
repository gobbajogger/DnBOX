# Subband filter-cost ablation

Same static 250 ms subband probe as the first prototype, same cutoffs,
windows, codecs and clock; only FIR length changed. Analysis and synthesis
both include 4 paths (2 bands x 2 channels). Nominal weighted products
per stereo frame average 8 taps / 16. This is arithmetic counting, not
emulated time. The calibrated comparison uses exact carrier and out-of-band
components referenced to the Build9 C oracle, and 0.499 feedback.

321 taps: 160.5 products/frame; 3 kHz first/fourth carrier -1.377/-5.498 dB,
strongest first out-of-band return -50.749 dB.
193 taps: 96.5; carrier -3.942/-15.774 dB; spur -35.406 dB.
129 taps: 64.5; carrier -5.780/-22.727; spur -22.696 dB.
33 taps: 16.5; first carrier improves to -1.126 dB but first spur is
-3.129 dB, and fourth carrier/spur rise +7.824/+6.018 dB above the original
fourth carrier. The short filter is not close just because the first
carrier is strong. None is an adoption candidate. Changing the filter
structure, rather than just making this FIR shorter, is the next useful test.

Run sweep.py beside the prior sj6-delay-subband folder and with its same
probe.py and Build9 C/Python reference in the working directory. The prior
compiled stock fixture is required by probe.py. No render binaries included.
