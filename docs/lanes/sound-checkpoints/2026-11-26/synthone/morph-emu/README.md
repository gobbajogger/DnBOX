# Decoder-only strict emulator comparison
Actual JieLi PI32v2 self-contained raw test binary, no devices, audio,
firmware installation, scheduler or USB. Same binary32 phase-rounding,
function boundaries and band search for both direct and packed across
three fixed note frequencies and a dynamic morph-position sweep. Direct
readout uses authentic table bands 7/8/12. Both phase states match exactly.
Paired and simple packed sample sums match exactly for all measured cases;
lookup decode was measured 9–27% cheaper with adjacent-pair sharing.
The packed/direct ratio is 2.28/2.33/4.31 unpaired and 2.08/1.92/3.18 paired.
Read strict emulated TIMER4 ticks relatively; this is not calibrated
firmware time, an audio-loop pass or verified flash placement. Have exact
large immutable input table only locally to generate selected test bands.
Both images are padded 0x120 because raw emulator XIP begins at 0x02000000.
