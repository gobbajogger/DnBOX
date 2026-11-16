# Synth One FX history in a bounded memory budget

**Status:** design only. Nothing here is in the firmware pool, and nothing here was played on a device.

**Source:** the full-FX extraction on `820edc7bae28a53e642fe53761a2fb90d5975804` (`experiments/synthone-port-feasibility-20261114/full-reference/source_fx.hpp`). The graph is the original Synth One stereo chain: bitcrush, tremolo, auto-pan, phaser, optional ladder into the delay, four variable delays, reverb hipass, three compressors, Costello reverb, master, and a fixed 50 ms widen on the right.

**Recommendation:** keep the four-line delay, store it as 12-bit samples in the 158,400 bytes the send delay already owns (`dly_buf`, 79,200 × int16), and do not allocate the Costello tank. At 48 kHz the longest delay time that fits is **0.264 s** (12,689 samples). The factory delay time is 0.25 s, so that preset still reaches its full length. A preset that asks for the 2.5 s maximum is shortened. Inside the cap, the same indexing as the original 10 s ring is bit-exact in float; the 12-bit store is the sample error. New pool bytes: **0**. The 148 bytes between the measured pool free and the 8,192-byte floor are not used.

The person playing the device would keep a short echo at the factory setting, would lose echoes longer than about a quarter of a second, and would not get a private Costello reverb from this budget. The drum and bass send delay cannot keep a separate history in the same array.

## Where the bytes can live

The linker map in `firmware/app.ld` is the whole RAM this tree describes. There is no external RAM region.

| Region | Size | What it is for | Delay history |
| --- | --- | --- | --- |
| `.pool` | 336 KiB (`0x54000`) | FX lines, canvases, the big instrument state | The send delay `dly_buf[79200]` already occupies 158,400 bytes here (`firmware/src/app/engine.c`). |
| Pool free | about 8,340 bytes recorded | Must stay at least 8,192 (`FM1_CAN`, the 8 KiB under the stack guard) | New allocation that still meets the floor is 8,340 − 8,192 = **148 bytes**. |
| `.data` / `.bss` | 96 KiB | Static state. The map comment treats this region as full. | Not a delay line. |
| XIP flash | app code, execute in place | Recorded H6 margin is 24 bytes (Elite) and 36 bytes (Pro) above an 8,192-byte floor (`e8bf6f12`, not an ancestor of this head). | Not writable sample memory. A delay line does not go here. |
| `FL_SMP` | 80 KiB + 120 KiB on SMK-37 | User sample slots in SPI NOR (`firmware/hal/fm1_flash_map.h`) | Not audio-rate RAM. This design does not take loop slots. |
| `FL_SMPV` and the 4 KiB hole at `0xFE000` | flash spare | Sector backup and an unused flash page | Same reason. |
| NOINIT, BLERAM, stacks | 1 KiB + 4 KiB + the two stacks | Reset mailbox, Bluetooth, call stacks | Too small and already spoken for. |

The device engine rate in source is 44.1 kHz (`D9_SR` in `firmware/src/dsp/drum909_dsp.h`). The inventory below is at 48 kHz because that is the rate in the extraction's reachable-delay figure. The same word counts are a slightly longer time at 44.1 kHz.

This proposal did not relink Elite or Pro. The 8,340-byte pool figure is the baseline in `experiments/synthone-port-feasibility-20261114/direct-scheduler/TARGET-COMPOSITION-DIAGNOSTIC-REVIEW.md`. The 24/36-byte flash figure is the H6 commit message, not a new image.

## 1. FX stages at 48 kHz

Payload sizes were read from the running Soundpipe objects (`tools/synthone-fx-bounded-memory/MEASURED.json`). Parameter ranges are the min, default, and max stored in `SourceFx`. Host `sizeof` includes 8-byte pointers and is labelled when it is not a sample payload.

The four variable delays are the problem. `sp_vdelay_init` is called with `maxdel = 10` s, so each line allocates `(int)(10 × 48000) + 1` floats:

`4 × 480001 × 4 = 7,680,016` bytes.

The graph never asks for 10 s. `delayTime` runs from 0.000362812 s to 2.5 s (default 0.25 s). The left and right lines are set to `2 × delayTime` (5.0 s at the maximum). The other two lines are set to `delayTime` (2.5 s). Re-init at those maxima allocates:

`2 × 240001 × 4 + 2 × 120001 × 4 = 2,880,016` bytes.

