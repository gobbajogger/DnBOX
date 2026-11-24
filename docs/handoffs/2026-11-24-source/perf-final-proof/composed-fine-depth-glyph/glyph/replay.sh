#!/bin/bash
set -euo pipefail
[[ $# == 1 ]] || { echo 'Usage: bash replay.sh EXACT_8E_SOURCE' >&2; exit 2; }
packet=$(cd "$(dirname "$0")" && pwd); src=$(realpath "$1")
(cd "$src"; sha256sum --quiet -c "$packet/PRE.sha256")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-8e-glyph-replay.XXXXXX")
echo "$w"
for arm in pre post; do
 mkdir "$w/$arm"
 cp -aL "$src"/{firmware,tests,host,tools,assets,experiments,ble} "$w/$arm/"
 mkdir "$w/$arm/build"; cp -aL "$src/build/gen" "$w/$arm/build/"
 cp "$packet/ui_lfo_wave_glyph_capture.c" "$w/$arm/tests/host/"
 if [[ $arm == post ]]; then
  (cd "$w/$arm"; patch --no-backup-if-mismatch -p1 < "$packet/sj6-lfo2-wave-glyph.patch"; sha256sum --quiet -c "$packet/POST.sha256")
 fi
 bash "$packet/replay-one.sh" "$w/$arm" renders ui_lfo_wave_glyph_capture
done
diff <(grep '^PASS' "$w/pre/renders.log") <(grep '^PASS' "$w/post/renders.log")
python3 "$packet/compare.py" "$w/pre/renders" "$w/post/renders"
