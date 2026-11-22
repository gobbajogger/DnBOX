# Synth One sub oscillator

The opt-in editable Synth One voice exposes original factory IDs 7, 8 and 9. They control sub level, the additional octave down and sine/square. A note starts one octave below its tuned pitch. The phase runs while its level is zero and survives live edits and a reused voice slot. Legacy INIT output bypasses the sub mix.

The old 32-byte sound cell and seven-byte master adjunct retain their sizes. The optional sub stores level in the top seven bits of adjunct byte zero, octave and square in the high bits of bytes one and two, and a two-bit format marker in the high bits of bytes three and four. Bit zero of byte zero records a master edit. Master values retain their seven-bit positions. The sub-only adjunct leaves the master bypassed. The existing note-lock, preset replacement and project save/load paths carry both versions.

The pinned Soundpipe reference exercised 38,102,400 sub samples and 2,592 forced phase edges. Every square value and every phase and increment matched. The sine's largest absolute deviation was 2.0862e-6 and its relative RMS error was 3.0926e-7. The existing production host runner checked all 2,048 parameter positions and found 40,960 stereo frames identical between the legacy cell and the zero-level sub cell. Elite and Pro link with 5,344 and 5,356 bytes of free XIP and 7,908 bytes of pool margin on the standalone opt-in profile.

The 38-control factory projection rendered all 694 programs. The pinned original hashes all matched the earlier projection. Firmware audibility improved from 616 to 679 programs, recovering 63 previously silent programs. All 110 zero-level sub renders were bit-identical to the earlier firmware projection. Seven programs met both stereo limits. Median spectral error was 10.47 dB left and 9.73 dB right. Complete factory parity remains open. The populated software gate is tracked in the lane qualification evidence. [Receipt](factory-38-result.json) and [per-program evidence](factory-38.jsonl.gz).

Run the focused comparisons from the repository root. Rebuild the original factory arm with the pinned test input bundle and use the normal factory projection runner with the extended sound-cell setter.

```sh
sh tests/host/synthone_sub_reference.sh build/host/synthone-sub-reference
sh tests/host/synthone_patch_test.sh build/host/synthone-patch
OPENBLAS_NUM_THREADS=1 python3 experiments/sound-hardware-v1/synthone-factory-projection/replay.py FACTORY_BUILD OUTPUT --crush-lfo --voice --sub --jobs 5
```