`sp_auxdata` is 16 bytes on this host. `2,880,016 + 4 × 16 = 2,880,080`, which is the figure in `full-reference/SOURCE-MANIFEST.json`. The samples are 2,880,016 bytes. The manifest total adds the four host headers. On a 32-bit target the header is 8 bytes, so the same sum would be 2,880,048. Either number is far past 148 bytes of new pool.

| Stage | History at 48 kHz | Max time | Parameters (min, default, max) |
| --- | --- | --- | --- |
| Bitcrush hold | 16 B (4 floats). One held sample, not a buffer. At the 2,048 Hz minimum the hold is 23.4 samples long. | one sample | `bitCrushSampleRate` 2048, 48000, 48000. `bitcrushLFO` 0, 0, 3. `bitCrushDepth` is in the parameter list and is not read by this process body. |
| Tremolo | 0 | none | `tremoloLFO` 0, 0, 3. The depth is the LFO passed into the frame. |
| Auto-pan | 16,388 B sine table (`4096 + 1` floats) plus the oscillator and pan objects | none (a phase) | `autoPanAmount` 0, 0, 1. `autoPanFrequency` 0.000520833, 0.25, 10 Hz. |
| Phaser | 192 B Faust state. No sample line. Init also fixes notch limits at 100 and 800 Hz, notch frequency 1.5, vibrato on, depth 1, invert off. | none | `phaserMix` 0, 0, 1. `phaserRate` 1, 12, 300. `phaserFeedback` 0, 0, 0.8. `phaserNotchWidth` 100, 800, 1000. |
| Delay-input ladder, ×2 | 64 B each. `delay[6]` is the ladder, not a user delay. Runs only when `filterType` is 0, and that path forces resonance to 0. | 6 samples | `filterType` 0, 0, 2. `cutoff` 64, 20000, 22050 Hz. `delayInputCutoffTrackingRatio` 0.5, 0.75, 1. `delayInputResonance` 0, 0, 0.98 (unread while `filterType` is 0). |
| `delayL`, `delayR` | 1,920,004 B allocated each. Reachable payload 960,004 B each. | 5.0 s (`2 × delayTime`) | Shared with the short pair, below. |
| `delayRR`, `delayFillIn` | 1,920,004 B allocated each. Reachable payload 480,004 B each. | 2.5 s (`delayTime`) | `delayTime` 0.000362812, 0.25, 2.5 s. `delayFeedback` 0, 0.1, 0.9. `delayOn` 0, 0, 1. `delayMix` 0, 0.125, 1. |
| Delay crossfades, ×2 | 4 B each (the mix position) | none | the delay mix above |
| Reverb hipass, ×2 | 52 B host `sizeof` each (`a[8]`). No delay line. | none | `reverbHighPass` 80, 700, 900 Hz. |
| Reverb-input compressor, stereo | 56 B Faust state per channel | none | ratio 1, 13, 20. threshold −60, −8.5, 0 dB. attack 0, 0.001, 0.01 s. release 0, 0.225, 0.5 s. makeup 0.5, 1.88, 4. |
| Costello reverb | **107,440 B** for eight lines. Longest line 4,567 samples (0.0951 s). Lengths are the opcode constants scaled by the rate, not a user time. | 0.0951 s | `reverbFeedback` 0, 0.5, 1. `reverbOn` 0, 1, 1. `reverbMix` 0, 0, 1. `reverbMixLFO` 0, 0, 3. Lowpass is forced to half the sample rate. `reverbOn` only moves the crossfade; the tank is still allocated and still runs. |
| Wet-reverb compressor, stereo | 56 B Faust state per channel | none | ratio 1, 13, 20. threshold −60, −8, 0. attack 0, 0.001, 0.01. release 0, 0.15, 0.5. makeup 0.5, 1.88, 4. |
| Reverb crossfades, ×2 | 4 B each | none | the reverb mix above |
| Master compressor, stereo | 56 B Faust state per channel | none | ratio 1, 20, 20. threshold −60, −9, 0. attack 0, 0.001, 0.01. release 0, 0.15, 0.5. makeup 0.5, 2, 4. |
| Master gain | 0 | none | `masterVolume` 0, 0.5, 2. |
| Widen | 9,600 B, 2,400 floats. `round(0.05 × 48000)`. Feedback is forced to 0. | 0.050 s fixed | `widen` 0, 0, 1. At 1 the right output is only this delay. |

