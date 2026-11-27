# SMK-37 input and SOUND navigation regressions

Base: consolidated firmware `73749f55d17a0643f1ddd072114172d29eecc2ae`.

- K4 cues the pattern in part and global views, including the ENGINE/PRESET row. K6 remains the preset control there. A deliberately assigned PAT SOUND can still recall its sound when the cued pattern plays.
- REC reads physically ordered events from all 37 keys. The stopped and running sequencer use the same pitch/step recording paths as before. Pad aliases, release ownership and press-time pitch are kept; a FIFO overflow invalidates the partial batch rather than recording a guessed note.
- A held synth key remains owned across normal same-part pages, including a page that will not accept new synth presses. Actual release ends the original note once. Changing synth part or taking over input for another mode can still end the old ownership.
- Empty automation slots no longer reject bank switches, and same-engine presets keep existing automation. A cross-engine change with real automation still refuses instead of reinterpreting another engine's indices. The prototype that parks automation per engine was not included; its project-recovery behavior is not yet qualified.

Focused tests: `tests/host/sound_input_hotfix_test.sh`, `tests/host/rec_37_keys_test.sh`, and `tests/host/freeplay_live_keys_test.sh` with the new REC route compiled in. Both stock builds link without PSRAM or a reserve change. No physical-device write, firmware release, or combined roadmap gate is claimed.
