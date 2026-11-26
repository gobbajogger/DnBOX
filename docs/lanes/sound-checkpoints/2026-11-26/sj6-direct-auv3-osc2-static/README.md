# Direct SuperJ6 AUv3: OSC2 with a fixed open filter

This bench isolates the second oscillator on the official Mac AUv3
0.999.8 build 9, disables filter modulation and pitch modulation, and
opens the filter to its maximum while retaining authored resonance and
master. MIDI, buffer boundaries, input identity and dry-route controls
are pinned to the fan-in experiment. Rendered twice at 44.1 kHz without
a DAW or standalone instrument. No firmware output is part of this fixture.

Run `python3 score_reference.py` from this directory with NumPy installed
to validate both native captures, their sample hashes, the exact live
control readback, and the repeatability report. The two native renders
agree to −72.52 dBr on the held note and −94.35 dBr during the selected
chord handoff window. They are also in ChatGPT Library.

`open_match.py` compares these captures to the separately staged original
ARM64 receiver harness and its cutoff proof. It requires that harness,
the user-owned original app and the dry fan-in archive. Native
repeatability does not imply the firmware waveform or gain matches.
