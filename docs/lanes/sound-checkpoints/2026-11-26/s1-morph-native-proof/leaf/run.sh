#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(cd "$D/../.." && pwd); A="$R/recovered-sound/s1-room-parity"; T="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"
S="$R/s1-morph-native-kr/tree/firmware/src/dsp/s1c"
F=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$T/include")
cat > "$D/decoder.c" <<'EOF'
#include "morph-native.inc"
uint32_t tested_prepare(float f){return prep_morph(f);}
float tested_tick(uint32_t *p,uint32_t plan,float x,int b){return tick_morph(p,plan,x,b);}
EOF
"$T/bin/clang" "${F[@]}" -I"$A" -I"$S" -DDNBX_MORPH_TARGET_FRAGMENT -DDNBX_MORPH_PAIR=1 -c "$D/decoder.c" -o "$D/decoder.o"
for n in uncached cached;do
 cp "$D/bench.c" "$D/bench-$n.c"
 if [[ "$n" == cached ]]; then
    python - "$D/bench-$n.c" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);t=p.read_text().replace('extern float exact_morph_sample(uint32_t *,float,float,int);','extern uint32_t tested_prepare(float);\nextern float tested_tick(uint32_t *,uint32_t,float,int);')
t=t.replace('for(unsigned i=0;i<N;i++)v+=exact_morph_sample(&phase,freq[k],(float)(i&1023u)*(1.f/1024.f),1);','uint32_t plan=0; for(unsigned i=0;i<N;i++){ if(!(i&7u))plan=tested_prepare(freq[k]); v+=tested_tick(&phase,plan,(float)(i&1023u)*(1.f/1024.f),1); }')
p.write_text(t)
PY
 fi
 "$T/bin/clang" "${F[@]}" -I"$R/fw/firmware/hal" -I"$S" -c "$D/bench-$n.c" -o "$D/bench-$n.o"
 "$T/bin/ld" --gc-sections -T "$D/bench.ld" "$D/"{start,arrays,direct,decoder}.o "$D/bench-$n.o" -o "$D/$n.elf"
 "$T/bin/objcopy" -O binary -j .text "$D/$n.elf" "$D/$n.bin"
 python - "$D/$n" <<'PY'
from pathlib import Path
import sys
p=Path(sys.argv[1]);Path(str(p)+'-padded.bin').write_bytes(bytes(0x120)+p.with_suffix('.bin').read_bytes())
PY
done
cd "$D"; mkdir -p work
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
for n in uncached cached;do
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMPRAM="$D/$n.ram" "$R/restored-gate/fm1-live-ba83317a" --bin "$D/$n-padded.bin" --entry 0x02000120 --mhz 360 --shot 500 > "$D/$n.log" 2>&1
done
sed -e "s/grab('fair')/grab('uncached')/" -e "s/grab('paired')/grab('cached')/" "$R/s1-morph-compact/emulated-lookup/score.py" > "$D/score.py"
python "$D/score.py" "$D" > "$D/result.json"
