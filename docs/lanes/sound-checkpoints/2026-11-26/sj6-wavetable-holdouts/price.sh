#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd);cd "$D"
T=$(mktemp -d);trap 'rm -r "$T"' EXIT
G=${1:-../prereqs/jieli-linux-toolchains-20260730.1/pi32v2/bin}
for spec in test{-sparse,}-{32-7-8,64-12-8,64-16-16}-1;do
 q=$(rev <<< "$spec"|cut -d- -f2|rev)
 for mode in direct dedup;do
   if [[ "$mode" == dedup ]];then python3 dedup.py "$spec/family-data.h" > "$T/family-data.h"; else cp "$spec/family-data.h" "$T/family-data.h"; fi
   "$G/clang" -target pi32v2 -mcpu=r3 -mfprev1 -Os -ffp-contract=off -DFAMILY_WORD="$q" -I"$T" -S target-leaf.c -o "$spec-$mode.s"
   "$G/clang" -target pi32v2 -mcpu=r3 -mfprev1 -Os -ffp-contract=off -DFAMILY_WORD="$q" -I"$T" -c target-leaf.c -o "$T/leaf.o"
   echo "$spec $mode"
   "$G/nm" -S "$T/leaf.o"
 done
done
