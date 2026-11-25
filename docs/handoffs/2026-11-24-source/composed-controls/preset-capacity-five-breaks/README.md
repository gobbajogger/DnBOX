# Preset counts with all five BREAK loops held fixed

Offline, whole original banks only. Replays the existing independently
installable DNB2 prototype encoder across the actual authorized 25 banks
(1,546 presets). Each encoded bank pays a 32-byte pool record header plus
full 4 KiB erase-sector rounding. No preset audio/data or archive content is
emitted. Directory sectors are already reserved in the separate layout.
The bank packing never spans distinct 8 KiB extents.

| Conditional free data | Static maximum | Same-size replacement reserve | Mixed engines with reserve |
| --- | ---: | ---: | ---: |
| 8 KiB | 68 | 42 | no two-bank combination |
| 16 KiB | 129 | 83 | 83 (68 Synth One + 15 SuperJ6) |
| 24 KiB | 180 | 129 | 118 (103 Synth One + 15 SuperJ6) |

8 KiB requires the existing offline lossless/audio and directory geometry.
16/24 KiB assume additional legacy journal migration, not current writable
user flash. Full restore of arbitrary larger future banks is not promised:
the reserve holds one second copy of the largest selected bank, up to 8 KiB.
The production allocator/installer and guarded all-controls fit are not
qualified. All five original BREAK loops are preserved in every scenario.

Replay: python count.py <prototype-encoder-dir> <authorized-J6-zip> <authorized-S1-zip>
