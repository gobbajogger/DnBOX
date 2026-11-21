# Elite encoder clock and shared decoder table

The published phase-restored source reproduces the previous Elite +1 ms measurement. Its application occupies 546,580 bytes. The original occupies 546,516. Sharing the two identical 64-byte IMA decoder rounding tables across translation units recovers that difference on both models. On Elite the resulting build preserves the first 290,880 stereo frames and remains within the timing ceiling. The recording has 333,312 frames. [Builds and physical replay results](replay-results.json)

| Elite, +1 ms shift | Application bytes | Peak µs | Raw late / xruns | Equal leading stereo frames | Different final project bytes |
| --- | ---: | ---: | ---: | ---: | ---: |
| Stock | 546,516 | 11,323 | 1 / 1 | 333,312 | 0 |
| Published phase restoration | 546,580 | 10,347 | 0 / 0 | 199,282 | 0 |
| Phase restoration with shared IMA table | 546,516 | 10,455 | 0 / 0 | 290,880 | 10 |
| Shared table and scanner-based detent clock | 546,580 | 10,380 | 0 / 0 | 199,811 | 11 |

All four full shots executed 648 halves at strict 360 MHz using the same 922 physical events and each image's own warmed flash. Every project comparison covers all 36,420 bytes. The shared IMA table also links at the stock Pro size of 546,508 bytes. Both shared-table builds retain the stock loader byte for byte and 8,972 free pool bytes. The inherited Chicago effects compilation has a separately measured 18.94% peaking EQ and 78.43% chorus tick increase. [Original native effects measurements](../direct-dma-startup.tar.gz)

## The original cutoff discrepancy

At DMA release 385, the two recorded K1 turns both left bass B cutoff at 5. The stock foreground had dispatched its tenth detent at its own clock value of 4,903 ms. The published phase-restored firmware dispatched it at 4,906 ms. Both dispatched the eleventh at 4,916 ms. For a 127-value knob the original UI acceleration in these intervals of 13 and 10 ms gives gains of 5 and 8 respectively. At release 386 stock cutoff was 10 and the phase-restored cutoff was 13. Both added 5 on the next detent at 4,932 ms. At 4,946 ms all scanned Gray states, quarter-step counts, pending detents and the K1 edit clock/run agreed. Cutoff 15 versus 18 was the only differing project byte. The images eventually saved the same project after the live audio had already diverged. [Stock IRQ snapshots](stock-irq-snapshots.csv) · [Restored IRQ snapshots](restored-irq-snapshots.csv)

The read-only captures reproduce the complete RAM, flash, DAC, core-load CSV and run log of the independent unobserved cutoffs. The one-millisecond clocks in the table below come from the UI's own stored values. The capture samples those values at audio interrupt entry. [Stock observer calibration](stock-observer-check.json) · [Restored observer calibration](restored-observer-check.json)

| DMA entry | Stock K1 last ms / run | Restored K1 last ms / run | Stock / restored cutoff |
| ---: | --- | --- | --- |
| 384 | 4,884 / 9 | 4,883 / 9 | 0 / 0 |
| 385 | 4,903 / 10 | 4,906 / 10 | 5 / 5 |
| 386 | 4,916 / 11 | 4,916 / 11 | 10 / 13 |
| 387 | 4,932 / 12 | 4,932 / 12 | 15 / 18 |

The detent-clock experiment records the millisecond clock when the matrix decoder accepts a complete Gray cycle. A physical UI read captures the timestamp together with its accumulated delta under the same IRQ guard. The SMC and synthesized arrow paths retain their own clocks. At 4,946 ms this produces stock cutoff 15 and identical captured audio. Bass B pattern length has already changed from 35 to 32. Its native first differing stereo frame is 199,811, with samples (-291, -291) stock and (-292, -292) candidate. A physical detent timestamp does not reproduce the original foreground grouping and target ownership. [Detent-clock patch](detent-clock.patch) · [Earlier cross-role ownership trace](../pro-1ms-encoder-causality.tar.gz)

## Where the equal-size recording diverges

