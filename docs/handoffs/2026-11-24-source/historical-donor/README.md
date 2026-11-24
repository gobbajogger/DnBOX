# Exact earlier DnBOX source donor recovery

This source-only patch set reconstructs tree `b1eaf004c2ac01670b11ee7dbf3ad9e5620fca51`, the historical audio/UI donor from the original local `1cb4b0af6361e3d1a1821db87650c20a79ab7270`. The recovered commit identity is not the original commit. No runtime files are changed on this preservation branch; the checkpoints do not represent a release.

Start with a clean disposable checkout at DnBOX integration baseline `73749f55d17a0643f1ddd072114172d29eecc2ae`. Verify the SHA-256 list in this directory, then apply the three numbered patches to its index.

```sh
(cd "$packet" && sha256sum -c SHA256SUMS)
test "$(git rev-parse HEAD)" = 73749f55d17a0643f1ddd072114172d29eecc2ae
for patch in "$packet"/0*.patch; do
    git apply --index --check "$patch"
    git apply --index "$patch"
done
test "$(git write-tree)" = b1eaf004c2ac01670b11ee7dbf3ad9e5620fca51
```

Host/target generated inputs and private preset/test assets are not included. The later Synth One/SuperJ6 oscillator and sync host donor is a different source state; follow its own exact preimage manifest and prerequisite links on the Sound preservation branch. This recovery proof restores the tracked source tree only.
