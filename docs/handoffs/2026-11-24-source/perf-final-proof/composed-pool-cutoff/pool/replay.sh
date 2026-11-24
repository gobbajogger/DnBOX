#!/bin/bash
set -euo pipefail
[[ $# == 1 ]] || { echo 'Usage: bash replay.sh EXACT_EB0_SOURCE' >&2; exit 2; }
packet=$(cd "$(dirname "$0")" && pwd); src=$(realpath "$1")
(cd "$src"; sha256sum --quiet -c "$packet/PRE.sha256")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-eb0-pool-replay.XXXXXX")
echo "$w"
for arm in pre post; do
 mkdir "$w/$arm"
 cp -aL "$src"/{firmware,tests,host,tools,assets,experiments,ble} "$w/$arm/"
 mkdir "$w/$arm/build"; cp -aL "$src/build/gen" "$w/$arm/build/"
 cp "$packet/"ui_*_capture.c "$w/$arm/tests/host/"
 if [[ $arm == post ]]; then
  (cd "$w/post"; patch --no-backup-if-mismatch -p1 < "$packet/shared-page-pool.patch"; sha256sum --quiet -c "$packet/POST.sha256")
 fi
 bash "$packet/build-pool.sh" "$w/$arm" "$arm" ui_page_pool_capture off catalog-off
 bash "$packet/build-pool.sh" "$w/$arm" "$arm" ui_page_pool_capture on catalog-full
 for test in ui_sound_reachable_capture ui_arp_endpoint_capture ui_synth_focus_capture ui_lfo_wave_glyph_capture; do
   bash "$packet/build-pool.sh" "$w/$arm" "$arm" "$test" on "$test"
 done
done
python3 "$packet/compare.py" "$w"
