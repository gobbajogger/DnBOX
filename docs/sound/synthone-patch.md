# Synth One editor interface for FWUI

Build Sound and FWUI together with `X0X_SYNTHONE_PATCH=1`. The current selectable Synth One patch is INIT. The interface in `firmware/src/app/engine.h` enumerates the controls consumed by the selected synthesizer, reads both part and patch ownership, and queues a new value for the audio thread.

| Factory ID | Encoder label | Physical range |
| ---: | --- | --- |
| 2 | OSC BALANCE | 0–1 |
| 5 | OSC 1 LEVEL | 0–1 |
| 16 | CUTOFF | 64–22,050 Hz |
| 17 | RESONANCE | 0–0.98 |
| 19 | FILTER ENV | 0–1.2 |
| 22 | FILTER ATTACK | 0.0005–2 s |
| 23 | FILTER DECAY | 0.005–2 s |
| 24 | FILTER SUSTAIN | 0–1 |
| 25 | FILTER RELEASE | 0–2 s |
| 26 | AMP ATTACK | 0.0005–2 s |
| 27 | AMP DECAY | 0.005–2 s |
| 28 | AMP SUSTAIN | 0–1 |
| 29 | AMP RELEASE | 0.004–2 s |
| 121 | WIDEN (with `X0X_SYNTHONE_WIDEN=1`) | 0–1 |

Use `engine_s1_patch_snapshot(part, &snapshot)` to read the original 13 controls from one audio boundary. A successful result supplies the preset, revision, note-lock state and physical targets. Enumerate the slots with `s1_patch_at(i)`. For the original 13 fields the slot index addresses `snapshot.physical[i]`. In a width-enabled build, `engine_s1_patch_snapshot_widen(part, &snapshot, &width)` also supplies field 121 from that same boundary. Use the descriptor's `factory_id` when calling the individual reader or setter. Store the latest revision with the selected part. A rejected snapshot means that the editor does not currently own a readable Synth One patch on that part.

Each encoder detent changes the position by one of 127 intervals across the descriptor range. Its nominal physical increment is `(d->maximum - d->minimum) / S1_PATCH_STEPS`. For the exact target use `s1_patch_physical(d, position)`. For display pass `s1_patch_display(d, applied)` and `d->unit` to the existing FWUI unit formatter. This preserves Hz and seconds and scales the dimensionless controls to percent. INIT may supply a value between adjacent positions before the first edit.

Call `engine_s1_patch_set(part, revision, factory_id, position)` with position 0 through 127 inclusive. `S1_EDIT_QUEUED` means the audio thread accepted the request for the next available boundary. Show the applied value from a subsequent snapshot. `S1_EDIT_BUSY` means a previous edit is pending or a note lock owns the current sound. `S1_EDIT_REJECTED` requires a fresh read of the selection and a valid ID and position. A replacement or project reload invalidates the old revision, even when the patch name stays INIT. One part's revision cannot edit the other part.

The enabled firmware saves the original 13 controls in project-cell slots 8–20. A width-enabled build also stores width in slot 21. The 50 ms history belongs to the selected owner and continues recording while the effect is dry. Existing INIT cells retain their original precision. A project containing these edited slots loads on an enabled build. The default firmware rejects an edited cell. The 13-control test exercises all 1,664 positions, audible control changes, both parts, stale revisions, note locks and project reload. The opt-in +1 ms full physical scene still changes saved bytes on both models.

[Public declarations](../../firmware/src/app/engine.h) · [Descriptors and conversion](../../firmware/src/app/s1_dry_part.c) · [Executable integration test](../../tests/host/synthone_patch_test.c) · [Native receipts](../../experiments/sound-hardware-v1/synthone-patch/verification.tar.gz)
