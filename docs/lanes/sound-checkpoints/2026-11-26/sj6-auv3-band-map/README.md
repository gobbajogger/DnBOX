# Installed SuperJ6 Sine/Triangle pitch-band reference

Twenty-six direct offline captures from the exact official installed current
Mac AU. Same Jupiter-derived no-effects calibration, fixed velocity87,
Vintage zero, authentic OSC2 Sine/Tri switches, full raw Normal cutoff
20 kHz, zero tracking. Two shapes, keys A2 to A8 and six intermediate
blend-position probes, with A4 repeats. Native preset/controls, signed PCM,
and event/clock receipts are included. No original application or
embedded original bank is included.

Run `python3 score_band_map.py native` with NumPy. It measures exact-clock
harmonic magnitudes and oscillator-origin phase, as well as phases
relative to the corresponding multiple of the measured fundamental.
Do not use the latter as permission to align whole waveforms. The
unblended/blended band labels come from the independently traced
original standalone selector; no private loaded AU state was tapped.
Fundamental magnitude varies slightly across fresh instances, so these
are stage harmonic descriptors, not a fixed whole-preset gain target.

Across lower and middle keys the relative harmonic magnitudes settle
near a common shape for each waveform (Sine h2/h3 around -30.67/-29.87 dB,
Tri -22.33/-19.14), with additional harmonics dropping at the upper
band boundaries. Retain a single stage-level shape hypothesis and use
unused keys/bends as holdouts rather than tune each note or patch.
Absolute installed gain mismatch remains an independent blocker.

A separate deembedding calculation uses only the authenticated Normal20k/Q1,
HP10, active zero-amount Crusher DC recurrence and terminal .999 DC recurrence.
Fixed-gain nodes cancel. Compile filter_transfer.c against the verified Normal
receiver headers as in the prior cutoff study, then run
`python3 deembed_shape.py scored.json filter_transfer.dylib`. A4 relative H2/H3
magnitudes and phases agree with the separate genuine standalone pre-Normal
Sine/Tri taps to measurement precision. This validates the scoped amplitude/phase
transfer for relative harmonics, not the 7 dB absolute level. The file includes
measured spectral descriptors only, no original embedded table contents.
