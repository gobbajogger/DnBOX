# SMK-37 input and SOUND navigation regressions

Base: consolidated firmware `73749f55d17a0643f1ddd072114172d29eecc2ae`.

- K4 cues the pattern in part and global views, including the ENGINE/PRESET row. K6 remains the preset control there. A deliberately assigned PAT SOUND can still recall its sound when the cued pattern plays.
- REC reads physically ordered events from all 37 keys. The stopped and running sequencer use the same pitch/step recording paths as before. Pad aliases, release ownership and press-time pitch are kept; a FIFO overflow invalidates the partial batch rather than recording a guessed note.
- A held physical synth key keeps its original voice, part, pitch and press token across page, part, mode, REC and ordinary transport changes. Physical key-up releases that owner once. Preset, engine, and project changes that would destroy a held voice are deferred until key-up; incoming project transfers return busy. Explicit Panic remains separate.
- Automation never refuses a preset or engine request. A different engine parks the departing automation instead of reinterpreting it. See [engine-owned automation](engine-owned-automation.md) for persistence and recovery tests.

Focused tests: `tests/host/sound_input_hotfix_test.sh`, `tests/host/rec_37_keys_test.sh`, and `tests/host/freeplay_live_keys_test.sh` with the new REC route compiled in. Both stock builds link without PSRAM or a reserve change. No physical-device write, firmware release, or combined roadmap gate is claimed.

Combined checkpoint: host held-note regression 18,001 checks; stopped/live 37-key REC 1,250 checks; engine switch, parked automation restore and sibling isolation passed. Stock Elite and Pro links leave 1,224 B and 1,236 B of XIP respectively; each leaves 584 B above its unchanged 8,192 B audio pool reserve. These are fit and focused host results, not audio-performance qualification.
