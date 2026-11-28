#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
: "${HOST_OWNER_SOURCE:?set HOST_OWNER_SOURCE to bench_owner stub if needed}"
HOST=host/x0x_host_legacy_render_test.c
BUILD=host/build_legacy_render_test.sh
OUT=$(mktemp -d)
trap 'rm -f "$HOST" "$BUILD"; rm -rf "$OUT"' EXIT
sed 's/^int main(int argc, char/static int scenario_main(int argc, char/' host/x0x_host.c > "$HOST"
cat "$D/check-semantic-matrix.inc" >> "$HOST"
sed 's@host/x0x_host.c \$U@host/x0x_host_legacy_render_test.c $U@' host/build_host.sh > "$BUILD"
X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_HOST_DEFS="-DX0X_S1C=1 -DX0X_UI_SMALL_CANVAS=1 -DDNBOX_LAB_LEGACY_SEMANTIC_RENDER=1 $HOST_OWNER_SOURCE" X0X_HOST_OUT="$OUT/run" bash "$BUILD"
"$OUT/run"
MUT=1 "$OUT/run"
