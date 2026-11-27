# Tap the compact dry oscillator bus before effects

Run bash run.sh with the absolute guarded firmware-fit-next directory.
Uses the previously authenticated native Init cue and compact velocity mode 3.
Adds a host observation immediately after juno6_sj6_render_osc_bus, before any FX
or terminal master. The compact 132300-frame terminal repeats the prior
unaltered render and matches the observed stereo source bus sample for sample.
For this dry cue the compact post-bus path adds no attenuation. This does not
measure the original's internal bus, clear its Apple stages, or explain the
remaining 13.07 dB difference; work upstream on the original and compact
voice/receiver/bus signal and compare exact inputs before changing production.
