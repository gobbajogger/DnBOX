# SMK-37 input and SOUND navigation regressions

Base: consolidated firmware `73749f55d17a0643f1ddd072114172d29eecc2ae`.

- K4 cues the pattern in part and global views, including the ENGINE/PRESET row. K6 remains the preset control there. A deliberately assigned PAT SOUND can still recall its sound when the cued pattern plays.
- REC reads physically ordered events from all 37 keys. The stopped and running sequencer use the same pitch/step recording paths as before. Pad aliases, release ownership and press-time pitch are kept; a FIFO overflow invalidates the partial batch rather than recording a guessed note.
- A held synth key remains owned across normal same-part pages, including a page that will not accept new synth presses. Actual release ends the original note once. Changing synth part or taking over input for another mode can still end the old ownership.
- Automation never refuses a preset or engine request. A different engine parks the departing automation instead of reinterpreting it. See [engine-owned automation](engine-owned-automation.md) for persistence and recovery tests.

Focused tests: `tests/host/sound_input_hotfix_test.sh`, `tests/host/rec_37_keys_test.sh`, and `tests/host/freeplay_live_keys_test.sh` with the new REC route compiled in. Both stock builds link without PSRAM or a reserve change. No physical-device write, firmware release, or combined roadmap gate is claimed.
