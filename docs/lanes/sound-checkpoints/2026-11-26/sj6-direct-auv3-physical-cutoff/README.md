# Direct SuperJ6 AUv3: physical cutoff A/B

Four direct renders from the Mac original 0.999.8 build 9 compare the authored
normalized cutoff 0.3345831036567688 to 1, with independent repeats. Only AU
parameter address #7 (`neoncutoffLayer1`) differs in the live control maps.
Address #129 (`neonfilterCutoff`) stays at 1 in both arms and is a separate
property. The dry OSC2-only cue, exact MIDI schedule, modulation-off controls,
buffer boundaries, authored master and resonance are held fixed.

- Attack (first 2,048 frames): −0.360 dB level difference, −3.81 dBr sample
  difference; both repeats are below −96 dBr.
- Held: −0.000017 dB level difference, −79.98 dBr sample difference, within
  repeat variability; the first six harmonic levels change <0.000032 dB.
- The selected chord-commit window differs by −99.84 dBr.

This proves the physical cutoff control changes the attack on this cue,
but does not explain the sustained gain or harmonic shape mismatch. It
does not show which runtime control or signal-path behavior causes the
effect to converge during the held note.

Run `python3 score_cutoff.py` with NumPy installed from this folder.
The four reference files are also backed up in ChatGPT Library.
