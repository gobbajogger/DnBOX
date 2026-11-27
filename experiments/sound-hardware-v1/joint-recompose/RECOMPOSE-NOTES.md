# Reconstruct a new two-engine Plan 4 diagnostic after runner recovery

This recomposes a new candidate from the recoverable half-attribution donor, not the historical ELF. The two space changes are the separate editor/BREAK space overlays. Keep all generated binaries and virtual NOR outside Git. Project fixture is offline, synthetic, never installable.

1. Verify exact half-attribution donor with the editor preimage and BREAK source pins in the editor-break-space receipt. Stage those overlays in an isolated copy. Generated Huffman assets and source are already present in that donor; the exact older huff909 patch applies in reverse with zero fuzz, proving the expected postimages. Do **not** apply it again.
2. Apply, in order: `experiments/original_synth/sj6-sampler-model-fade.patch`, `experiments/original_synth/sj6-unison-before-highpass.patch`, `experiments/sound-hardware-v1/909-unity-cursor-local-diagnostic.patch`, `docs/lanes/sound-checkpoints/2026-11-26/s1-morph-native-proof/scheduler/core1-s1.patch`, `experiments/sound-hardware-v1/sj6-legacy-zero-mix-diagnostic.patch`, `experiments/sound-hardware-v1/cold-linked-duplicates-diagnostic.patch`, then `experiments/sound-hardware-v1/joint-fixed-render-cue-diagnostic.patch`. Use zero fuzz and retain configured H16/guarded donor flags.
3. Add `X0X_CORE_PLAN=4`, `DNBOX_SJ6_SOURCE_UNISON_ORDER=1`, `DNBOX_SJ6_MODE_FADE_LAB=1`, `DNBOX_LAB_JOINT_FIXED_RENDER_CUE=1` to the verified guarded link's flag file. Build stock Pi32 target both models. Output directory must be under the guarded build tree.
4. Generate and boot the new cold virtual NOR with exact current ELF using guarded replay helper, offsets=-1, 360 MHz strict and shared lock. Capture cold project RAM, then use source-only fixture + seed author to type Synth One A plus arp, SuperJ6 B Init. Validate source out of project and all unchanged project bytes. The exact full synthetic seed is generated in /tmp only.
5. Run strict actual replay. Require five correct typed-owner native USB frame replies, the direct MIDI helper cue accepted, positive simultaneous pre-mix activity spans, valid boot stack/guards, and unchanged virtual NOR. Check separately if late/XRUN/skip counters zero and peak <=10809 us. Note-only cue is internal, not MIDI transport qualification. The current observer counts per span, not in each actual worst half, and does not establish a like-for-like speed delta vs unavailable historical ELF.

## What happened

The initial boot-time cue acknowledged the notes but Elite recorded no overlapping audible span and only four settled owner readbacks. Its timing is not treated as a joint run. Apply the separate `later-independent-cue.patch` diagnostic: wait for 112,640 rendered frames before the one-shot direct cue, and count A and B separately. That made the test witness both real sources separately and together.

| Fresh combined diagnostic | Elite | Pro |
| --- | ---: | ---: |
| APP available | 788 B | 800 B |
| Pool usable after the 8 KiB reserve | 2,588 B | 2,588 B |
| Joint-audible spans | 455 | 407 |
| Worst DMA half | 25,851 µs | 23,286 µs |
| Late halves / XRUNs | 169 / 169 | 198 / 198 |

Both models: five settled and correctly typed owner replies; 319,488 stereo frames; zero skipped cycles and bad mailboxes; emulator NOR unchanged. These are normal compact diagnostic engines, not matched full Synth One effects. Zero dropouts and 10,809 µs do not pass. Different donor and cue timing preclude a performance comparison to previous native receipt. Collect timing by audio half with true per-owner activity before attributing the measured worst case to a particular stage.
