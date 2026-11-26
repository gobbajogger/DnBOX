# Direct SuperJ6 AUv3: cutoff timing around note ingress

Both profiles were submitted to the same already-running Mac bench without a
new app launch. The Brainwave chord starts at frame 49,459. The test arms open
physical cutoff #7 at 49,331 or 49,587, respectively, using identical buffer
splits at both frames. Both restore the authored value at 70,000. Each arm
and its no-change control have independent repeats. Only #7 changes.

By frames 54,000–69,872, the before-onset write has converged to the no-op
control. The after-onset write persists: −2.659 dB level difference and
−1.892 dBr waveform residual versus the before-onset arm. Its repeat residual
is −38.242 dBr. Selected spectral peaks change by −1.02 to −8.76 dB.
This makes cutoff timing a contributor to tone, but does not establish
that it explains the full compact-engine residual.

Run `python3 score.py .` with NumPy from this folder. It validates both
original identities, exact MIDI and splits, step readbacks, checksums,
and all eight captures. All eight are also in ChatGPT Library.
