# Does a pending display transfer absorb a shorter audio callback?

For stock Elite at +1 ms, all three audio callbacks above 10,909 µs interrupted CPU0 while it was drawing pixels. The preceding display transfer had already finished in every case. A playback optimization confined to an existing SPI wait would leave all three callbacks unchanged. [Measurements](elite-plus1.json)

| DMA release | Entry ms | Audio µs | Interrupted foreground function | Remaining SPI time |
| ---: | ---: | ---: | --- | ---: |
| 419 | 5,307.305 | 10,958 | `cv_line` | 0 µs |
| 450 | 5,667.070 | 11,323 | `cv_rect` | 0 µs |
| 605 | 7,465.897 | 10,938 | `cv_text_clip` | 0 µs |

The proposed guard would allow an existing faster audio path only when the foreground was already blocked inside `fm1_lcd_wait` and the same hardware transfer would still be outstanding after the stock IRQ returned. The DMA engine advances independently of CPU0. If both versions returned to a still-busy transfer, they could potentially resume foreground work at the same peripheral deadline. Of 648 completed halves, 112 interrupted `fm1_lcd_wait`. On all 112 the transfer expired before the original audio IRQ returned. The 8,000 ms cutoff also caught a 649th interrupt in flight. Only completed callbacks enter the counts. Strict simulation used the genuine original Elite +1 package, matched warm seed and 922-event physical scene. The new observer reproduced the unobserved full DAC, RAM, flash, core-load CSV and run log byte for byte. [Observer calibration](observer-calibration.json)

The stock main loop dispatches faders after `ui_frame()` and polls other controls between drawing passes. Shortening a callback that interrupts `cv_rect` can therefore advance a foreground sample. On the measured equal-size firmware this already changed the 909 mixer value at release 567 despite identical physical fader readings, and a later 50 ms FADER BANK press selected different destinations. [Fader and bank chronology](../scan-time/README.md)

A different route would need to show where foreground timing can resynchronize **after** the drawing code resumes. Instrument the original and a known faster, independently seeded image from each over-budget IRQ through the first subsequent ADC, button or millisecond-clock read. Record intervening SPI writes and waits along with the affected control target and the audio release that first consumes it. A shared future hardware deadline with no intervening time-sensitive decision is a candidate resynchronization point. A changed sampling decision is a counterexample. At that point a firmware change would still need exact full DAC and project comparison at all six shifts on both boards, the actual application flash sizes, and physically correct startup with zero raw late. The three measured callbacks establish the limit of the present wait-only proposal.

The paired follow-up traced the next actual clock and panel reads. At release 450 the five intervening blocking SPI waits in each image had deadlines at least 21.875 ms apart. The fader read after release 566 had no intervening SPI activity in either image and occurred 11.808 ms later in the equal-size build. [Foreground reads and scheduled completions](next-read/README.md)

## Reproduction

Apply the read-only observer patch to the pinned emulator commit `ba83317aa55cbcdb2830c5da97d5f64b763499e2`. It adds the interrupted CPU0 program counter and the simulated SPI1 completion clock to the observer stream immediately before each normal audio IRQ entry. Build `cargo build --locked --release -p fm1-live` using the archived Rust 1.85.1 environment. The tested diagnostic executable had SHA-256 `3e857ba0c074aca0ae6f8f8636dc750c8327a3e93a8eacf38bd5109ddec25c1f`. [Observer patch](observer.patch)

Use a fresh destination and the genuine stock Elite `shift1` and `seed` directories obtained from the direct DMA qualification packet. The first command checks the complete primary artifacts before analysis. [Original replay instructions](../clock-handoff/README.md)

```sh
python3 ../scan-time/trace.py --emulator "$DIAGNOSTIC" --elf "$STOCK_ELF" \
    --fwsc "$STOCK_FWSC" --primary "$STOCK/shift1" --seed "$STOCK/seed/flash.bin" \
    --causality-archive ../pro-1ms-encoder-causality.tar.gz --output "$TRACE"
python3 analyze.py "$STOCK_ELF" "$TRACE/trace.jsonl" elite-plus1.json
```
