#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
C=(-std=gnu11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections)
for pair in base:old trial:new;do
    side=${pair%:*}; sym=${pair#*:}
    cc "${C[@]}" -Ds1u_comp=${sym}_comp -Ds1u_comp_coef=${sym}_coef -c "$D/$side/s1u.c" -o "$T/$sym.o"
    objcopy --keep-global-symbol=${sym}_comp --keep-global-symbol=${sym}_coef "$T/$sym.o"
done
cc "${C[@]}" -Wl,--gc-sections "$D/oracle.c" "$T/old.o" "$T/new.o" -lm -o "$T/check"
"$T/check"
