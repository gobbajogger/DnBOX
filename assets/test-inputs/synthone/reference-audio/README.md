# Synth One: 694 original factory audio references

This asset branch contains the original authored Synth One factory sounds, not a firmware release. The archive contains 694 clips and an embedded per-preset manifest. These are three-second stereo Float32 little-endian captures at 44,100 Hz. All 694 original hashes match the pinned [testbench comparison records](https://github.com/gobbajogger/DnBOX/blob/36601a5855fd64fe9262bf9eb2db1b68643a5d17/experiments/sound-hardware-v1/synthone-factory-projection/rows-35.jsonl.gz).

Download this branch's numbered parts and PARTS.sha256. Then, in that directory:

```sh
sha256sum -c PARTS.sha256
cat synthone-factory-original-694.tar.zst.part-* > synthone-factory-original-694.tar.zst
printf '%s  %s\n' 67d6002ff2dc78a97bf0131705c8eebfbe28c5f84d817d26f3e8c69b5d9fae9d synthone-factory-original-694.tar.zst | sha256sum -c -
tar --zstd -xf synthone-factory-original-694.tar.zst
```

Keep this asset branch out of the firmware integration branch. For regular code work, use a single-branch clone to avoid fetching these audio objects.
