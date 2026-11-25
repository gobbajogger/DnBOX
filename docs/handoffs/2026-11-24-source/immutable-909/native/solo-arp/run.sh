#!/bin/bash
set -euo pipefail
root=$1; w=$2
d="$root/full-gate-inputs/d9-native-reloc-wheel"; h="$root/perf-final-proof/init-fdata-id3-heavy"
b="$root/perf-final-proof/controls-init-only/source-wheel-native-scale"
tc="$root/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
export PYTHONDONTWRITEBYTECODE=1
"$tc/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -idirafter "$tc/include" -I"$w/src/firmware/src" -S -emit-llvm "$w/off.c" -o "$w/off.ll"
grep '^@native_' "$w/off.ll"
setdir="$w/src/labout"; (cd "$setdir";sha256sum pro/never-install.elf >elf.sha256)
wanted=$(awk '/^guarded /{print $NF}' "$d/pro.link.receipt");sha256sum "$setdir/pro/never-install.elf"|grep "$wanted"
e=$(mktemp -d /tmp/d9-solo-arp-seed.XXXXXX)
trap 'rm -rf -- "$e"' EXIT
ln -s "$e" "$w/seeds"; ln -s "$setdir" "$w/run-fdata"
python "$d/seed-non-user.py" "$root" "$setdir" "$e" --models pro >"$w/seed.log" 2>&1
printf 'SMK37DATAX0X\000\000\000\000' >"$w/native-owner-tag.bin"
cp -a "$b/build/gen" "$w/src/build/"
bash "$h/build-project.sh" "$root" "$w/src" >"$w/project.log" 2>&1 || { tail -20 "$w/project.log";exit 1;}
mv "$w/src/project.bin" "$w/project.bin"
for variant in 4 8 0;do
 mkdir -p "$w/arm-$variant"; cp "$w/seed.log" "$w/arm-$variant/seed.log"
 D9_DIAG_OFF="$variant" python "$w/paired.py" "$root" "$w" "$w/arm-$variant" s1-sj6-max909 pro:0
done