Small-stage state without the sine table, without the four long payloads, and without the Costello tank fits in one explicit control block of **1,300 bytes**. That block is what `ProposedControl` measures in the probe: bitcrush, pan phase, phaser, both ladders, both hipass filters, six compressor states, four line indexes, and the widen index, with pointer fields stored as `uint32_t`. It does not include allocator headers.

The 16,388-byte pan table does not fit in 148 bytes and is not in the 1,300. `fm_sinf` is already in `firmware/src/dsp/fastmath.h`. Over one second at amplitude 1, the table oscillator versus `fm_sinf` differs by at most 0.00252 at 0.25 Hz and 0.00337 at 10 Hz (RMS 0.00181 and 0.00142). That substitution drops the table.

## 2. Two budgets that fit, and one fallback

All three use memory that is already in `.pool`. None of them add pool bytes, and none of them put samples in flash.

The four lines keep the original index arithmetic (`sp_vdelay_compute`, including the cubic coefficients) and a physical window instead of the 10 s ring. The window is the recent samples. A read that would need an older sample is outside the design: `delayTime` is limited so the cubic taps stay inside the window. A direct check of 20,000 frames against `sp_vdelay` at delays inside that window was bit-exact. The same check through the whole extracted frame, on the existing 48 kHz reference stimulus, was bit-exact in float.

12-bit storage is the store already used by the send delay: `sat12_store` in `firmware/src/dsp/fxbus.c` writes `int16(clamp(sample × 2048))`, so a sample inside ±1 has a step of 1/2048 and the int16 range reaches ±16 before it saturates. None of the probes below saturated.

### Design A — full-rate 12-bit, four lines (recommended)

Carve the 1,300-byte control block out of `dly_buf`, keep a 2,400-sample widen, and split the rest 2:2:1:1 with 4 samples of cubic headroom on each line.

| Piece | int16 slots | Bytes | Time at 48 kHz |
| --- | --- | --- | --- |
| Control block | 650 | 1,300 | — |
| Widen | 2,400 | 4,800 | 0.050 s (fixed) |
| `delayL` | 25,382 | 50,764 | admitted 25,378 samples = 0.5287 s (`2 × delayTime`) |
| `delayR` | 25,382 | 50,764 | same |
| `delayRR` | 12,693 | 25,386 | admitted 12,689 samples = 0.264 s |
| `delayFillIn` | 12,693 | 25,386 | same |
| **Total** | **79,200** | **158,400** | `delayTime` maximum **0.264 s** |

At 44.1 kHz the same 12,689 admitted samples are 0.288 s, so the factory 0.25 s still fits. Spare slots after the split: 0. New pool bytes: 0.

Fidelity cost against the original, measured on the host:

- Factory-range delays (the reference fixture is 0.05 s, the lifecycle fixture is 0.005 s): float window is bit-exact. 12-bit, including a 12-bit widen, is the whole sample error.
- A program at `delayTime = 2.5` s is played at 0.264 s. That is a different echo, not a quieter copy of the same echo.

### Design B — 10:1 decimation in the same words

Use Design A's word counts, step the delay every 10 audio samples, and ramp the output between those steps. The stored 12,693 samples then cover `12693 × 10 / 48000 = 2.64` s, and the long lines cover 5.29 s, so the original 2.5 s maximum fits. Widen stays full rate.

Fidelity cost: the echo bandwidth and the feedback loop both run at 4.8 kHz. On the existing 220 Hz fixture this is already several times the error of Design A. On noise at the factory delay time the peak error is about 0.4, against about 0.003 for Design A. Decimation is the way to keep the long maximum, and it spends that error on every preset, including the short ones.

### Design C — one shared 12-bit bus (fallback)

Leave `dly_buf` as one line of 79,200 samples and use the firmware's own 256-sample guard (`78,944` usable, the same guard as `fx_retime`). Write the mono sum once. Read at `delayTime` and at `2 × delayTime`, with feedback from the long tap. That is the send delay's shape (`fx_delay` in `fxbus.c`), not the four-line graph. With the ping tap, `delayTime` fits up to about 0.82 s at 48 kHz. The 2.5 s maximum does not.

This fallback adds no second delay kernel if a later link can call the delay that is already in the image. The measured cost on a 50 ms program, which fits easily, is still a different stereo image from the four lines.

## 3. Host measurements