At 4,946 and 4,958 ms the equal-size build reproduced every stock project byte and available stereo sample, including cutoff 15 and 18 respectively. Physical stereo frame 290,880 is the first difference, 64 frames into DMA release 569. Its left/right samples were (-1322, -1383) in stock and (-1320, -1382) in the candidate. The observer's full eight-second RAM, flash, DAC, core CSV and log all match each build's primary recording byte for byte. [Stock calibration](stock-fader-observer-check.json) · [Equal-size calibration](table-fader-observer-check.json)

The first four-fader sweep started at 6,801 ms. The ADC scanner applied the same median filtering, smoothing and dead band in both images. The main loop calls the foreground fader pickup code **after** drawing the frame. At the following interrupt entries, all four published ADC values agreed across builds. The project mixer levels already differed before the affected bank reached the DAC. [Stock fader captures](stock-fader-snapshots.csv) · [Equal-size fader captures](table-fader-snapshots.csv)

| DMA entry | Approximate shot ms | Four ADC faders, both builds | Stock / equal-size 909 mixer | Stock / equal-size bass B mixer |
| ---: | ---: | --- | ---: | ---: |
| 566 | 7,013.3 | 95, 95, 95, 95 | 77 / 77 | 100 / 100 |
| 567 | 7,024.9 | 101, 101, 101, 101 | 97 / 77 | 100 / 100 |
| 568 | 7,036.5 | 107, 107, 107, 107 | 97 / 107 | 100 / 107 |
| 569 | 7,048.1 | 113, 113, 111, 113 | 97 / 107 | 100 / 107 |

The fixture held FADER BANK from 7,081 to 7,131 ms. The equal-size build selected bank B by release 574 at 7,106.116 ms. Stock stayed in bank A through the following sweep. Its foreground continued applying the four part levels. The candidate routed the second sweep to bass A, reverb return, delay return and master filter. Native offsets map all ten differing final bytes to the five mixer levels, two returns and three master filter controls. The same hardware inputs therefore still require stock-equivalent foreground sampling and bank-button handling under the reduced audio workload. [Native field offsets](field-layout.json) · [Final project bytes](replay-results.json)

## Reproduction

From firmware commit `03f92a7d94ad29bdb4a6bb238311ba12776401ea`, apply `exact-size/source.patch` from the original packet, then the published phase restoration. The resulting main file has Git blob ID `36a4429a74b67a350a1a3864fcaa6751dbf6cf67`. For the equal-size control, apply the shared-table patch. For the detent-clock branch apply the detent-clock patch on top of that control. The stock control uses commit `dea023ce632538621b59bb307ebcaf440805b032`. Build each with the pinned native JieLi compiler and SDK using `BOARD=smk37 MODEL=elite sh build.sh`. The Pro size check uses `MODEL=pro`. [Original packet](../direct-dma-startup.tar.gz) · [Published phase restoration](../phase-restored.patch) · [Shared table patch](shared-table.patch)

Using the same emulator binary identified in the receipt and the extracted direct-dma-startup directory, this pair records the controls and compares the candidate. Set MS to 4946, 4958 or 8000 and use new output directories for each invocation.

```sh
python3 replay.py --packet "$PACKET" --emulator "$EMU" --ms "$MS" \
    --elf "$STOCK_ELF" --fwsc "$STOCK_FWSC" --output "$STOCK"
python3 replay.py --packet "$PACKET" --emulator "$EMU" --ms "$MS" \
    --elf "$CANDIDATE_ELF" --fwsc "$CANDIDATE_FWSC" --output "$RUN" \
    --stock-elf "$STOCK_ELF" --stock-run "$STOCK/shot"
python3 trace.py --emulator "$EMU" --elf "$CANDIDATE_ELF" --fwsc "$CANDIDATE_FWSC" \
    --primary "$RUN/shot" --seed "$RUN/seed/flash.bin" --output "$TRACE" \
    --causality-archive ../pro-1ms-encoder-causality.tar.gz
```

Use `--faders` on the 8,000 ms trace to include sampled fader values, foreground pickup state, destination bank and project controls.
