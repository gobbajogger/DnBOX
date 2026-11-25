# Synth One 4-bit history: diagnostic only

The class uses 16 samples per addressable block, signed 4-bit values and one
shared exponent per block, using 2,115 of the existing 2,152 delay bytes.
Pad history decimation drops from 45 to 13. The measured best variant
removes the old compact wet boost without changing authored mix or feedback.
Apply host-bfp4.patch to the exact recovered source, set the recovered
s1c_config.h S1C_DL_WET from 1.6f to 1.0f for the no-wet-boost variant, and
use the prior s1-controlled-ab procedure and the same six private reference
pairs/calibration. This diagnostic does not preserve live warm history on
a longer time regrid. Do not use it in firmware.

The Pad last second improves from -6.299 to -1.102 dB. Whole L/R move to
+1.157/-0.748 dB and spectra to 11.119/12.559 dB. Waveform alignment error
worsens and Sahara's late tail goes from +1.250 to +3.148 dB. Larger sampled
bandwidth helps, but this does not pass the acceptance tolerances, and the
other repeat paths should not pay for a Pad-specific gain correction.
