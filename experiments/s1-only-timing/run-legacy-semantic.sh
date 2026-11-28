#!/bin/bash
# Run in an already patched, disposable diagnostic source checkout.
set -eu
D=$(cd "$(dirname "$0")" && pwd)
BR=host/x0x_host_legacysemantic_test.c
BS=host/build_legacysemantic_test.sh
OUT=$(mktemp -d)
trap 'rm -f "$BR" "$BS"; rm -rf "$OUT"' EXIT
sed 's/^int main(int argc, char/static int scenario_main(int argc, char/' host/x0x_host.c > "$BR"
cat "$D/check-legacy-semantic.inc" >> "$BR"
sed 's@host/x0x_host.c \$U@host/x0x_host_legacysemantic_test.c $U@' host/build_host.sh > "$BS"
X0X_S1C=1 X0X_SYNTHONE_PATCH=1 \
X0X_HOST_DEFS='-DX0X_S1C=1 -DX0X_UI_SMALL_CANVAS=1 -DDNBOX_LAB_LEGACY_SEMANTIC=1' \
X0X_HOST_OUT="$OUT/test" bash "$BS"
"$OUT/test"
