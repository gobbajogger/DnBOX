#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd);R=$(dirname "$D")
P="$R/rehydrated-reverb/dual-room/tree"
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
(cd "$R";sha256sum -c "$D/base.sha256") >&2
cp -a "$P"/{firmware,assets,experiments,host,tests,tools,build} "$T"/
patch -s -d "$T" -p1 < "$D/boot-home.patch"
bash "$D/boot-test.sh" "$T"
python "$D/mock.py" "$T"
cp "$D/home-test.c" "$T/tests/host/home-strip-entry-test.c"
sed -n '/^static void message(/q;p' "$T/tests/host/smc_ui_test.c" > "$T/tests/host/home-host-base.inc"
cd "$T"
X0X_HOST_DEFS=-DX0X_UI_HOME_STRIP_SPIKE=1 SMC_UI_TEST_FILE=tests/host/home-strip-entry-test.c \
 SMC_UI_OUT=build/home-strip-entry sh tests/host/smc_ui_test.sh
