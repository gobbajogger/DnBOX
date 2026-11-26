# Installed SuperJ6: localize extra Sine/Triangle harmonics before Normal filter

Direct current installed Mac AUv3, same Jupiter base, single A4, 44.1 kHz, real
OSC2-only Sine or Triangle, fixed velocity, Vintage 0, all other neutral
effects fixed. Original public Normal cutoff compares raw .45/.50/.625/.75/1.
Compiler source replay into original generator binds these to 424/599/1967/4743/
20000 Hz at damping 1 on this exact zero-tracking, zero-envelope context.
No raw binaries, preset data, device writes or released source changes.

Measured change in h2/h3 *relative to h1*, including their phase relative
to an integer multiple of the fundamental phase, follows current Normal
transfer over all useful cutoff positions. For example at .50 Sine
changes -6.2496/-11.6379 dB vs calculated -6.2501/-11.6378, with <.003 degree
phase error; Triangle -6.2497/-11.6379. Across the two channels and four
nontrivial cuts first three ratio residuals are about .001 dB, and phase
about .004 degrees. Independent repeats confirm harmonic ratios despite
up to .414 dB non-repeatability in absolute fundamental. Thus a coherent
source of these extra harmonics is present **before** the original Normal
filter in the loaded graph, not added by final output/compressor. This does
not identify the before-filter original node, quantify its absolute gain in
these takes, or reproduce the effect by tuning a patch.

For scored output compile filter_transfer.c (uses the verified
sj6-normal-arithmetic-leaf/normal_coeff.h and normal_receiver.h) as a
macOS library, then run score_harmonic_cutoff.py native filter_transfer.dylib.
The separate sine_velocity.py / velocity.json use the previous genuine
Jupiter velocity calibration with authored Vintage, preserved under
Git 36722b9 and Library. That independent run shows a small nonmonotonic
harmonic amplitude change with actual input velocity; it is not a matched
Vintage-zero treatment. The new native comparison included no mono/other
gain changes; the gain calibration and full release requirements remain
separate.
