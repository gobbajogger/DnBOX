#!/bin/bash
set -euo pipefail
p=$(cd "$(dirname "$0")" && pwd)
root=$(realpath "${1:-$p/../..}")
f="$root/perf-final-proof"
b="$f/controls-init-only/source-wheel-native-scale"
l="$f/init-fdata-readonly-lab"
w=$(mktemp -d "$root/.d9-native-replay.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
(cd "$b"; sha256sum --quiet -c ../WHEEL-NATIVE-SCALE-POST.sha256)
mkdir -p "$w/src/build" "$w/src/labout"
cd "$w/src"
# Keep experiments with the same local headers; symlinking it creates two
# physical paths to #pragma-once headers during native clang compilation.
for x in firmware tests host tools experiments;do cp -a "$b/$x" .;done
for x in assets ble;do ln -s "$b/$x";done
patch --fuzz=0 -s -p1 < "$f/init-only-legacy-root-prune/prune.patch"
patch --fuzz=0 -s -p1 < "$l/boot.diagnostic.diff"
cp "$l/fdata_readonly_lab.h" firmware/src/app/
export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$w/src" D9_LAB_OUT="$w/src/labout" PYTHONDONTWRITEBYTECODE=1
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
gcc -D_GNU_SOURCE -std=c11 -O2 -Wall -Wextra -Werror -Ifirmware/src "$l/oracle.c" -o "$w/oracle"
"$w/oracle"
for model in elite pro;do
 case "$model" in pro) obj=smk37;; *) obj=smk37-$model;; esac
 mkdir -p "build/$obj"
 cp -a "$b/build/$obj/gen" "build/$obj/"
 python "$l/link-sealed.py" --board smk37 --model "$model" --sdk "$root/prereqs/sdk" > "$w/$model.log" 2>&1 || { tail -45 "$w/$model.log";exit 1; }
 grep '^guarded' "$w/$model.log"
 rm -rf -- "build/$obj"
done
gcc -D_GNU_SOURCE -std=c11 -O2 -Wall -Wextra -Werror -Ifirmware/src "$p/linked-oracle.c" -o "$w/linked-oracle"
"$w/linked-oracle" "$w/src/labout"/{elite,pro}/lab_fdata.bin
# Explicit, optional, sole serialized synthetic emulator run.
if [[ ${2:-} == --run-synthetic ]];then
 e=$(mktemp -d /tmp/d9-native-wheel-replay.XXXXXX)
 trap 'rm -rf -- "$w" "$e"' EXIT
 (cd "$D9_LAB_OUT";sha256sum {elite,pro}/never-install.elf >elf.sha256)
 python "$p/seed-non-user.py" "$root" "$D9_LAB_OUT" "$e" --run
fi
if [[ ${2:-} == --run-heavy ]];then
 mkdir "$w/results"
 bash "$p/replay-heavy.sh" "$root" "$w"
fi
