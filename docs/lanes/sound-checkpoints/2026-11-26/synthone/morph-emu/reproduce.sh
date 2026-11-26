#!/bin/bash
# Emulator-only raw images. Do not package, install, or publish them.
set -euo pipefail
R=$(pwd); G=$R/s1-morph-compact; D=$G/emulated-lookup; A=$R/recovered-sound/s1-room-parity
T=$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2
H=$G/private/shared-tight
python - "$D" <<'PY'
from pathlib import Path
import numpy as np,sys
d=Path(sys.argv[1]);w=np.fromfile('s1-fx-parity/reference-build/factory-inputs/public-morphs.f32','<f4').reshape(13,4,4097)
with (d/'raw-waves.f32').open('wb') as f:
 for i in (7,8,12):f.write(w[i,:,:4096].tobytes())
(d/'arrays.S').write_text('.section .rodata.fixture,"a",@progbits\n.p2align 2\n.globl direct_wave\ndirect_wave:\n.incbin "'+str(d/'raw-waves.f32')+'"\n')
PY
F=(-target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$T/include")
for f in start.S arrays.S bench.c direct.c;do
 "$T/bin/clang" "${F[@]}" -I"$R/fw/firmware/hal" -I"$G/phase-round" -c "$D/$f" -o "$D/${f%.*}.o"
done
for n in fair paired;do
 extra=(); [[ $n == fair ]] || extra=(-DDNBX_MORPH_PAIR=1)
 "$T/bin/clang" "${F[@]}" -I"$A" -I"$G" -I"$H" -DDNBX_MORPH_TARGET_FRAGMENT "${extra[@]}" -c "$G/phase-round/morph-phase-bounds.c" -o "$D/$n-morph.o"
 "$T/bin/ld" --gc-sections -T "$D/bench.ld" "$D/"{start,arrays,bench,direct}.o "$D/$n-morph.o" -o "$D/$n.elf"
 "$T/bin/objcopy" -O binary -j .text "$D/$n.elf" "$D/$n.bin"
 python - "$D/$n" <<'PY'
from pathlib import Path
import sys
p=sys.argv[1];Path(p+'-padded.bin').write_bytes(bytes(0x120)+Path(p+'.bin').read_bytes())
PY
done
cd "$D"; mkdir -p work
exec 9>/tmp/dnbox-gate-firmware.lock; flock -n 9
for n in fair paired;do
 FM1_STRICT=1 FM1_NO_SKIP=1 FM1_NO_PLAY=1 FM1_DUMPRAM="$D/$n.ram" "$R/restored-gate/fm1-live-ba83317a" --bin "$D/$n-padded.bin" --entry 0x02000120 --mhz 360 --shot 500 > "$D/$n.log" 2>&1
done
python "$D/score.py" "$D"
