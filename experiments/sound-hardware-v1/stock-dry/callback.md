# Stock Synth One INIT in the production render callback

The reference run used commit `fcf82f607cd35a263c9923071aaa25066ff6d60f`, Git tree `9e19d0977a4f86ce79c2f1b1409c408079d6a97d`. The fixture extracts the stock `main_fm1.c` half-buffer definitions and `render_block` verbatim. The timed executable links the regular host build's firmware engine, runtime, DSP, kits, effects and master. The host adapter supplies the clock and external MIDI queue. The two real 256-frame engine renders and volumes run in the 512-frame callback.

Each scene schedules 2,048 consecutive blocks at 44,100 Hz. The first starts two six-note chords on separate MIDI sources and channels, one per Synth One INIT part. Both groups of notes change every two seconds. The second adds a 120 BPM sixteenth-note 909 hat, backbeat snare and bar-end crash with 808 kick and clap. Timing includes dispatch of the queued notes and drums, rendering, mixing and volume. Outside each callback, the fixture reads both production owners and checks each of the 12 exact pitches against the expected gated voice slots. The production part meters confirm which kits and synths produced audio. A second untimed executable checks both floating-point channels before the final clamp/integer conversion at every frame. Its entire stereo PCM must match the timed run byte for byte.

`protocol.json` records the thresholds used before measurement. A CPU or wall miss occurs when callback execution exceeds 512/44100 seconds. A scheduled miss occurs when the callback finishes after the next scheduled half would begin, including wake-up delay. `report.json` contains measured levels relative to 24-bit full scale, PCM SHA-256, timing percentiles and every missed scheduled block. The compressed CSVs give one row per callback, with columns for thread CPU duration, monotonic wall duration, delayed wake-up, and positive scheduled finish lateness, all in nanoseconds.

Run from a Linux machine with the project's normal private build inputs. The working directory for outputs must be separate from the fixture and source.

```sh
sh replay.sh /path/to/qualified/source /path/to/empty/output
```

The replay checks all 243 input SHA-256 values before timing. Eight silent callbacks settle preset admission. Timing begins on the first chord attack. The host uses its default scheduler and a serial simulation of the second CPU. Load shedding is disabled. The source is the stock release audio path with host transports. Host measurements omit physical DMA scheduling, AC79 core concurrency, USB audio transfers and device interrupts. They qualify these two INIT scenes only. The Pro prebuild completed before timing; the full gate was held until the timed scenes ended. Other tenants and operating-system scheduling were uncontrolled.

[Callback source, run command, per-block measurements and exact build inputs](callback-proof.tar.gz)
