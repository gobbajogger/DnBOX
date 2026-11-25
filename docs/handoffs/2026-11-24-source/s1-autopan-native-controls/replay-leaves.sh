#!/bin/sh
set -eu
d=$(realpath "$(dirname "$0")"); root=$(realpath "$1"); tc=$root/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
b=$(mktemp -d "$root/.s1-pan-leaves.XXXXXX")
trap 'rm -rf -- "$b"' EXIT HUP INT TERM
cc -std=c11 -O2 -ffp-contract=off "$d/generate.c" -lm -o "$b/generator"
"$b/generator" > "$b/table.inc"; cmp "$d/s1_pan_port_table.inc" "$b/table.inc"
cc -std=c11 -O2 -ffp-contract=off -fsanitize=undefined -fno-sanitize-recover=all "$d/verify.c" -lm -o "$b/verify"
"$b/verify"
compile() {
 "$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -std=c11 -Os -ffreestanding -fno-builtin    -ffunction-sections -fdata-sections -idirafter "$tc/include" "$@"
}
compile -c "$d/native-probe.c" -o "$b/probe.o"
readelf -SW "$b/probe.o" | grep -E 'physical_|s1_pan_(detent|boundary)'
test -z "$("$tc/bin/nm" -u "$b/probe.o")"
printf '%s\n' 'JieLi object: no unresolved runtime or libm symbols'
