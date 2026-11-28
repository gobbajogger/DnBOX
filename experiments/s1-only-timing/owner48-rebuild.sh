#!/bin/sh
# Reconstruct the source-only Synth One diagnostic. Do not install resulting packages.
set -eu
[ "$#" = 2 ] || { echo "Usage: $0 REPOSITORY NEW_SCRATCH_DIR" >&2; exit 2; }
repo=$(realpath "$1")
mkdir -- "$2"
dst=$(realpath "$2")
[ "${dst#/dev/}" = "$dst" ]
donor=873ec0a44a31e497b995c5329d10d225d4f8cf1d
pin=a54125b655d0aa245c89fd16ede63077c6fa9d33
git -C "$repo" cat-file -e "$donor^{tree}"
git -C "$repo" cat-file -e "$pin^{tree}"
mkdir "$dst/source" "$dst/inputs"
git -C "$repo" archive "$donor" > "$dst/donor.tar"
tar -xf "$dst/donor.tar" -C "$dst/source"
rm "$dst/donor.tar"
git -C "$repo" archive "$pin" experiments/s1-only-timing > "$dst/inputs.tar"
tar -xf "$dst/inputs.tar" -C "$dst/inputs"
rm "$dst/inputs.tar"
P="$dst/inputs/experiments/s1-only-timing"
cd "$dst/source"
for change in \
  workload dry-workload dry-six dry-optimize osc-phase morph-and-env semitones \
  dry-six-drums dry-six-stationary 808-reverse-body osc-unfiltered-path \
  break-derived-offsets crc-shared editor-word 909-render-local-state 808-shaper-zero \
  display-lossless-69 adsr-share-branch 808-rebalance-r3 \
  boot-slice-48 main-scene-48 clip-keys-48 clip-main-48 live-display-48-prereqs \
  live-display-48-ordinary-knob main-remaining-fallback cold-app-flash \
  glide-advance-outline chicago-qfit minimax-stock-after-qfit owner-48-snapshot \
  native-canvas-48-switch lcd48-no-large-fallback lcd48-build-flags \
  808-rebalance-r2 owner48-808-metal-products owner48-808-panmap owner48-909-fixed-drive-phase
do
  git apply "$P/$change.patch"
done
cp "$P/chicago_fast32_minimax.json" tools/
