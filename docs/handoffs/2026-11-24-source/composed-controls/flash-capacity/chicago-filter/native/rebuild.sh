#!/usr/bin/env bash
set -euo pipefail
here=$(cd "$(dirname "$0")"; pwd)
root=$(cd "$here/../.."; pwd)
base=$(cat "$root/recovery/fit/.current")/all-controls-fit-ramp-fixedpoints
out=${1:?fresh isolation directory}
mkdir -p "$out"
out=$(realpath "$out")
test ! -e "$out/elite" && test ! -e "$out/pro"
private=$(mktemp -d /tmp/dnbx-fast32.XXXXXXXX)
chmod 700 "$private"
trap 'rm -rf "$private"' EXIT
python3 "$root/recovery/storage-layout-v2/filter_words.py" "$base/elite/src/experiments/original_synth/chicago_packed_private.h" --fast32 --emit-test "$private" > "$out/generate.log"
workers=()
for m in elite pro; do
   p="$out/$m"
   case "$m" in elite) board=smk37-elite;; pro) board=smk37;; esac
   mkdir -p "$p/src/build/$board/gen"
   cp -a "$base/$m/src/"{firmware,tools,experiments} "$p/src/"
   (cd "$p/src"; patch -s -p1 --fuzz=0 < "$here/fast32.patch")
   for n in assets ble; do ln -s "$base/$m/src/$n" "$p/src/$n"; done
   cp -a "$base/$m/src/build/$board/gen/"* "$p/src/build/$board/gen/"
   cp "$private"/p_{a,e}.h "$p/src/build/$board/gen/"
   (
      cd "$p/src"
      export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$PWD" D9_LAB_OUT="$PWD/labout" PYTHONDONTWRITEBYTECODE=1
      export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
      python3 "$here/link.py" --board smk37 --model "$m" --sdk "$root/prereqs/sdk" > "$p/build.log" 2>&1
   ) &
   workers+=("$!")
done
for p in "${workers[@]}"; do wait "$p"; done
grep '^guarded' "$out"/{elite,pro}/build.log
