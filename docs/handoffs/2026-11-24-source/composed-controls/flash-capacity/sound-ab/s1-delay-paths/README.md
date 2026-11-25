# Path-specific Synth One delay experiment

The original full FX stereo renders do not include solo stems. These are causal
proxies from the recovered engine itself: compact existing 2,152 B F16 at dd45
against the same renderer's 240,000 B F16 at dd1. No reverb, mix, master or
compressor feeds back into these captured outputs. No compensating wet
gain is applied before capturing. The original four delay nodes reduce under
linearity to three stored histories, but due to quantization the two separate
right-line ablations need not numerically sum to the compact combined waveform.
The right intermediate 2T node is not directly audible; it enters the cascade.

| Tap | Full-rate late RMS | Compact RMS error | 500–1,000 Hz |
|---|---:|---:|---:|
| Left 2T | .231 | -4.78 dB | -27.16 dB |
| Right direct T | .279 | -1.78 dB | -23.84 dB |
| Right cascade 2T+T | 1.169 | -7.26 dB | -39.07 dB |
| Right combined | 1.422 | -10.08 dB | -36.46 dB |

The cascade is dominant on the right. Under the BFP4 diagnostic, dropping it
moves Pad right whole-level error -0.454 to -11.333 dB; dropping the direct
right path moves it to -1.861 dB. Neither path is safely optional. Selective
history in the shared right final stage yielded the strongest right spectral
benefit. The 2:2:3 split improves Pad right spectral distance 12.559→7.870 dB
versus previous BFP4, but left worsens 11.119→11.737, the Pad final-second
error is +1.063 dB, and Sahara Lead still worsens versus recovered baseline
(+1.250→+3.181 dB). None of seven allocations wins across all six references.
The lab scheduler and BFP4 have no qualified warm growth and no native timing
witness, so don't graft them.

Run `bash taps/reproduce.sh` for the Pad stems. Reproduce a six-preset
budget e.g. `python path.py 2 2 3`; run the local `build-run.sh` with
DNBOX_AB_LATEST set to this directory, and feed the six private reference WAVs
and unchanged saved calibration to `measure.py` as in s1-controlled-ab.
Codes `2 2 1 1` and `2 2 1 2` are direct-only and cascade-only,
respectively; without the fourth argument both paths remain. No production,
Git or hardware changes.