Command: `tools/synthone-fx-bounded-memory/run.sh` (exit 0). It compiles the extracted `SourceFx` against the vendored Soundpipe and writes `tools/synthone-fx-bounded-memory/MEASURED.json`.

The existing FX fixtures are the synthetic programs in `fx_reference_test.cpp` and `fx_lifecycle_test.cpp`:

- Reference: 220 Hz sine at amplitude 0.15, `delayTime` 0.05, mix 0.4, delay on, reverb mix 0.25, reverb on, phaser mix 0.3, widen 1, LFOs 0.3 / 0.8 / 0.6 / 0.9. One second at 48 kHz and at 44.1 kHz.
- Lifecycle: `delayTime` 0.005, mix 0.7, reverb mix 0.5, phaser mix 0.3, widen 1, 60,000 frames at 48 kHz, sine then silence, with the source's delay resets at frames 12,000 and 36,000 and the control restore at 24,000 and 48,000.

The replica of `SourceFx::frame` matched the original bit for bit on both reference rates and on the lifecycle (144,100 frames, max error 0, spectral difference 0). The float window matched the original on the 48 kHz reference as well.

SNR below uses the original stereo RMS as the signal and that channel's error RMS as the noise. Spectral figures are the max and RMS of the magnitude difference on non-overlapping 4,096-point Hann transforms. Extended rows are not the existing fixtures; they are there so a short sine cannot hide a long-delay or a bright-signal failure.

| Run | Frames | Max abs L / R | RMS L / R | SNR dB L / R | Spectral mag max L / R | Spectral mag RMS L / R |
| --- | --- | --- | --- | --- | --- | --- |
| Reference, 12-bit Design A, 48 kHz | 48,000 | 6.23e-4 / 1.69e-3 | 1.88e-4 / 6.96e-4 | 53.9 / 42.5 | 0.220 / 0.935 | 0.00714 / 0.0268 |
| Reference, 12-bit Design A, 44.1 kHz | 44,100 | 5.88e-4 / 1.68e-3 | 1.88e-4 / 6.96e-4 | 53.9 / 42.5 | 0.197 / 0.867 | 0.00710 / 0.0267 |
| Lifecycle, 12-bit Design A | 60,000 | 1.35e-3 / 3.07e-3 | 2.53e-4 / 5.41e-4 | 36.8 / 30.2 | 0.392 / 0.600 | 0.00805 / 0.0169 |
| Reference, Design A plus reverb forced off | 48,000 | 4.66e-2 / 8.81e-2 | 2.29e-2 / 3.91e-2 | 12.2 / 7.5 | 42.9 / 78.9 | 0.878 / 1.51 |
| Reference, reverb off only (float delays) | 48,000 | 4.67e-2 / 8.86e-2 | 2.30e-2 / 3.98e-2 | 12.1 / 7.3 | 43.1 / 80.0 | 0.884 / 1.53 |
| Reference, Design B decimate ×4 | 48,000 | 7.66e-3 / 2.62e-2 | 3.61e-3 / 1.24e-2 | 28.2 / 17.5 | 1.33 / 3.63 | 0.0254 / 0.0606 |
| Reference, Design B decimate ×10 | 48,000 | 1.86e-2 / 6.41e-2 | 9.04e-3 / 3.07e-2 | 20.2 / 9.6 | 3.77 / 13.5 | 0.0788 / 0.221 |
| Reference, Design C one bus | 48,000 | 2.41e-2 / 8.66e-2 | 1.15e-2 / 4.05e-2 | 18.1 / 7.2 | 9.51 / 83.9 | 0.102 / 1.58 |
| Extended, noise, delay 0.25 s, feedback 0.9, reverb off, mix 1, 12-bit | 48,000 | 9.77e-4 / 3.18e-3 | 3.99e-4 / 8.66e-4 | 48.9 / 42.2 | 0.566 / 1.22 | 0.0141 / 0.0276 |
| Extended, same noise, decimate ×4 | 48,000 | 0.424 / 0.763 | 1.22e-2 / 3.01e-2 | 19.2 / 11.4 | 0.914 / 3.36 | 0.106 / 0.356 |
| Extended, same noise, decimate ×10 | 48,000 | 0.423 / 0.755 | 2.11e-2 / 5.16e-2 | 14.5 / 6.7 | 2.15 / 6.78 | 0.184 / 0.569 |
| Extended, `delayTime` 2.5 s, feedback 0.9, 6 s, float window clamped to 0.264 s | 288,000 | 8.75e-2 / 0.471 | 3.47e-2 / 0.140 | 6.7 / −5.4 | 67.5 / 199 | 0.932 / 2.39 |
| Extended, same 2.5 s program, decimate ×10 | 288,000 | 1.78e-2 / 5.04e-2 | 3.59e-3 / 1.33e-2 | 26.4 / 15.0 | 4.01 / 9.95 | 0.0228 / 0.107 |
| Extended, same 2.5 s program, one bus | 288,000 | 0.122 / 0.160 | 4.45e-2 / 5.66e-2 | 4.5 / 2.4 | 88.3 / 105 | 0.680 / 0.992 |

