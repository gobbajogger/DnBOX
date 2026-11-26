# Synth One full-rate RevSC lifting with finer block scales

Same source room topology, modulation/addresses, rates, coefficients, output
and authored preset words. Replace only history representation as documented
in ../pair-lifting/README.md. This version uses one byte per band per packet
to encode ceil(peak/signed-limit) over four intervals per power-of-two
exponent (six exponent bits with offset 48, two fractional bits). Zero is
reserved. No dynamic predictor/filter states outside each packet. Source
control ranges are not changed. Clipping/zero statistics are available in
the same-input room trace; no clipping in the measured Organ formats.
Neither codec is lossless or target-qualified.

The 11+6/16 cubic variant uses 29,040 B of per-owner rings and staging
+440 B for the host RevSC state (29,480 B, below 29 KiB). On original
Organ same-input stem the stereo spectra shift .172/.190 dB and the whole
waveform residual is −66.991 dBr. All original read-address trajectories
were byte-identical. Eight complete authored host holdouts meet the
1 dB windows / 3 dB last-second spectrum / −20 dBr whole-wave bounds
(compare donor and genuine original PCM hashes before substituting history).
On the independently stratified 32-cue suite, however, a new last-second
left-channel spectral mismatch emerges on Pulsating Ultraworlds:
3.529 dB vs .004 dB with unmodified donor, despite −52 dBr whole.
WSW Keys Ever So's ~12 dB late discrepancy was already present with the
exact donor; it is not a codec-induced regression or a waiver. This
candidate cannot be called a whole release result. Consult the recorded
unrounded output; do not score isolate-only metrics as adoption.

This 29,480 B is a single room figure; the product has two independently
renderable room owners and only 2,100 B per owner already reserved. Do not
reuse unchecked UI, delay or SuperJ6 memory. Even granting the earlier
possible 26,880 B canvas once (not currently reserved or demonstrated), two
room histories in this format cannot be paid for by that canvas. The scalar
history pack remains CPU-unqualified (packet bursts and multiple reads).
There are zero changes to the production tree.

Replay from workspace root:
```sh
D=s1-organ-wet-recovery/lifting-e6m2
bash "$D/screen.sh" > /tmp/lift-fine-screen.json
cmp "$D/screen.json" /tmp/lift-fine-screen.json
bash "$D/donor.sh" > /tmp/lift-fine-donor.json
cmp "$D/donor.json" /tmp/lift-fine-donor.json
bash "$D/extended.sh" 11-6-16-cubic > /tmp/lift-fine-extended.json
cmp "$D/extended-11-6-16-cubic.json" /tmp/lift-fine-extended.json
(cd "$D" && sha256sum -c SHA256SUMS)
```
