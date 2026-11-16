# Synth One FX history in a bounded memory budget

**Status:** design only. Nothing here is in the firmware pool, and nothing here was played on a device.

**Source:** the full-FX extraction on `820edc7bae28a53e642fe53761a2fb90d5975804` (`experiments/synthone-port-feasibility-20261114/full-reference/source_fx.hpp`). The graph is the original Synth One stereo chain: bitcrush, tremolo, auto-pan, phaser, optional ladder into the delay, four variable delays, reverb hipass, three compressors, Costello reverb, master, and a fixed 50 ms widen on the right.

**Recommendation:** Design I. Float lines at the full sample rate, sized to the loaded preset, so the delay matches the original while the time fits. A 12-bit or decimated line is not this recommendation. The map in `firmware/app.ld` has no SDRAM or PSRAM. The FX pool is 344,064 bytes at `0x01C20000`, inside the 512 KB window that starts at `0x01C00000`.

Of 694 public factory presets, 40 have an armed identical history that fits in the 158,400-byte send buffer. 274 fit only if the pool is emptied down to the 8,192-byte floor. A 0.25 s delay alone is 288,064 bytes at 48 kHz, and with the reverb tank it is larger than the pool. A longer tweak than the loaded line is refused. It is not decimated.

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

## 4. Presets and Design I

Public factory banks are `AudioKit/AudioKitSynthOne` at `6466a371`, paths under `Presets/Data`. `public-source-manifest.json` stores each file's SHA-256. The JSON bodies are not committed here. They were read once, and every file matched that manifest. `User.json` is one user slot and is not in the 694. The private owner archive (`Synth one.zip`, 906 presets, SHA-256 `68ad8b5f…`) is not in the checkout and is not counted.

Tempo follows the fresh-kernel load order in `S1DSPKernel+getSetSynthParameters.mm`. `delayTime` is snapped with `S1Rate::nearestTime` while sync is still the default on and the tempo is still 120 BPM. Setting `arpRate` snaps that result again at the preset's tempo. Setting `tempoSyncToArpRate` off does not put the original seconds back. The delay divisions run from two bars down to a 1/64 triplet. Eight, six, four, and three bars are excluded. A division outside 0.0003628117914 s to 2.5 s is skipped. The FX graph then uses the snapped seconds as `delayTime`.

Bit-exact line size is `ceil(seconds × rate) + 4` floats, the window that matched `sp_vdelay` on the host. The left and right lines are `2 × delayTime`. The other two are `delayTime`. The reverb tank is the eight Costello lines: 107,440 bytes at 48 kHz and 98,760 bytes at 44.1 kHz. Widen is `round(0.05 × rate)` floats.

Armed history is those delay lines, the reverb tank, the widen line, the pan table when auto-pan is on, the phaser and ladder state when they are in the path, and the compressors and hipass filters the frame always runs. Heard history drops a stage the stored patch bypasses (delay toggle or mix at 0, reverb toggle or mix at 0, widen at 0, auto-pan amount at 0).

### Effective delay time, 694 factory presets

325 presets have an effective `delayTime` above 0.264 s. 369 are at or under that cap. This is the synced time, not the raw stored number.

| Effective delay (s) | Presets |
| --- | --- |
| 0.00–0.25 | 300 |
| 0.25–0.50 | 189 |
| 0.50–0.75 | 169 |
| 0.75–1.00 | 5 |
| 1.00–1.25 | 19 |
| 1.25–1.50 | 6 |
| 1.50–1.75 | 2 |
| 1.75–2.00 | 2 |
| 2.00–2.25 | 0 |
| 2.25–2.50 | 1 |
| 2.50 | 1 |

Banks: Sound of Izrael 101, Red Sky Lullaby 100, Bonus 94, JEC 88, Spidericemidas 61, Sound of Izrael 2 51, Brice Beasley 46, Electronisounds 43, BankA 41, DJ Puzzle 28, Francis Preve 26, Starter Bank 15.

### (a) Per-preset bytes

`tools/synthone-fx-bounded-memory/IDENTICAL.json` has one row per preset at 48 kHz and at 44.1 kHz: bank, name, stored time, snapped time, division, delay bytes, reverb bytes, heard bytes, and armed bytes. The largest armed preset at 48 kHz is Red Sky Lullaby "Outer Limits" (arp 1 BPM, snapped to the 2.5 s cap): 2,880,064 delay bytes and 3,014,268 armed bytes.

A 0.25 s snap, which is an eighth note at 120 BPM, is 288,064 delay bytes at 48 kHz. With the tank, widen, and the small state, the armed total is about 406 KB, which is larger than the pool.

### (b) How many fit

