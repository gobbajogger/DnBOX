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
