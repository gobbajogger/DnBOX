# SuperJ6 LFO 2 shape on PWM 1 and PWM 2

K4 on LFO RATE selects TRI, RAMP UP, RAMP DOWN, or SQUARE. The edit uses real mailbox slot 32. The PWM BASE alias and the existing rate, mode, depth and route controls stay available. Both PWM receivers use the same phase, selected shape and amount transfer. A held shape change keeps the running clock and current tick phase. The wave choice applies to the compact engine's existing mapped receiver, using its 80 Hz grid and Q0.40 phase.

The reference is SuperJ6 0.999.8 build 9, aumu/AuPr/jay6, inspected AU SHA256 a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1. The Conductor contract identifies original LFO2 Waveform control 98 at preset main-pass position 137, with a literal +0 default. Runtime wave row indices run from 0 to 5. The verified deterministic entries are 0 triangle, 1 ascending ramp, 2 descending ramp and 3 square. Entry 4 is held entropy and entry 5 is sine. The port wires only the first four runtime entries. Authored selector decoding remains open. The authenticated bank contains 640 field words at exact quarter steps. Those frequencies and the runtime rows alone do not establish how the original setter maps the words, the original exposed selector bounds, or whether the internal sine row is selectable by this field. Access to the previously approved Build-9 binary was unavailable from the attached runner. The pending evidence is the original ID 98 setter trace across its normalized input range. Sample-and-hold also needs its independent elapsed-time and entropy behavior qualified in the compact path.

| Runtime index | Wave on bin i in 0–255 | Amount added to PWM base |
| --- | --- | --- |
| 0 | 256-bin triangle rotated by 64, −1 to +1 | wave × depth × 0.5 |
| 1 | i / 128 − 1 | depth × (wave + 1) × 0.5 |
| 2 | 1 − i / 128 | depth × (wave − 1) × 0.5 |
| 3 | +1 for i < 128, −1 otherwise | wave × depth × 0.5 |

All four paths clamp base plus amount to [0,1]. The existing 0.5 − 0.47 × amount conversion drives the selected pulse duty. Only the effective duty is substituted during rendering. The raw oscillator base, peer oscillator, and unrelated part remain independent. Depth zero or route OFF bypasses this path and retains the chosen shape.

The shape uses two bits of the existing receiver runtime word. The Juno base-cell byte 31 uses a tagged upper nibble for the nondefault selections. The valid old low-nibble marks keep their exact semantics, including the original quantized vibrato marker and the shared vibrato/A REF records. Triangle uses the same bytes as legacy projects. The record remains bank-qualified, and the shape does not claim a shared row. Shape is captured and restored through the existing note lock and project paths. The production mailbox gains two payload bytes across both parts. Native fit and timing remain outside this host qualification.

Reproduction uses the existing host UI harness. Set `X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1` and retain the combined tools and full tools in `PYTHONPATH`. Set `X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -Wl,--wrap=dnbx_pwm_from_unit'` and `SAN_FLAGS='-fsanitize=shift -fno-sanitize-recover=shift'`. Use temporary `SMC_UI_OUT`, `SMC_UI_TEST_FILE=tests/host/ui_superj6_lfo2_shape_test.c` and `SJ6_SHAPE_CASE=or` with `sh tests/host/sj6-smc-ui.sh`. Then rerun the generated test with `SJ6_SHAPE_CASE=00`, `01`, `10`, and `11` for each owner/receiver pair. Use the mailbox packet's production comparison fixture for both PCM arms and its sanitized probe cases. The first synthetic probe is now slot 33 because slot 32 is a real shape edit.

Pinned sources

- [Original ID, default and preset order](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/control_contract.json)
- [Runtime wave row and phase rotations](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/tick_prefix.cpp)
- [Wave table constructors](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/lfo_laws.cpp)
- [First-route amount transfer](https://github.com/gobbajogger/DnBOX/blob/5d18b182955d107c7f8ca09fb5b0b08a83a6adc9/experiments/superj6-conductor-source-20261115/lfo_apply.cpp)

The final packet was rebased onto the combined donor with the Synth One wet-MIX join. The transfer oracle matches 2,048 raw PWM values and 65,536 stereo frames across both receivers and all four shapes. All twelve held shape edits changed 8,191 or 8,192 selected-part frames and zero peer frames apiece. All four owner/receiver pairings pass at every supported selector position, including exact bypass and reload. The separate legacy comparison matches 32,768 stereo frames plus both saved sound images. The existing mailbox's ten sanitized host cases, A REF's 7,594 checks, and the joined Synth One LFO/shared-row fixture's 382 checks pass on that donor. The forward and reverse patch checks and host evidence are in this packet.