| Budget | Bytes | What is reclaimed | Armed fit, 48 kHz | Heard fit, 48 kHz | Armed fit, 44.1 kHz |
| --- | --- | --- | --- | --- | --- |
| `dly_buf` | 158,400 | Send delay only. Owner: drum and bass send, `engine.c`. | 40 / 694 | 213 / 694 | 56 / 694 |
| Send plus small pool objects | 162,976 | Above, plus undo 4,096 (UI undo), scope 256 (engine meter), BLE MIDI out 224 (`ble.c`). | 45 / 694 | 222 / 694 | 58 / 694 |
| That set, and the screen strip | 229,216 | Above, plus `cv_px` 66,240 (240×138 RGB565, `gfx.c`). The screen strip is gone. | 110 / 694 | 274 / 694 | 125 / 694 |
| Pool ceiling | 335,872 | Entire `.pool` except the 8,192-byte floor. This also removes the 909, the 808, both 303s, breaks, the send reverb inside `fxbus_t`, the Chicago inserts, and the project stage. Those struct sizes were not measured here (`x0x_drum_tables.h` is generated). The ceiling is the linker length minus the floor. | 274 / 694 | 393 / 694 | 286 / 694 |

Delay payloads alone, ignoring the tank, fit in `dly_buf` for 184 of 694 presets at 48 kHz. The longest effective time in that group is 0.136 s. The next preset is 0.139 s and does not fit. 525 presets have the delay switched on. 131 of those have delay payloads that fit in `dly_buf`.

### (c) Past the budget

The line length is chosen when the preset loads, from that preset's snapped `delayTime`. A shorter tweak stays inside the line and stays bit-exact. A longer tweak is not accepted: the sounding time stays at the allocated length, and the extra movement is not decimated, companded, or read from an old tail. Loading a preset whose armed history is above the reclaimed budget does not install a shorter stand-in. The delay stays on the previous exact preset, or it stays bypassed. Opening the reverb on a preset that was loaded without the tank is the same refusal unless the tank's 107,440 bytes (48 kHz) are already reserved.

### `dly_buf` rule

Exclusive use. A fixed split cannot hold a float four-line history of even 0.14 s and also a musical send delay. While Synth One's delay is the loaded exact preset, the drum and bass delay sends write nothing and add nothing. Their reverb combs stay in `fxbus_t`, which is not `dly_buf`. Dry drums and bass are unchanged. Turning the synth delay off, or leaving that preset, fades the wet output to dry over 256 samples, clears the buffer, then lets the send write again and fades its wet in over 256 samples. The old send echo does not come back. The host test is `ownership.cpp`: 48,000 SourceFx frames while the send is suppressed (0 send writes, 0 buffer mutations), fade step 0.00879 against an abrupt step of 0.251, stale peak 0 after the clear and 0.307 if the clear is skipped.

## 5. Recommendation

Use **Design I** for every preset whose armed byte count is within the budget that is actually reclaimed. The table in (b) is the fit list. Lossy Design A (12-bit, 0.264 s cap) and Design H (full rate up to that cap, then /2, /4, or /10) do not meet an identical-sound requirement. They remain measured in `MEASURED.json` and `HYBRID.json` so the cost of ignoring this section is on paper.

On the delay section alone, Design H at full rate matches the short fixture to about 52 / 49 dB SNR (12-bit, not float). At 2.5 s it is /10 and the raw SNR is about 2 dB; after a 27-sample alignment it is about 34 / 24 dB. A 256-sample fade across the 0.20 s to 0.40 s boundary has max step 0.00098. An abrupt clear has max step 0.294. Mu-law stays full rate at 0.40 s (SNR about 34 dB) and is still /10 at 2.5 s. A mono pool uses /4 for a 2.5 s program and is a different stereo image (right-channel SNR about 6 dB on the short fixture). None of those are bit-exact.

Per line, the four-line 12-bit budget reaches these `delayTime` maxima at 48 kHz: /1 = 0.264 s, /2 = 0.529 s, /4 = 1.057 s, /10 = 2.644 s. That covers the 2.5 s parameter maximum and does not cover the 10 s init. Mu-law doubles the stored samples, so the same steps are 0.529 s, 1.057 s, 2.115 s, and 5.287 s. The stock graph always runs all four lines at `2×` and `1×` the same `delayTime`, so it never frees three lines for one long line.

## 6. Earlier lossy recommendation

The 12-bit plan below was the recommendation before the identical-sound requirement. It is not the recommendation now.

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
| Owner preset archive, 906 presets | `Synth one.zip` is not in the checkout. The 694 count is the public factory banks only. |
| Per-engine `.pool` struct sizes | `x0x_drum_tables.h` is generated, so this host did not take `sizeof` of the 909, 808, 303, or break engines. The pool ceiling is the linker length minus the floor. |
