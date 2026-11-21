# Sound stock link measurements

Start from the recorded Sound source commit `0cddaeb822458fdf4ed3b0b9035fcf22248fda10` in a clean worktree. Apply one compressed patch through `gzip -dc PATCH | git apply --index -`. For the compact dictionary comparison, apply `compact-range-dictionary.patch` on top of the compact Juno patch. The resulting source trees are listed in the [memory report](../../../docs/lanes/sound-memory.md). Run the ordinary stock build separately for Elite and Pro with the pinned toolchain and SDK. The compressed receipts include the actual link logs, binary hashes, target sizes and per-engine symbol counts.

The playable subset supports two Synth One owners with independent dry voices. The compact Juno variant is a resource fixture. The shipped-Juno alternative retains the existing Juno DSP and chorus. Factory admission, independent full effects and target timing are still open. The PR's UI source is used in all three variants.
