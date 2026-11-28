#!/bin/sh
set -eu
here=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
base=${1:?stock source root}/firmware/src/dsp/s1c
cand=${2:?patched source root}/firmware/src/dsp/s1c
tmp=$(mktemp -d /tmp/s1-room-int-host.XXXXXX)
trap 'rm -rf "$tmp"' EXIT
cc=${CC:-cc}
names=$(sed -nE 's/^[a-z0-9_]+ (s1u_[a-z0-9_]+)\(.*/\1/p' "$base/s1u.h")
defs=''
for n in $names; do
  case $n in s1u_rvb_*) v=base_${n#s1u_rvb_};; *) v=base_$n;; esac
  defs="$defs -D$n=$v"
done
flags='-DX0X_S1C=1 -O2 -std=c11 -ffp-contract=off -fsanitize=undefined -fno-sanitize-recover=all'
$cc $flags $defs -c "$base/s1u.c" -o "$tmp/base.o"
$cc $flags -I"$cand" "$here/check.c" "$tmp/base.o" -lm -o "$tmp/check"
"$tmp/check"
