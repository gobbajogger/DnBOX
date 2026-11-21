# Portable Synth One original test inputs

This bundle restores the exact original89-file source closure used by the
56079bbd source comparison plus original raw/lossless table data and notices.
It has no executables, objects, SDK/toolchain or app installers. Source derives
from the project's GPL work and original MIT AudioKit/AudioKitSynthOne code.
MANIFEST.json lists all97 entries and every hash. Archive SHA256:
`885f793f13845b74ded63151a65be75fdfadea79fc7696655397ea07bd498306`.

The914465-byte archive is split into two parts so each plugin request is below
700KB including base64. These are ordinary project source/test inputs, not the
unpublished SuperJ6 bank archive. No denied disclosure route is being retried.

From the pinned lane checkout, restore into a fresh directory outside the repo:

```
python tools/restore-synthone-test-inputs.py \
  --bundle assets/test-inputs/synthone --output /tmp/s1-original-inputs
python tests/host/run_synthone_factory.py \
  --source-build /tmp/s1-original-inputs --output /tmp/s1-factory-run
```

Restore checks every part, archive member and original byte before writing. It
replaces only the explicit@SOURCE_BUILD@ placeholder with the new absolute base
in the compile recipe. The renderer then verifies all actual source paths,
rebuilds all objects and tests the current repository runtime. No old objects
are retained or needed. `--build-only` emits audio NOT_RUN, not PASS.

For deterministic output recovery, compare ALL1388 files against
`docs/sound/v2-synthone-factory-pcm-manifest.json`. Platform/libm/compiler changes
may cause differences: a nonmatching recreation is not the original. Preserve
it separately and do not upload it as the expected artifact.

```
python tools/package-synthone-factory-pcm.py --pcm /tmp/s1-factory-run/pcm \
  --expected-manifest docs/sound/v2-synthone-factory-pcm-manifest.json \
  --output /tmp/s1-factory-pcm.tar
```

The packager checks every byte before packing and again as consumed. Identical
original/compact pairs are tar hardlinks, so both original filenames recover
without storing duplicate audio. Uncompressed USTAR avoids compressor-version
byte differences. Every timestamp/owner/mode is deterministic. Large final
archive publication requires Git LFS and is a separate action, not performed
by these tools. Keep original captures/outputs until durable readback succeeds.
