# Foreground reads after the audio interrupt

Stock and the equal-size phase-restored Elite firmware reached different foreground input times before any shared SPI completion. This comparison follows the stock +1 ms worst callback and the fader sample preceding the first different DAC half. Both images used the original 922-event, strict 360 MHz eight-second replay and their own protected firmware bytes. [Matched timings and individual transfer deadlines](results.json)

| After release | Observation | Stock | Equal-size firmware |
| ---: | --- | ---: | ---: |
| 450 | Callback duration | 11,323 µs | 10,065 µs |
| 450 | Audio IRQ return, shot ms | 5,678.403487 | 5,677.129522 |
| 450 | Next sensitive read, shot ms | 5,711.141267 | 5,689.259100 |
| 450 | Firmware millisecond value read | 5,711 | 5,689 |
| 450 | First physical key bitmap read, shot ms | 5,711.177355 | 5,689.284545 |
| 566 | Audio IRQ return, shot ms | 7,020.711807 | 7,021.382525 |
| 566 | First physical key bitmap read, shot ms | 7,023.351622 | 7,035.163282 |
| 566 | First fader read, shot ms | 7,023.401105 | 7,035.209016 |
| 566 | Fader value read | 97 | 107 |

At release 450 the candidate returned 1.274 ms earlier. Each image then issued six foreground SPI transfers and polled SPI status 183 times before reading the millisecond clock. Both waited through five transfers of 1, 4, 1, 4 and 1 bytes. The nearest pair of completed hardware deadlines across the two runs was **21.875 ms apart**. The sixth transfer was still active when each read the clock. Its size was 512 bytes in stock and 1,024 in the candidate. The next clock reads occurred 21.882 ms apart on DMA releases 453 and 451 respectively. These are cumulative execution paths through the two images. The single callback-duration difference measures only one contributor.

At release 566, stock resumed in `cv_rect` and the equal-size image resumed in `x0x_lcd_tile_hash`. Neither made a foreground SPI transfer or status poll before the next physical key read. Stock then sampled the first fader at 97. The candidate sampled it at 107 **11.808 ms later**. Their 909 mixer levels at release 567 were 97 and 77, and the DACs first differed 64 frames into release 569. Looking separately from release 567 gives the same absence of a foreground display transfer before the next key read. [Previous mixer and DAC observations](../../scan-time/README.md)

The SPI deadline follows the firmware's transfer start. These measurements exclude both an already-running display transfer and a later common display completion as a way to align these two foreground reads. Preserving the historical fader response requires information about when the foreground will sample before it consumes the ADC state. An unconditional bank-edge latch would also change the measured Elite outcome in the calibrated 50 ms bank-press window. The existing physical gate still requires all six original shifts on both boards.

## Controls and reproduction

The observer records CPU0 bus reads while the foreground runs. The input ranges cover all 636 bytes of the SMK HAL state and the separate `fm1_ms` clock. It also recognizes direct TIMER4 counter and ADC result reads, SPI status polls and the actual deadlines programmed for each outgoing SPI1 transfer. The diagnostic window covers releases 448–459 and 561–578. The earliest relevant read is the first hardware input or clock observation after the recorded IRQ return. A completed blocking wait requires a busy status read before a scheduled deadline and a ready status read at or after it.

For each image the diagnostic reproduced the unobserved **entire RAM, flash, DAC, core-load CSV and run log**. These checks compare each image with its own primary. [Stock parity](stock-parity.json) · [Equal-size firmware parity](table-parity.json)

Apply the original interrupt-context observer patch to emulator commit `ba83317aa55cbcdb2830c5da97d5f64b763499e2`, then apply the additional read observer patch. Build the diagnostic using the pinned Rust 1.85.1 tools. Rebuild the stock and equal-size firmware as described in the primary packets. Use the matching 8,000 ms primary and warmed seed for each image. The map argument supplies the required existing native symbol names and sizes. The script resolves their actual addresses in each ELF. An observer rejects an unrelated package's flash. [Original observer](../observer.patch) · [Read observer](read-observer.patch) · [Firmware sources and build recipe](../../scan-time/README.md)

```sh
python3 replay.py --emulator "$DIAGNOSTIC" --elf "$ELF" --package "$FWSC" \
    --seed "$SEED" --primary "$PRIMARY" --map "$PRIOR_OBSERVER_MAP" --output "$TRACE"
python3 analyze.py "$STOCK_TRACE/trace.jsonl" "$TABLE_TRACE/trace.jsonl" results.json
```
