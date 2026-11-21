# Foreground phase after audio startup

Apply `exact-size/source.patch` from [the verified startup packet](direct-dma-startup.tar.gz) to the documented base `03f92a7d94ad29bdb4a6bb238311ba12776401ea`, then apply [phase-restored.patch](phase-restored.patch). The resulting `firmware/src/app/main_fm1.c` Git blob is `36a4429a74b67a350a1a3864fcaa6751dbf6cf67`. The patch reuses the measured duration of the first render when restoring the original foreground startup interval before the panel's settle wait.

| Board | Shift ms | Worst µs | Changed project bytes of 36,420 | First different DAC stereo frame |
| --- | ---: | ---: | ---: | ---: |
| Elite | 0 | 10,455 | 3 | 111,684 |
| Elite | +1 | 10,347 | 0 | 199,282 |
| Elite | +2 | 10,336 | 4 | 199,286 |
| Elite | +3 | 10,458 | 7 | 102,402 |
| Elite | +4 | 10,409 | 5 | 102,402 |
| Elite | +5 | 10,403 | 2 | 146,496 |
| Pro | 0 | 10,102 | 10 | 111,684 |
| Pro | +1 | 10,399 | 11 | 120,896 |
| Pro | +2 | 10,329 | 4 | 102,402 |
| Pro | +3 | 10,455 | 3 | 199,286 |
| Pro | +4 | 10,424 | 10 | 199,811 |
| Pro | +5 | 10,405 | 2 | 146,496 |

Each native strict run completed 648 halves with zero raw late transfers. On Elite +1 the first differing sample was 6,461 versus 6,462 in each channel. An earlier snapshot at 4,946 ms had stock bass B cutoff 15 and candidate 18, the only difference in the entire project. At 4,958 ms the cutoff was 18 versus 21. At the first snapshot the sequencer, drum, effects, dry buses, master value and both original DMA banks matched. The projects converged by their final save, after the audio had already diverged.

The phase variant costs 64 additional application bytes on each model. Both shifted worst cases meet the 10,909 µs target. One of 12 final projects and zero of 12 complete DAC streams matched stock. The measurements above were read from the native run. The Linux workspace disconnected before its separate receipt archive could be finalized. The recovered patch was checked against its recorded source edits and the previously published base. Re-execution of this variant remains open.
