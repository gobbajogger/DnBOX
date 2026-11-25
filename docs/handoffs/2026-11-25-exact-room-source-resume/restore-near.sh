#!/bin/bash
set -euo pipefail
W="$(cd "$(dirname "$0")/.." && pwd)"; R="$W/rehydrated-reverb"; C="$R/checkpoints"; A="$C/2026-11-24-source/composed-controls"; H="$C/2026-11-25-huff-tanh-slot3-fit"; F="$C/2026-11-25-first-paired-guarded-fit"
D="$R/near-bound"
mkdir -p "$D"
cp -a --reflink=auto "$W/all-controls-remote-join/fast32-fit/tree" "$D/"
cp "$F/guarded-fit-near-bound/"{run.sh,replay-maps.sh,replay-all-maps.sh,replay-near-maps.sh} "$D/"
cp "$W/all-controls-remote-join/fast32-fit/link-guarded-exact.py" "$D/base-wrapper"
python3 "$A/all-controls-remote-join/cold-data-fit/pin-tables.py" "$D/base-wrapper" "$A/all-controls-remote-join/cold-data-fit/tables.json" "$D/link-guarded-exact.py"
tu(){ patch --batch -F0 -p1 -d "$D/tree" < "$1"; }
wr(){ patch --batch -F0 -p1 -d "$D" < "$1"; }
part(){ sed -n '/^--- a\/tree\//,$p' "$1" |patch --batch -F0 -p1 -d "$D"; }
for p in typed-receivers typed-owner-sharing typed-carrier-sharing undo-span-fit title-fit; do tu "$A/receiver-cold-join/$p.patch"; done
for p in typed-index-cold sparse-position pref-read-fit; do tu "$A/receiver-cold-join/final-fit/$p.patch"; done
( cd "$D"; grep -E '  tree/firmware/src/app/(original_presets.inc|ui.c|remote_)' "$A/receiver-cold-join/final-fit/FINGERPRINTS.sha256" |sha256sum -c )
tu "$H/huff909-fit/huff909.patch"
tu "$H/tanh-split-fit/guarded-tanh-split.patch"
wr "$H/huff-tanh-slot3-fit/combined-placement.patch"
wr "$F/guarded-fit-s1-desc/xdata-runtime-guard.patch"
tu "$F/s1-descriptor-fit/compact-s1-descriptors.patch"
wr "$F/guarded-fit-s1-desc/xdata-and-s1-placement.patch"
tu "$F/guarded-fit-ui-glyph/static-fx-glyph.patch"
part "$F/guarded-fit-mod-palettes/chicago-cold-palette.patch"
part "$F/guarded-fit-miami-cap/miami-compact-cap.patch"
part "$F/guarded-fit-miami-decimator/miami-cold-decimator.patch"
tu "$F/guarded-fit-808-spec/808-spec-factor.patch"
wr "$F/guarded-fit-near-bound/full-fit-placement.patch"
( cd "$D/tree"; sha256sum -c "$F/guarded-fit-near-bound/SOURCE.sha256" )
