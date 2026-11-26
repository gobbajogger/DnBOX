At the same size, deterministic first-difference quantization error
shaping on the odd residual or even anchor did not generalize. Both were
tested as separate whole-program candidate builds with the same 32 source
PCM/event hashes; raw reports remain in ../lifting-shaped and
../lifting-shaped-low. Unshaped only adds one late spectral issue to exact
donor (Pulsating Ultraworlds L 3.529 dB). High-only adds two
(Ultraworlds 4.153, Synthwave 3.318), low-only adds three (Ultraworlds
4.594, Synthwave 3.728, Moogy Glide 3.674). Each still has the existing
Ever So scope difference. None should be shipped.
 
A separate 16-bit boundary singleton removed extrapolation from the last
odd of each tile at cost of two bytes per packet; at 29,040 B the best tested
11+5/16 still missed Synthwave (3.458 dB). See ../lifting-boundary/frontier.json.
