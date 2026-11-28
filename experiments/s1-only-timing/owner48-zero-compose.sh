#!/bin/sh
# Compose a source-only emulator diagnostic, never an install image.
set -eu
[ "$#" = 2 ] || { echo "Usage: $0 REPOSITORY NEW_SCRATCH_DIR" >&2; exit 2; }
F=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
sh "$F/owner48-rebuild.sh" "$@"
cd "$2/source"
for x in owner48-release-prefix owner48-s1-remove-diagnostic-counters \
  owner48-808-bank-persistent-step owner48-909-drive16-combine \
  owner48-808-cymbal-front-two-pass owner48-909-runway-fused owner48-held-bass-prestage \
  owner48-peak-scope-pcm owner48-909-sampled-envelope-local \
  owner48-project-lane-duplicate owner48-zero-asset-blob \
  owner48-zero-project-bounds owner48-zero-inplace-fx owner48-zero-osc4 owner48-zero-tom3
do
  git apply "$F/$x.patch"
done
sed -i '1i#define DNBOX_S1_OSC4 1' firmware/src/dsp/s1c/s1c.c
