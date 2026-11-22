# Strict emu360 handoff

The runner was rebuilt from DnBOX tag `archive/diag/dropout-observer-20261113` at `ba83317aa55cbcdb2830c5da97d5f64b763499e2` using Rust 1.85.1 and `cargo build --offline --locked --release -p fm1-live`. All five source files match the historical patched-runner pins. The local binary is `cargo-target/release/fm1-live` relative to this directory. Its SHA-256 is `3063cd6dda355cfc2f3f461efc163a73d2da1ba32cec535a6465cf607ec6b872`.

`run-calibrated.sh elite` and `run-calibrated.sh pro` used read-only package and source mounts, one writable output directory per model, a cleared environment, and a network-denying seccomp filter. The runner received no host credentials, display session or physical device interface.

| Model | Cold and warm two-second peaks | 8 s at 0 ms shift | 8 s at +1 ms shift |
| --- | ---: | ---: | ---: |
| Elite | 3,884 / 3,885 µs | 11,235 µs | 11,199 µs |
| Pro | 3,888 / 3,888 µs | 11,244 µs | 11,212 µs |

All eight runs had zero late halves, xruns and skipped cycles. At 8 s, the PCM captures contained 331,776 stereo frames, or 648 complete DMA transfers. `x0x_dbg` recorded 647 finished guest renders and `in_audio=1` at snapshot. The calibrated driver also emits a verdict for an older research criterion of ≤10,909 µs. Its exit was 1 on both models under that criterion. All four bounded shots meet the assigned <11,609 µs threshold with zero raw late, for this specific stimulus.

Guest RAM confirms 909 tracks 1–10, 808 track 1, a running sequencer and positive Chicago voice history. The selected engines were Miami A and Chicago B. Miami's terminal voice histories show zero note, velocity and wave-preparation counts, so scripted Miami events do not qualify Miami activity. Both the runtime factory pack and loaded user-loop counts were zero. BREAK's terminal render buffer was zero. This does not qualify active BREAK, Synth One or SuperJ6, or a ten-minute window.

The compact receipts are `lane-integ-resume-0130-e29c99ec-calibrated-summary.json` and `lane-integ-resume-0130-e29c99ec-calibrated-activity.json`. The complete raw archive is `lane-integ-resume-0130-e29c99ec-strict-emu360-bounded-raw.tar.gz` (8,786,220 B, SHA-256 `08fe202ea7143c3fa2b99dce7c536eead788bd2a06257fa0a5803c75a5489ea8`).

## Seed an authentic BREAK in the final combined emulator run

1. Recheck the final combined source's `firmware/src/app/slots.c` and `firmware/hal/fm1_flash_map.h` against this map. Build the stock package and matching ELF for each model at that SHA. Run a strict 2,000 ms seed shot separately per model with `--fwsc "$FWSC" --mhz 360 --shot 2000 --flash "$SEED"`, `FM1_STRICT=1` and a no-input scene whose first event is after 2,000 ms. The runner must start in an isolated output directory with a `work` subdirectory. Confirm terminal duration, zero skipped cycles, and the final 1 MiB virtual flash's prefix against `package_flash_image(FWSC)` using the final source's `replay.py::validate_seed`.
2. Use the archived file-only single-loop fixture against that model's seed. The checked `starting-break-pack.bin` is 22,050 Hz and `BR1 1` is one bar both in the canonical pack and by user-slot inference. This command copies the 17,400-byte ADPCM payload unchanged to an ordinary FSMP slot, calculates its CRC, and refuses other populated sample slots. Repeat with another `--loop` name and fresh output per independent loop test if needed.

   ```sh
   python3 -B "$OBSERVER/dnbox/dropout/fixture.py" \
     --flash "$SEED" --pack "$FINAL_SOURCE/assets/starting-breaks/starting-break-pack.bin" \
     --pack-sha256 8df1316157fc8ce8dcb87cfd267fea83790702ccd1963ae91c0de60ca255b582 \
     --loop 'BR1 1' --slot 2 --out "$FRESH_FIXTURE_DIR"
   ```

3. Revalidate the resulting `fixture.flash` package prefix and retain the fixture receipt and raw hash. Copy the fixture to a fresh `flash.bin` per shot and use `--flash` on the copy, since `fm1-live` saves to that file at exit. Run the final scene with `FM1_STRICT=1` and `--fwsc "$FWSC" --mhz 360 --shot 600000` if the gate window starts at boot. If the gate requires 600 seconds of active playback after setup, extend the shot through the full setup plus 600,000 ms and retain interval boundaries. Select the actual registered BREAK slot in the physical script and verify runtime slot identity, meaningful BREAK dry contribution, both drum kits and scheduled synth activations. The archived `FM1_DNBOX_MAP` and `FM1_DNBOX_TRACE` hooks support mapped RAM witnesses with the final ELF and layout. Use both variables together. Retain terminal duration, raw late/xruns, peak, complete PCM and scheduler counters. The old 922-event fixture cannot verify all required activity.

The checked SMK user-slot byte ranges are `[0xC2000,0xD6000)`, `[0xD6000,0xF4000)` and `[0xB3000,0xBB000)`. Slot 2 has 122,368 data bytes after its 512-byte header. All five canonical ADPCM payloads total 109,274 bytes and fit that byte budget, but the archived helper implements one zone per fixture and does not create a five-zone fixture. User-slot operation in an emulator neither demonstrates the factory XIP fit nor writes a device. No final-SHA seed, scene or ten-minute run was executed in this pass.
