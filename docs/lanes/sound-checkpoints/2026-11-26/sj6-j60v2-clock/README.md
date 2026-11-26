# SuperJ6 J60V2 converter clock

Host diagnostic, no integration change. The original active branch converts
through a 96 kHz chorus kernel and back, and applies the original output
scalar afterward. Do not replace the branch with a fixed 47-sample delay:
the signed impulse shape and conversion phases move with host rate. Stopped
and Mix 0 are exact dry paths in saved comparisons.

A generated (not copied) 64-tap Blackman-sinc reference with linear
interpolation between 257 phase rows reproduced the independently fed
original 96 kHz middle kernel. Complete active branch worst signed residual
was −145.71 dBr for tested impulses and stereo bursts at 44.1/48 kHz;
impulse peak offsets were +47/+48 frames, respectively. That bench result is
separate from this scheduler. Full coefficient storage and arithmetic are
not suitable for the current guarded firmware. Actual target format and
cost remain open.

This header reproduces the integer source coordinate and the interpolation
row/subphase using only increments and carries per output sample. At 44.1 kHz
it uses 147/320 up and 320/147 down; at 48 kHz, 1/2 and 2/1. The C test checks
7,505,000 steps, including uint32 cursor rollover. The known ratio range is
the scope of this header, not general arbitrary-ratio DSP. Rebuild with:

    cc -std=c99 -Wall -Wextra -Werror -O2 test.c -o /tmp/j60v2-clock-test
    /tmp/j60v2-clock-test

No rate-change policy, filter/amplitude/sound result, target-cycle result,
or production route is implied by the clock test.

`python3 coefficients.py` regenerates phase knots and measures the worst
absolute-sum error of a coefficient row against the generated 257-phase
reference. This bound measures the coefficient approximation, not the final
chorus audio. Signed native-host oracle experiments independently measured
8-interval Float32 cubic interpolation at 44.1 kHz: −75.83 dBr impulse,
−91.82 dBr broad stereo burst and −123.10 dBr tail. A smaller 48-tap prune
with the original window intact gave −55.96/−71.00/−63.79 dBr on that
fixture. Quantized 16-interval alternatives and a factored 48 kHz bridge
are candidates, not validated firmware.

For rational coordinate `q+p`, converter delay subtracts 32 before
the signed tap index: `x[q-32+j]`, `j=-31..32`. Up can read after host
`q+1` has already been written, so exact write-before-step storage needs
65 upstream and 64 downstream historical frames/channel. Pruning to
`j=-23..24` still needs 57 upstream and 56 downstream because the center
does not move. The C test proves these maximum ages for both tested rates.
