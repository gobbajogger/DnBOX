# SuperJ6 panel route

On the SMK-37 Elite and Pro, choose a bass part and press **PARA**. Turn **K2** to the **SOUND** row, then turn **K5** to **SUPERJ6**. **K4** or **K6** selects POLY, PAD, PLUCK or BRASS. The SOUND row shows the applied preset and status. Turn K2 to a parameter row and use K5–K8 to edit it. **SCALE** opens the part list. K2 moves through the list and K3 edits the selected control. **PATCH** closes the list.

The 8,918 host checks covered physical edits on both parts, all 128 formatted positions for each of the 16 macros, waveform, sub and LFO pixel landmarks, and the filter and level response to a sounding note. The measured production UI blob is `e6b2880b4121a32c20b5aa95ebbbcb52d95dc88a`.

The 26 captures follow these panel inputs in the SMK host with the production selector, sound provider and framebuffer. Each part has four preset selector frames, four parameter rows and five list positions. [Frames and pixel hashes](frames.json) · [240×240 PNG archive](frames.tar.gz)

## Stock compact controls

| Page | K5 | K6 | K7 | K8 |
|---|---|---|---|---|
| Oscillator | Wave mix, 0–100% pulse | Sub, 0–50% gain | Width, 10–90% base pulse duty | Level, 0–100% |
| Filter | Base cutoff, 35–12,035 Hz | Resonance, 0–100% | Envelope depth, 0–7,000 Hz | High pass, off or 15–2,015 Hz |
| Envelope | Attack, 2–3,002 ms | Decay, 10–3,010 ms | Sustain, 0–100% | Release time constant, 10–4,010 ms |
| LFO/chorus | PWM LFO rate, 0.1–8.1 Hz | PWM depth, 0–20 percentage points of duty | Chorus rate, 0.1–5 Hz | Chorus wet mix, 0–50% |

Each macro has 128 stored positions. The displayed lowpass base frequency and envelope depth are the two inputs to the compact filter coefficient. The renderer limits their sum internally. The lowpass curve follows the loudest active envelope, and the envelope depth tick marks its full travel. High pass bypasses at raw zero. A single envelope controls amplitude and filter movement. The chorus mixes the delayed signal with opposite polarity on the two stereo channels.

## Original SuperJ6 provider

The stock provider exposes four factory cells, 16 editable macros and two performance inputs for each bass part. The macro IDs in these cells are local to the compact engine. The full original editor requires the Sound provider to expose its admitted original field IDs, physical units and ranges, an applied snapshot per part, live edits and a persistent factory/revision identity for saved projects. Firmware UI can then bind the original controls and qualify edits, reload and peer isolation on the production provider.

At Sound revision `dea023ce632538621b59bb307ebcaf440805b032`, the experimental importer selected all 640 records in 12 banks. The application accepted a subset of original fields on 474 records. Original secondary ADSR IDs 119–122 have a tested experimental edit entry with no persistence in the stock cells. The bank data requires 122,359 read-only bytes before the reader code. Sound owns production admission, the remaining field and FX coverage, and target fit. [Factory reader and field coverage](https://github.com/gobbajogger/DnBOX/blob/dea023ce632538621b59bb307ebcaf440805b032/experiments/sound-hardware-v1/sj6-factory/README.md) · [Secondary ADSR entry](https://github.com/gobbajogger/DnBOX/blob/dea023ce632538621b59bb307ebcaf440805b032/experiments/sound-hardware-v1/sj6-amp2/typed-entry.md)

## Reproduce

Run from the repository root. The registered test runs both selector owners, every compact matrix control, the part list, project save/load and owner isolation. The packaging script checks the exact set of unique 240×240 RGB captures.

```sh
SMC_UI_TEST_FILE=tests/host/juno_slot_integration_test.c \
  SMC_UI_OUT=build/host/juno-route sh tests/host/smc_ui_test.sh
python3 tests/ui_inventory/superj6_slot.py build/host/juno-route build/host/juno-catalog
cmp build/host/juno-catalog/frames.tar.gz docs/lanes/fwui/current/superj6-slot/frames.tar.gz
cmp build/host/juno-catalog/frames.json docs/lanes/fwui/current/superj6-slot/frames.json
```

The audio comparison exercised each of the four factory cells, all 16 compact macros and all 128 positions. Each case rendered three notes (43, 54 and 65) for 512 frames, released them, then rendered another 256 frames. Both uncompressed, interleaved binary32 outputs contained 6,291,456 stereo frames (50,331,648 bytes). A byte comparison passed with SHA-256 `ce0cbe1c55fd2b1078d51d908e075c32f3010ccb270d03954ee84a49c365aa99`. The comparison isolates the compact engine and chorus adapter from the shared mixer. [Sweep source](../../../../../tests/host/juno_slot_audio_replay.c)

```sh
sh tests/host/juno_slot_audio_replay.sh build/host/juno-audio-replay
```
