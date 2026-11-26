#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D"); cd "$R"
q=$(cat strict-warmfold-pro-c1/local-tree.txt); t="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
T=/tmp/phaser-warm-own.Y0XNbQ/tree; s=$T/firmware/src
w=$(mktemp -d /tmp/traffic-layout.XXXX); trap 'rm -r "$w"' EXIT
"$t/bin/clang" -target pi32v2 -Os -DX0X_909_HUFF_DIAG=1 -idirafter "$t/include" -I"$s" -I"$s/dsp" -I"$T/build/smk37/gen" -c "$D/layout.c" -o "$w/layout.o"
"$t/bin/objcopy" -O binary -j .traffic_layout "$w/layout.o" "$w/layout.bin"
e="$q/tree/lab-cold-0-2/pro/never-install.elf"
"$t/bin/nm" -S "$e" | grep -E ' (d808|d909)$' >"$D/symbols.txt"
python "$D/read.py" "$w/layout.bin" "$e"
