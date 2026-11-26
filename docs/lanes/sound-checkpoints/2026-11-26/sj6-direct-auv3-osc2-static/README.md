# Direct SuperJ6 AUv3: OSC2 with AU cutoff #129 fixed

This bench isolates the second oscillator on the official Mac AUv3
0.999.8 build 9, disables filter modulation and pitch modulation, and
sets AU cutoff #129 to its maximum while retaining authored resonance and
master. **Correction:** #129 does not open the physical filter for this
cue; the separate physical cutoff #7 stayed at its authored value. See
the [#7-only native experiment](../sj6-direct-auv3-physical-cutoff/) for
the controlled physical-cutoff result. MIDI, buffer boundaries, input identity and dry-route controls
are pinned to the fan-in experiment. Rendered twice at 44.1 kHz without
a DAW or standalone instrument. No firmware output is part of this fixture.

Run `python3 score_reference.py` from this directory with NumPy installed
to validate both native captures, their sample hashes, the exact live
control readback, and the repeatability report. The two native renders
agree to −72.52 dBr on the held note and −94.35 dBr during the selected
chord handoff window. They are also in ChatGPT Library.

`open_match.py` compares these captures to the separately staged original
ARM64 receiver harness and its cutoff proof. Its then-assumed 20 kHz receiver setting was not a physically matched
comparison. The script is retained as historical evidence. It requires that harness,
the user-owned original app and the dry fan-in archive. Native
repeatability does not imply the firmware waveform or gain matches.
