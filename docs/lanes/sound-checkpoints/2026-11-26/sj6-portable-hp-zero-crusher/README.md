# Compact portable SuperJ6 HP10 and active zero-Crusher response

Independent host implementation for the exact tested 44.1 kHz profile:
original Apple HighPass 10 Hz, 0 dB resonance followed by the original
connected Distortion configuration with amount and wet mix at zero.
Connected zero is not a genuine bypass. These diagnostic leaves are
not routed in shipping firmware, do not support arbitrary settings yet,
and do not replace the separate terminal filter or correct absolute gain.

`compact.h` uses four floats for each HP state (increment-coordinate
representation) and two for each active zero-Crusher channel. This is
48 bytes total for one stereo owner. The coefficients come from standard
DSP equations, not a fitted preset. With FMA contraction off, it matches
the two signed Mac Apple chains at −101.04 and −99.29 dBr residual.
The standalone HP test is −89.21 and −87.80 dBr (versus about −58 dBr for
naive single-precision direct form). The zero-Crusher stage ranges from
−124 to −128 dBr against four source-bound captures. Whole-buffer and
irregular splits are bit-identical. No target cycle or code-size claim.

Run `bash verify.sh` with a C compiler and NumPy. Inputs and outputs pin
the exact component identity, controls, and SHA of the signed stereo
render. The same reference arrays are backed up in ChatGPT Library.
