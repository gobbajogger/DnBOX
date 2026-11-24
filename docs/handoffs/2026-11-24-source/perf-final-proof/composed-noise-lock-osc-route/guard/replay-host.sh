#!/usr/bin/env bash
set -euo pipefail
p=$(realpath "$(dirname "$0")")
s=$(realpath "$1")
(cd "$p"; sha256sum -c SHA256SUMS --status)
(cd "$s"; sha256sum -c "$p/PREIMAGE.sha256" --status)
w=$(mktemp -d "${TMPDIR:-/tmp}/s1-lock-owner.XXXXXX")
mkdir -p "$w/base/build"
for n in firmware assets experiments tools tests host ble; do cp -aL "$s/$n" "$w/base/"; done
cp -aL "$s/build/gen" "$w/base/build/"
cp -a "$w/base" "$w/join"
(cd "$w/join"; git apply "$p/noise-lock-owner.patch"; sha256sum -c "$p/POSTIMAGE.sha256" --status)
for side in base join; do
 (cd "$w/$side"; sed '/^int main(void)$/,$d' tests/host/ui_synthone_filter_mode_test.c >tests/host/lfo-test-helpers.h
  cp "$p/ui_synthone_noise_lock_owner_test.c" tests/host/
  python3 "$p/instrument-host-wet.py")
done
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
profile() {
 (cd "$w/$1"; SMC_UI_OUT="$w/$2" SMC_UI_TEST_FILE=tests/host/ui_synthone_noise_lock_owner_test.c sh tests/host/sj6-smc-ui.sh) >"$w/$2.log" 2>&1
}
if profile base red; then echo "Expected prior mixed lock capture failure" >&2; exit 1; fi
grep -E '^(FAIL|owner)' "$w/red.log"
profile join green
S1_FILTER_PROJECTS="$w/green" X0X_HOST_DEFS="$X0X_HOST_DEFS -DLEGACY_LOAD=1" profile base old
grep '^PASS' "$w"/{green,old}.log
