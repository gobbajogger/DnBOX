#!/bin/bash
set -euo pipefail
[[ $# == 1 || $# == 2 ]] || { echo 'Usage: bash replay.sh EXACT_SOURCE [FM]' >&2; exit 2; }
packet=$(cd "$(dirname "$0")" && pwd); src=$(realpath "$1"); mode=${2:-}
prefix=; if [[ $mode == FM ]]; then prefix=FM-; elif [[ -n $mode ]]; then exit 2; fi
(cd "$src"; sha256sum --quiet -c "$packet/${prefix}PRE.sha256")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-fx-gen-replay.XXXXXX")
echo "$w"
for arm in pre post; do
  mkdir "$w/$arm"
  cp -aL "$src"/{firmware,tests,host,tools,assets,experiments,ble} "$w/$arm/"
  mkdir "$w/$arm/build"; cp -aL "$src/build/gen" "$w/$arm/build/"
  cp "$packet/ui_tape_gen_live_capture.c" "$w/$arm/tests/host/"
  if [[ $arm == post ]]; then
    (cd "$w/$arm"; patch --no-backup-if-mismatch -p1 < "$packet/fx-actual-summary.patch"; sha256sum --quiet -c "$packet/${prefix}POST.sha256")
  fi
  bash "$packet/replay-one.sh" "$w/$arm" final ui_tape_gen_live_capture
done
diff <(grep -E '^PASS|^PCM' "$w/pre/final.log") <(grep -E '^PASS|^PCM' "$w/post/final.log")
python3 "$packet/compare.py" "$w/pre/final" "$w/post/final"
