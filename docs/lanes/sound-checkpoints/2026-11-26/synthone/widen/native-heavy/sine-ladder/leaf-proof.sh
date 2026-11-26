#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(cd "$D/../.." && pwd)
B=$R/s1-organ-wet-recovery/native-widen-bfp/tree/firmware/src/dsp/s1c
P=$D/tree/firmware/src/dsp/s1c
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
C=(-std=gnu11 -O2 -fno-fast-math -ffp-contract=off -ffunction-sections -fdata-sections)
cc "${C[@]}" -Ds1u_sin=original_sine -Ds1u_ladder=original_ladder -c "$B/s1u.c" -o "$T/b.o"
cc "${C[@]}" -Ds1u_sin=test_sine -Ds1u_ladder=test_ladder -c "$P/s1u.c" -o "$T/a.o"
objcopy --keep-global-symbol=original_sine --keep-global-symbol=original_ladder "$T/b.o"
objcopy --keep-global-symbol=test_sine --keep-global-symbol=test_ladder "$T/a.o"
cc "${C[@]}" -Wl,--gc-sections "$D/sine-oracle.c" "$T/b.o" "$T/a.o" -lm -o "$T/sine"
"$T/sine"
cc "${C[@]}" -Wl,--gc-sections "$D/ladder-oracle.c" "$T/b.o" "$T/a.o" -lm -o "$T/ladder"
"$T/ladder"
