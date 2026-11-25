# SuperJ6 delay: cell center host proof

This is an isolated A/B of four variants (unchanged compact, corrected center,
tight untuned strides, both). The original reference is the previously
Build9-qualified C behavioral port, not a fresh AU execution. The compact
translation unit uses the same kernel as the established compact host fixture.
No proprietary inputs, PCM, hardware writes, or firmware qualification.

The history stores an average of N input frames. At current frame n and
phase p (0-based phase), the newest completed cell spans
n-p-N .. n-p-1 and has temporal center n-p-(N+1)/2.
For interpolation where age 1 selects that cell, targeting frame n-D requires
age D/N + 0.5 - (p+0.5)/N. The baseline age formula adds (N-1)/(2N),
placing the rendered impulse centroid (N-1)/2 frames late on average.
The center-only change leaves ring bytes, stride geometry, wet mapping,
feedback, control ramps, synchronization and LFO code alone, and the
stride-1 equation unchanged. This changes the phase and indirectly the
recirculated response; it is not always beneficial in every band.

48 tone/feedback combinations and 128 impulse phases per delay tested at
44.1 kHz, with opposite channel silent in every arm. Center-only reduced
first-echo centroid absolute error across phases: stride 8 from 7 to 3.5
frames; stride 16 from 15 to 7.5; stride 32 from 31 to 15.5; stride 128
from 127 to 63.5. Mean delay error on all these powers of two is zero after
centering (within 0.0001 frame reporting precision). At ~130 ms and 1 kHz,
normalized waveform residual vs. original goes -6.360 to -16.151 dB on
echo 1 and +1.169 to -8.423 dB on echo 3 (feedback 0.499). In the same
130-ms probe at 3 kHz, center correction alone does not restore the carrier:
first repeat remains -13.852 dB vs original; tightening from stride 8 to 6
makes it -7.203 dB, but the first repeat still has a strong image. At ~248 ms,
3 kHz remains above Nyquist in every arm; more total energy is not proof
of a faithful repeat. At ~496 ms and 1 kHz, combined improves first repeat
carrier -28.633 to -6.548 dB but improves third only from -34.687 to -33.674 dB
relative to baseline (both are very weak); compared with tight geometry
alone, third regresses -20.040 to -33.674 dB. Tight strides aren't qualified:
the non-routed diagnostic path was tested, not transitions into/out of
routed coarsening. No claim of original ping-pong parity, tempo sync,
live LFO/retime trajectory, target linking/timing or release acceptance.

    for x in base center geometry combined; do
      cc -std=c11 -O2 -fPIC -shared -ffp-contract=off -I "$x" compact_probe.c -lm -o "$x.so"
    done
    python compare.py > result.json
    python centroid.py > centroid.json
    python short.py
