#!/bin/sh
set -eu
root=$(realpath "$1"); w=$(realpath "$2"); seed=$(realpath "$3"); carrier=$(realpath "$4"); m=$5;o=$6
d=$(realpath "$(dirname "$0")")
keys=$(python3 - "$o" <<'PY'
import sys
o=int(sys.argv[1]);a=['2350:e2:+4','2600:e2:+4']
if o:a+=['2780:e2:-2']
for i in range(18):
 t=2900+110*i
 a += [f'{t}:PARA',f'{t+45}:PARA^']
a+=['4980:e4:+4','5110:e4:+4','5240:e5:+4','5370:e5:+4','5510:k14@121']
print(','.join(a))
PY
)
python3 "$d/native-pan.py" "$root" "$w" "$seed" "$carrier/virtual-store-readonly-autopan.bin" "$m" 5850 "$keys" "$o"
