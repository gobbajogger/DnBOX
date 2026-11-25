# SuperJ6 3-bit fifth-rate streaming delay

The host compares the causal stereo prototype with the Build9-verified
full-rate kernel, at .62 feedback, without adjusting authored wet mix.
At 496 ms, versus the previous 4-bit witness:

| Probe | Prior 4-bit | New 3-bit |
|---|---:|---:|
| First 3 kHz repeat level error | -4.112 dB | -0.545 dB |
| Fourth 3 kHz repeat level error | -15.666 dB | -1.944 dB |
| Fourth bright chord overall error | -0.756 dB | -0.361 dB |
| Fourth 220 Hz waveform residual | -36.004 dB | -26.541 dB |
| Worst decoded stereo history frames on tested edit | 103 | 55 |

Lower resolution worsens low-frequency precision and 3 kHz is not a null
match. The separate one-second stride-10 mode loses 18.091 dB on the first
3 kHz repeat. Mode switching with live history, original ping-pong, live
modulation worst case, and full native performance remain open.

The isolated JieLi object confirms 4,092 B mutable state, 3,948 B of that
packed ring data/headers. Interpolation table is 160 B in read-only memory.
No full firmware link is claimed. The host checks variable callbacks,
two retimes and silent opposite-channel preservation.

Put the earlier sj6-delay-proof reference C/Python next to this source.
Run python verify_stream.py --cutoff 4000 for the half-second test;
python verify_stream.py --stride 10 --delay 44100 --cutoff 1980 for one
second. Recreate interp-table.inc with python generate_interp.py. Build
the layout-only object with the prior JieLi flags (see previous probe).
