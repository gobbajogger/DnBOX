# Later compact Sync host donor: selected source overlay

This restores the 54 changed host/firmware/test/tool source inputs for the later PWM2 + compact Sync + shared owner checkpoint, starting from the exact [historical donor](../historical-donor/README.md). It is not the historical source tree, a product update, or a full snapshot of generated files.

Apply the historical donor first and verify tree `b1eaf004c2ac01670b11ee7dbf3ad9e5620fca51`. In that disposable checkout:

```sh
(cd "$packet" && sha256sum -c SHA256SUMS.sync)
for patch in "$packet"/0*.patch; do
    git apply --index --check "$patch"
    git apply --index "$patch"
done
test "$(git write-tree)" = 80052aa341ca470a9dfb1a1ab72e0e418f667730
sha256sum -c "$packet/SYNC-SOURCE.sha256"
sha256sum -c "$joined_replay/PREIMAGE.sha256"
```

A clean index replay produced the tree above and passed all 54 overlaid source hashes and all 15 joined replay preimage hashes. It does not include or verify an independently rebuilt host executable. The prior exact host replay also used generated headers supplied outside Git. Its `build/gen/` inputs are pinned here:

| Generated header | SHA-256 |
|---|---|
| `felucca_font.h` | `9589e4ae24a01e30c7d9b8789f5892747d618392baf9f023d2f51b566504bb69` |
| `x0x_break_bank.h` | `15a6b4a15cf9ececb73cb2e6aae1d784e30eee21fdd1cd0ae9b8e6c53df61216` |
| `x0x_drum_samples.h` | `6fd7fe81e4af56afe59bc0fde462c5c237d1e86c332a3bb163a7f21eee619c50` |
| `x0x_drum_tables.h` | `9235c69643359a1d9e4f692e754d25e8b9a4014b9e41886eed995d8aaf100eb5` |

Use the [pinned joined Synth One host fixture](https://github.com/gobbajogger/DnBOX/tree/9de22cb0d3be4243c8d3560c0c3c645ed3090f3c/docs/handoffs/2026-11-24-source/ui-1340-reconciled/synthone-sync-join.s979TF) only with authorized local generated inputs and authenticated Synth One program data. No user data, program banks, audio, device or release artifacts are in these patches. The historical checkout retains optional reference source files that the trimmed original host work directory did not carry.
