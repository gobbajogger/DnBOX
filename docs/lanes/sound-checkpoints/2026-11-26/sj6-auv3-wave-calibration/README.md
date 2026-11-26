# SuperJ6 installed AU, shape-matched compressor-inlet comparison

The current installed SuperJ6 Audio Unit rendered 42 waves and paired compressor-on/off captures: Triangle, Saw and 50%-nominal Pulse, verified real public waveform detents, six fader levels and a reversed-order midpoint repeat per shape. All use Jupiter Sine as the common preset base, change only scoped oscillator/diagnostic controls, and use 44.1 kHz signed stereo with exact callback partitions. No user settings, instrument or effect binaries are in these artifacts. See Git b273ff6e for the earlier separate Sine experiment and original-component pin.

The Apple bare calibration uses each shape's actual, same-machine compressor-off waveform as a signed multiharmonic proxy. Normalization is to express the measured bare transfer in actual peak units, not a gain correction, and the scorer maps back to physical RMS using that shape's measured peak/RMS ratio. The bare high-range Saw test extends the .90 peak ceiling to 1.30, with exact bypass and repeat witnesses. Both RMS and the first harmonic must pass the training and held-out points in both channels and held windows; h3/h5 are additionally reported. This compares *relative* compressor-on/off response; no fitted downstream gain is used to force a pass. The proxy does include the known post-compressor part of the loaded path and is not a direct internal tap.

Inferred stage signal: Triangle .7340–.7952 peak, Saw .9163–.9927, Pulse .6215–.6733. Against the source-authenticated standalone current oscillator/Normal replay plus conditional HP10/Crusher unity, their inferred RMS excesses are +6.69–7.39, +6.79–7.49 and +7.00–7.69 dB, respectively. The previous Sine covers the same common level region. Implied downstream linear factor is .147–.160 across all shapes, including the explicitly traced .1532 original master path. This is evidence for a shared upstream cause; multiple waveform-specific gains are not needed to explain these controlled cues. It neither identifies the original node nor authorizes a fitted gain or full-AU parity claim.

To reproduce scoring, extract all three per-wave archives into the same parent directory. They contain `native/`, and the Saw archive additionally contains `extended/`. Run `python3 score_waves.py native extended` with NumPy. Per-wave archives retain the exact native AU and bare processor PCM hashes. Scripts extend the preceding direct AUv3 bench, not a production instrument or firmware. No physical device writes or firmware release.

Git keeps the 42 genuine installed-AU reference renders and all calibration manifests. Bare Apple calibration PCM is separate in the complete Library per-wave archives (hashes below). It is needed for reproducing the full inverse score from scratch, not for using or hearing the native reference renders.

68b221f4d70746d3f500170f190c7e8ad5adfc9c0ada6e56c178e0ec3e61de57  pulse-reference.tgz
ef6482370cb7e0f9529cec6b87795ef9bd8be9762c2cc87213a47cd8bc94e60f  saw-reference.tgz
5384ea63f680587b5ff6d4e50d1b5b4fde8b28b4289d1260f7e319af22a82334  tri-reference.tgz

Additional full AU compressor-off harmonic fingerprints from A4 held frames
25000 through 54999 (origin frame 16384): relative to h1, measured h2/h3
are Sine -30.65/-29.85 dB, Triangle -22.31/-19.12, Saw -6.00/-9.52 and
Pulse -106.96/-9.52. Agreement of common gain intervals is not a null test
or evidence that Sine/Triangle partials already match isolated source.

`shape_drive.py native ../sj6-auv3-compressor-inlet/native` rechecks the
same-clock compressor-off levels and h2–h6 from the four oscillator sweeps.
The full-to-half fader changes true level by about 5.81–6.23 dB, while the
relative Sine/Triangle h2 and h3 remain the same on every fader, including
midpoint repeats. This is inconsistent with simple drive-dependent
quadratic/cubic coloration after that fader, not proof of the source node.
Do not remove the Sine/Triangle harmonics when a full-pipeline reference
contains them or use amplitude fitting as a sound fix.
