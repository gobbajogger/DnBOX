#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); R=$(dirname "$D")
B="$R/ui-native-expcompact-coldread/tree"
C="$(realpath "${1:-$D/tree}")"
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
python - "$B" "$C" "$T/exact.c" <<'PY'
import re,sys
from pathlib import Path
b,c,o=map(Path,sys.argv[1:])
def src(p):return (b/'firmware/src'/p).read_text()
def dec(p,n,typ='float'):
 return re.search(r'static const '+typ+r' '+n+r'\[[^\]]*\]\s*=\s*(\{[^}]*\});',src(p))[1]
s='''#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "drum808.h"
#include "control_grids_shared.h"
#define X0X_IMA_TABLES_DEFINE
#include "ima_adpcm.h"
'''
tables=[
('sj6_native_room.inc','grid','dnbox_sj6_native_grid','float'),
('fxbus.c','fx_beats','dnbox_delay_beats','float'),
('../app/chicago_bpmdly.h','beats','dnbox_delay_beats','float'),
('master.c','RATIO_V','dnbox_master_ratio','float'),
('../app/ui.c','ratios','dnbox_master_ratio','float'),
('ima_adpcm.h','IMA_ROUND_CORR','IMA_ROUND_CORR','uint8_t'),
]
for i,(f,n,t,typ) in enumerate(tables):s+=f'static const {typ} expected{i}[]={dec("dsp/"+f,n,typ)};\n'
s=re.sub(r'\n    \};$', '\n    };',s)
for i,root in enumerate((b,c)):
 t=(root/'firmware/src/dsp/drum808.c').read_text()
 if i==0:s+=re.search(r'enum \{ L_BD[^;]+;',t)[0]+'\n'
 j=t.index('static int voice_active(');k=t.index('\n}',j)+2
 s+=t[j:k].replace('voice_active(',f'voice{i}(')+'\n'
s+='int main(void){\n'
for i,(_,_,t,_) in enumerate(tables):s+=f' if(sizeof {t}!=sizeof expected{i}||memcmp({t},expected{i},sizeof {t}))return {i+1};\n'
s+=''' drum808_t a;
 for(unsigned fill=0;fill<256;++fill){
  for(unsigned i=0;i<sizeof a;++i)((unsigned char *)&a)[i]=(uint8_t)(i*97u+fill*67u+(i>>3));
  for(int lane=-2;lane<16;++lane)if(voice0(&a,lane)!=voice1(&a,lane))return 9;
 }
 puts("Exact shared immutable float/IMA table domains; stock 808 active-byte 4608 equivalent reads including invalid lanes and independent kit pointer.");
 return 0;
}
'''
o.write_text(s)
PY
cc -std=c11 -O2 -fno-fast-math -ffp-contract=off -I"$C/firmware/src/dsp" "$T/exact.c" -o "$T/exact"
"$T/exact"
for mode in gcc;do
 "$mode" -std=c11 -O2 -ffp-contract=off -DX0X_IMA_TABLES_DEFINE "$C/tests/host/ima_delta_lut_exact_test.c" -o "$T/ima"
 "$T/ima" --check
 "$mode" -std=c11 -O1 -ffp-contract=off -DX0X_IMA_TABLES_DEFINE -fsanitize=address,undefined "$C/tests/host/ima_delta_lut_exact_test.c" -o "$T/ima"
 ASAN_OPTIONS=detect_leaks=0:abort_on_error=1 UBSAN_OPTIONS=halt_on_error=1 "$T/ima" --check
done
for arm in "$B" "$C";do
 P="$arm/firmware/src/dsp"
 cc -std=c11 -O2 -fno-fast-math -ffp-contract=off -I"$P" "$D/drum808-equivalence.c" "$P/drum808.c" "$P/x0x_param.c" -lm -o "$T/808"
 "$T/808" | sha256sum | cut -d" " -f1
done >"$T/hash"
test "$(sort -u "$T/hash"|wc -l)" -eq 1
printf "808 equal all native-host stereo/send, active and meter outputs: "
head -1 "$T/hash"
