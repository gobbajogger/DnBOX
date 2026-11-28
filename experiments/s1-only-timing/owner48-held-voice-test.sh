#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
cd "$D/baseline"
F=(-O2 -std=c11 -DX0X_HOST -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -ffp-contract=off -fno-fast-math -ffunction-sections -fdata-sections -Ifirmware/src -Ifirmware/src/dsp/s1c)
cc "${F[@]}" -c firmware/src/dsp/s1c/s1c.c -o "$D/base.o"
cc "${F[@]}" -c "$D/candidate/firmware/src/dsp/s1c/s1c.c" -o "$D/candidate.o"
nm -g --defined-only "$D/candidate.o" | awk 'NF==3 {print $3,"candidate_"$3}' > "$D/rename"
objcopy --redefine-syms "$D/rename" "$D/candidate.o"
printf 'int ima4_decode(int n,int *i,int *p){return 0;}\n' > "$D/ima.c"
cc "${F[@]}" "$D/owner48-held-voice-test.c" "$D/base.o" "$D/candidate.o" "$D/ima.c" firmware/src/app/s1c_part.c firmware/src/dsp/s1c/{s1u,s1c_front}.c firmware/src/dsp/s1pack/s1pack.c -lm -Wl,--gc-sections,--wrap=s1u_env_step -o "$D/check"
"$D/check"