Reading the table: on the existing fixtures, 12-bit storage stays near 30–54 dB SNR and never saturates. Turning the private reverb off moves the same fixture to about 12 and 7 dB, and adding 12-bit storage on top barely changes that. The reverb, not the quantiser, is the large fidelity cost of the budget. Asking for 2.5 s and getting 0.264 s is also large: on that probe the right-channel error RMS is above the original stereo RMS (SNR −5.4 dB). Decimation keeps the 2.5 s time and scores better on that one long sine, and it scores worse on the short fixture and on noise.

Design A rows quantise the 50 ms widen as well as the four lines. Design B and Design C rows leave that widen at float32, so their right-channel error does not include the widen step.

## 4. Recommendation

Use **Design A**.

| Budget | Bytes | Notes |
| --- | --- | --- |
| New `.pool` | 0 | The 148-byte margin above the floor stays unused. |
| New flash samples | 0 | No sine table and no delay image. |
| `dly_buf` reused | 158,400 | 1,300 control + 4,800 widen + 152,300 for the four 12-bit lines. |
| `delayTime` at 48 kHz | 0.264 s max | Factory 0.25 s fits. Original maximum 2.5 s does not. |
| `delayTime` at 44.1 kHz | 0.288 s max | Same word counts. |
| Costello tank | not allocated | 107,440 bytes at 48 kHz. Silence versus the original tank is the measured 12 / 7 dB SNR on the reference fixture. |
| Pan table | not allocated | `fm_sinf` instead. Measured peak error under 0.0034 at amplitude 1. |

Risks:

- `dly_buf` is the drum and bass send delay today. One array cannot hold both histories. Running both at once mixes them. This design makes the synth insert the owner of that array, or it time-multiplexes so only one consumer writes it.
- The private reverb is the audible loss on the existing fixture, larger than the 12-bit delay. The send reverb already in `fxbus` (four 12-bit combs and the allpasses, inside `fxbus_t`) can still be the shared tank. This probe did not compare that algorithm with Costello.
- Presets above 0.264 s change length. The parameter maximum in the source is 2.5 s. Shipping Design A means the control law or the preset import has to clamp, or the long presets are wrong.
- The reader is new code. The recorded flash margin is 24 and 36 bytes. This document does not claim that code fits. A later Elite and Pro link has to show code free still at least 8,192 and pool free still at least 8,192, with no growth of `.pool`.
- The 1,300-byte control size is this host's layout of the explicit struct (32-bit pointer fields, float filters). A target link is the size that counts. A few dozen bytes either way only move the delay cap by a few milliseconds.
- CPU cost of four cubic taps plus the phaser and compressors was not measured on the pi32v2.

### What still needs a build or a device

Not claimed here:

- Elite and Pro image: code free and pool free after an implementation.
- Cycle cost on the audio callback.
- A listening check.
- Costello versus the existing send reverb, sample for sample.
- Whether the owner wants the synth delay to replace the send delay.

### Not run

| Item | Reason |
| --- | --- |
| `full_chain_compare.cpp` and `full_chain_lifecycle.cpp` | They pull the compact scheduler, the full oracle, and waveform tables. `generate_source_fx.py` also names `SYNTHONE-150-METADATA.private.json`, which is not in this checkout. They are not the FX-only harness. |
| Private captures | The FX harness fixtures above are synthetic. Those two tests do not reference a private recording. |
| Send-reverb versus Costello | `fxbus.c` was not linked into this probe. The reverb-off rows are silence in that slot, not the Schroeder tank. |
| Firmware link, pool free, flash free, device playback | Design only. The 8,340 and 24/36 figures are earlier records, not a new image. |
