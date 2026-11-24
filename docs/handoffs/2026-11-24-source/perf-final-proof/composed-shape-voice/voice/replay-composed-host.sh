#!/bin/sh
# Host simulation only: no product build, flash, browser, network or publication.
# Usage: sh replay-host.sh <combined-source-dir> <product-postimage.sha256> <firmware-git-with-donors> <fresh-evidence-dir>
set -eu
S=$(realpath "$1"); M=$(realpath "$2"); R=$(realpath "$3")
O=$(realpath -m "$4"); mkdir "$O"
P=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
D=$(mktemp -d); trap 'rm -rf "$D"' EXIT HUP INT TERM
sha256sum "$M" >"$O/source-ledger.sha256"
printf '%s  %s\n' 950af4ab6b7ad7f153e99a7a58157ddda87dbafd85d0189ed2896ed05cf7a0d0 "$M" | sha256sum -c --quiet
export PYTHONDONTWRITEBYTECODE=1 ASAN_OPTIONS=detect_leaks=0 X0X_REMOTE_MUTANTS=0
unset X0X_SUPERJ6_FACTORY X0X_HOST_DEFS
for mode in pre post; do
    W="$D/$mode"; mkdir "$W"; cp -a "$S/." "$W/"
    (cd "$W"; sha256sum -c --quiet "$M")
    printf 'Validated %s exact product source files before test/patch (%s)\n' "$(wc -l < "$M")" "$mode"
    case "$mode" in
        pre) ref=8eff93afdb65f88edf96e6dd079866dc1f128d54 ;;
        post) ref=0aea5ab07730204abe106212715d1a4bc30f2fbf ;;
    esac
    for f in remote_dnr3_test.c remote_dnr3_test.sh remote_dnr3_ble_test.c remote_dnr3_ble_test.sh; do
        git -C "$R" show "$ref:tests/host/$f" >"$W/tests/host/$f"
    done
    if [ "$mode" = post ]; then (cd "$W"; git apply --check "$P/VOICE-forward-4files.patch"; git apply "$P/VOICE-forward-4files.patch"); fi
    python3 - "$W/tests/host/remote_dnr3_test.c" "$P" "$mode" <<'PY'
import sys
from pathlib import Path
file, base, mode = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
src = file.read_text()
assert src.count('int main(void)')==src.count('    t15_budget();')==1
new=(base/'bpm-position.cfrag').read_text()
calls='\n    focused_bpm_position();'
if mode=='pre':
    new+=(base/'missing-voice.cfrag').read_text()
    calls+='\n    missing_voice_signal();'
file.write_text(src.replace('int main(void)',new+'\nint main(void)').replace('    t15_budget();','    t15_budget();'+calls))
PY
    (cd "$W"; X0X_REMOTE_TEST_OUT="$W/usb" sh tests/host/remote_dnr3_test.sh) >"$O/$mode-usb.log" 2>&1
    if [ "$mode" = post ]; then
        (cd "$W"; X0X_REMOTE_BLE_TEST_OUT="$W/ble" sh tests/host/remote_dnr3_ble_test.sh) >"$O/post-ble.log" 2>&1
    fi
done
cat "$O/pre-usb.log" "$O/post-usb.log" "$O/post-ble.log"
