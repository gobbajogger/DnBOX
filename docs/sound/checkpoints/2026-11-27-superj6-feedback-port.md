# Recovered SuperJ6 feedback and compact math comparison

The reconstructed host oracle in the adjacent reference file reproduced 67830 float output words (22610 per cue) exactly against the authentic post-OTA observation, spanning k=3.15 live, k=3.90 live, and k=4.12 cold high at an 8510.413 Hz cutoff. All cues included the return to zero, with no filter-history reset. The cold high path seeds the real four poles and sets a latch; clearing it at zero must not clear the states. This does not verify a warm entry across the high knee.

A separate approximation removes hardware FMA dependence and calls current firmware fastmath functions for tanh and sqrt. On the same captured input and physical automation, its complete-window residuals were −147.12 / −147.15 / −111.20 dBr. The high case is not bit-exact (11961/22610 different output words), but the worst scored high held window was −112.85 dBr. Standard libm without FMA scored −147.38 / −147.38 / −107.42 dBr. Error is matched-input post-OTA, not a whole-engine or all-preset result. All outputs finite.

The exact reference standalone PI32 translation unit is 1036 B text vs 582 B for the zero-feedback demonstration, with unresolved libm dependencies. Do not interpret this as integrated size/cycle cost. The no-FMA/fastmath target's native standalone timing is pending. It still needs relevant mixed/holdout routing, actual modulation/window lifecycle, clean fallback, and paired whole-engine/board builds before any promotion.

Source-only archived proof, no original assets/app/PCM, Library SHA256 cbedee8c80688f636a1886805f9ad7a2d758524d5124d5f907594505dc544638.

The unequal Raw and Brainwave Main/Sub level and release differences are not closed. The original common resonance gain .867 on Brainwave cannot explain those unequal levels. Keep master and fixed role corrections ruled out.
