# Static Normal20k/Q1, stacked after staged AMP ingress

Independent authored/dry A/B of only one authentic physical Normal branch at
44.1 kHz, latest imported signed readbacks. No device, Git, output gain, FX
reorder, timing or phase shift. Typed source is untouched except for
diagnostic effect of exact Normal recurrence after prior per-layer AMP ingress.
Original generator coefficients use the verified current physical 20000/Q1
receipt, not legacy tracked A3/A4 fixtures. The stage uses F64 FMA with each
sample narrowed before next history, in source order. Existing four histories
are reused only in this cold, fixed-control/no-switch fixture. Do not infer
live mode / cutoff transition or copy this into portable firmware. General
input law, coefficient ownership, live tracking and source-type modes differ.

Supported Solidify on this exact trajectory: signed compact delta −39.261 dBr
for dry and authored, whole-level moves +.001674 dB against original, whole
native spectral RMS improves .0004 dB. Other two preset/FX pairs remain byte-
identical to previous AMP-stage candidate because mode or physical conditions
are not admitted. No material improvement to held parity, no adoption.

Use prior donor + amp-ingress.py first. Pass that copied source and a new
isolated output path to normal-static.py. Export with the prior signed capture
package and authenticated original input archive. Exact source hashes and wet/
dry PCM hashes in .pin/.sha and manifest, no PCM is bundled.
