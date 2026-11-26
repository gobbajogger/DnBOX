#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
R=$(cd "$D/../.." && pwd)
V="${1:?0 or 1}"
M="${2:?elite or pro}"
ELF="$D/$V-$M/out/$M/never-install.elf"
PIN=$(sed -n 's/^guarded .* ELF \([0-9a-f]\{64\}\)$/\1/p' "$D/$V-$M.log")
[[ $PIN =~ ^[0-9a-f]{64}$ ]]
printf '%s  %s\n' "$PIN" "$ELF"|sha256sum -c --status
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -r "$T"' EXIT
S="$T/$M-virtual-nor.bin"
python3 - "$R" "$D" "$M" "$ELF" "$PIN" "$S" <<'PY'
from pathlib import Path
import sys
r,d,model,elf,pin,seed=sys.argv[1:]
origin=Path(r)/'s1-morph-native-nosplit/replay-overlay.py'
source=origin.read_text().replace('r=d.parent','r=Path(sys.argv[1])')
a=source.index('pin=')
b=source.index('old=',a)
source=source[:a]+f'pin={{{model!r}:{pin!r}}}\n'+source[b:]
sys.argv=[sys.argv[0],r,'make',model,elf,pin,seed]
source=source.replace('verb,model,elf,expected,seed=sys.argv[1:]','verb,model,elf,expected,seed=sys.argv[2:]')
exec(compile(source,str(origin),'exec'),{'__file__':str(origin)})
PY
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
cd "$T"
export DNR3_WEB="$R/remote206-typed/remote/dnr3.mjs"
usb=$(node "$R/recovered-heavy-replay/owner-scan.mjs" commands)
keys=$(tr -d '\n' < "$R/fw/experiments/sound-hardware-v1/strict-timing/heavy.keys")
env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_BENCH_KEYS="$keys" FM1_USB_SEND="$usb" FM1_USB_TRACE=1  FM1_DNBOX_PCM="$T/audio.s16le" FM1_DUMPRAM="$T/ram.bin" "$R/restored-gate/fm1-live-ba83317a"  --fwsc "$R/out/DnBOX-0.0.27-SMK37-$M-73749f55-test.fwsc" --mhz 360 --usb --shot "${DNBOX_INTEROP_SHOT:-8000}"  --flash "$S" > run.log 2>&1
node "$R/recovered-heavy-replay/owner-scan.mjs" receipt "$T/run.log" | tee owner.json
python3 "$R/recovered-heavy-replay/probe.py" "$ELF" "$T"
grep '^flash:' run.log
