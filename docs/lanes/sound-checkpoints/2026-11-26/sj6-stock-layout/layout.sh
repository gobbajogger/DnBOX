#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); R=$(dirname "$D"); S="$R/sj6-j60-warm-center"
W="${SJ6_STRICT_WARM_SOURCE:-/tmp/phaser-warm-own.Y0XNbQ}/tree"
P="$R/sj6-j60-right-native";B="$R/sj6-cross-preset"
O=$(mktemp -d "$D/scratch.XXXX"); trap 'rm -rf "$O"' EXIT
T="$O/tree"; mkdir "$T"
cp -as "$W/firmware" "$T/"
for p in wet-lease table-share j60-post; do
 while read -r file; do
   [[ "$file" == a/* ]] || continue
   file=${file#a/}; rm -- "$T/$file"; cp "$W/$file" "$T/$file"
 done < <(awk '/^--- /{print $2}' "$P/$p.patch")
done
ln -s "$W/experiments" "$T/experiments"
cd "$T"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6.c,dsp/dnbx_core.c} >&2
sha256sum "$W/lab-cold-0-2/"{elite,pro}/never-install.elf >&2
for p in wet-lease table-share j60-post;do patch --fuzz=0 -s -p1 < "$P/$p.patch";done
python3 "$P/apply.py" firmware/src/dsp/juno6_j60_native.inc "$P/right-stretch-host.patch"
sha256sum firmware/src/{app/original_runtime.c,dsp/juno6_j60_native.inc,dsp/dnbx_core.c} >&2
export TMPDIR="$O"

for q in juno6.h juno6.c; do rm -- "firmware/src/dsp/$q"; cp "$W/firmware/src/dsp/$q" "firmware/src/dsp/$q"; done
python3 "$D/wire.py" firmware/src/dsp/juno6.h firmware/src/dsp/juno6.c
TC="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
F=$(cat "$R/all-controls-remote-join/compile-flags.txt")
for m in elite pro;do
 for layout in 0 1;do
  echo "$m $layout"
  "$TC/bin/clang" -target pi32v2 -mcpu=r3 -mfprev1 -ffp-contract=off -Os -isystem "$TC/include"   -Ifirmware/src -Iexperiments/original_synth -Iexperiments/wobble -I"$W/build/smk37-elite/gen"   -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ORIGINAL_SYNTH=1 -DX0X_ARP_HOOKS=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1   -DDNBX_SJ6_RECLAIM_STOCK_LAYOUT=$layout "-DX0X_USB_PRODUCT_NAME=\"SMK37-${m}\""   $F -c "$D/layout.c" -o "$O/layout.o"
  "$TC/bin/nm" -S "$O/layout.o" | awk '$3=="R"'
 done
done

# Default/legacy preprocessor remains exact and guarded activation is refused.
cc -E -P -Ifirmware/src/dsp -Ifirmware/src "$W/firmware/src/dsp/juno6.c" > "$O/pre-before"
cc -E -P -Ifirmware/src/dsp -Ifirmware/src firmware/src/dsp/juno6.c > "$O/pre-after"
cmp "$O/pre-before" "$O/pre-after"
sha256sum "$O/pre-before"
if cc -E -P -DDNBX_SJ6_RECLAIM_STOCK_LAYOUT=1 -Ifirmware/src/dsp -Ifirmware/src firmware/src/dsp/juno6.c > /dev/null 2>"$O/guard"; then
 echo "Legacy activation should be refused"; exit 4
fi
grep 'stock Juno layout may be reclaimed' "$O/guard" | head -1
