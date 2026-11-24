#!/bin/bash
set -euo pipefail
[[ $# == 1 ]] || { echo 'Usage: bash replay.sh EXACT_5BF_SOURCE'; exit 2; }
packet=$(cd "$(dirname "$0")" && pwd); src=$(realpath "$1")
(cd "$src"; sha256sum --quiet -c "$packet/PRE.sha256")
w=$(mktemp -d "${TMPDIR:-/tmp}/dnbox-5bf-focus-replay.XXXXXX")
echo "$w"
for arm in pre post; do
  mkdir "$w/$arm"
  cp -aL "$src"/{firmware,tests,host,tools,assets,experiments,ble} "$w/$arm/"
  mkdir "$w/$arm/build"; cp -aL "$src/build/gen" "$w/$arm/build/"
  cp "$packet/ui_synth_focus_capture.c" "$w/$arm/tests/host/"
  (
    cd "$w/$arm"
    if [[ "$arm" == post ]]; then
      patch --no-backup-if-mismatch -p1 < "$packet/focused-control-cell.patch"
      sha256sum --quiet -c "$packet/POST.sha256"
    fi
  )
  bash "$packet/replay-one.sh" "$w/$arm" exact ui_synth_focus_capture
done
diff <(grep -E '^[AB] (S1|SJ6)' "$w/pre/exact.log") <(grep -E '^[AB] (S1|SJ6)' "$w/post/exact.log")
python3 "$packet/compare.py" "$w/pre/exact" "$w/post/exact"
