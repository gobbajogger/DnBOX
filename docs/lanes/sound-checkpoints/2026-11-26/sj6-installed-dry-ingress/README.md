# Solidify captured dry host write order

Run `python3 check.py SJ6-Three-Factory-Presets-original-AUv3.tgz`.
The archived original runner waits for preset load, reads its author state,
then explicitly writes the dry effects, ending with Crusher #107.
Solidify's bank loaded with #107=1. The explicit last write drove it to
zero; #108 remained ~.03187546 and #109 remained zero in the public
before/after readback. There is no subsequent write to either field in
the saved sender. This is the scope for saved dry PCM 76bdf841…; the
bank-load control order is not the dry capture invocation order.

The [executed original receiver](../sj6-crusher-physical-receiver/README.md)
shows #107=0 writes zero physical wet and unity Booster. In a *different*
ordering where #108 writes afterward, its nonzero latent field would
set ~0.5691% wet; that inference must not be applied to the explicit dry
capture. No capture taps the physical Apple parameter after the installed
AU handles it, so hidden graph writers, connectivity, and an exact
insertion point for the suffix remain separate verification problems.
No input assets or audio stored here.
