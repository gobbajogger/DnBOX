# Native SuperJ6 Jupiter Sine fader and velocity response

The installed Mac AUv3 rendered ten direct reference arms through the same
reusable bench ID. Fixed-velocity (87) isolated OSC2 uses raw faders
.125/.25/.5/1. Dynamic velocity tests use MIDI 32/64/87/127 with fixed
velocity off and fader 1; MIDI 64 is repeated. The both-muted control is
bit zero. Each uses the same unshifted single-note schedule and readback
matched wet-disable settings, with exactly the listed exceptions.

Late-held fader slopes differ from an ideal/genuine isolated receiver
scaling law by no more than 0.318 dB; MIDI velocity by no more than
0.177 dB. The two velocity-64 runs agree in level to about 0.000004 dB.
These controls cannot explain the measured 10+ dB absolute discrepancy
in the separate authored-patch projection. This does not identify the
remaining absolute level stage or establish full audible parity.

Run `python3 score_response.py .` with NumPy to check all readbacks,
exact unadjusted sample hashes and the relative slopes. Same arrays
are archived in ChatGPT Library.
