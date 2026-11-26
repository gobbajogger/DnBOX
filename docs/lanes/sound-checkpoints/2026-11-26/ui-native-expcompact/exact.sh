#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D")
S="$R/s1-morph-native/tree"
A="$(realpath "${1:-$D/tree}")"
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
python - "$S/experiments/original_synth/voice_gain.c" "$A/experiments/original_synth/voice_gain.c" "$T/inverse.c" <<'PY'
from pathlib import Path
import sys
o=Path(sys.argv[-1])
s='#include <stdint.h>\n#include <stdio.h>\n#include <string.h>\n'
for i,f in enumerate(map(Path,sys.argv[1:3])):
 a=f.read_text();lo=a.index('static const float control_span_');hi=a.index('static float clamp',lo)
 t=a[lo:hi].replace('duration_inverse',f'inv_{i}')
 s+=t+'\n'
s+='''
int main(void){
 for(uint32_t d=0;d<131072;d++){
  float a=inv_0(d),b=inv_1(d);
  if(memcmp(&a,&b,4)){fprintf(stderr,"word mismatch %u\\n",d);return 1;}
 }
 const uint32_t edge[]={0xfffffffeu,0xffffffffu,2147482880u};
 for(unsigned i=0;i<3;i++){float a=inv_0(edge[i]),b=inv_1(edge[i]);if(memcmp(&a,&b,4))return 1;}
 puts("Inverse: all 64 table words match exactly; 131,075 including former division route and boundary values checked.");
 return 0;
}
'''
o.write_text(s)
PY
cc -O2 -fno-fast-math -ffp-contract=off "$T/inverse.c" -o "$T/inverse"
"$T/inverse"
for model in smk37-elite smk37;do
 G="$A/build/$model/gen"
 grep -q 'x0x_exp_layout\[19\]' "$G/x0x_drum_tables.h"
 cc -O2 -fno-fast-math -ffp-contract=off -I"$G" "$S/tests/host/exp_curve_exact_test.c" -o "$T/exp"
 printf '%s EXP: ' "$model"; "$T/exp"
done
