#!/bin/bash
# Read-only simulator timing and word digest. Never enable on any physical device.
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
R=$(dirname "$D")
M="${1:?elite or pro}"
[[ $M == elite || $M == pro ]]
V=${2:?0 or 1}; E="$D/native-$M-$V/out/$M/never-install.elf"
P="$R/out/DnBOX-0.0.27-SMK37-$M-73749f55-test.fwsc"
PIN=$(sed -n 's/^guarded .* ELF \([0-9a-f]\{64\}\)$/\1/p' "$D/$M$V.log")
[[ $PIN =~ ^[0-9a-f]{64}$ ]]
printf '%s  %s\n' "$PIN" "$E"|sha256sum -c --status
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -r "$T"' EXIT
Z="$T/virtual-nor.bin"
python3 - "$R" "$M" "$P" "$E" "$Z" "$PIN" <<'PY'
from pathlib import Path
import importlib.util,sys
r,model,p,e,z,pin=sys.argv[1:];r=Path(r)
sys.path.insert(0,str(r/'restored-harness-inspect/helpers'))
import guarded_replay as g
spec=importlib.util.spec_from_file_location('seed_reader',r/'fw/tools/fm1_rescue.py')
loader=importlib.util.module_from_spec(spec);spec.loader.exec_module(loader)
seed,_,_=g.project(loader,model,Path(p),Path(e),pin)
assert len(seed)==0x100000
# Use existing synthetic S1 project envelope so the firmware sees an already
# provisioned virtual store and does not initialize flash sectors during boot.
authored=g.regular(r/'recovered-sound/fresh-h16-replay/files/sealed'/f'{model}-h16-virtual-nor.bin')
assert g.sha(authored)=={'elite':'d9953d21980f6ae7dc26296c8b513618f11389db87beb54430ff7c2d3377ff31',
                        'pro':'487e879c06aa19fc67ed809f6cbfa15fc1d80759b90326c2e0c3cc39b3271600'}[model]
out=bytearray(seed)
for i in (0,1,2,3,5,6):
    o=0x9d000+8192*i;n=g.u32(authored,o+12)
    assert 4<=n<=3840
    out[o:o+32]=authored[o:o+32];out[o+256:o+256+n]=authored[o+256:o+256+n]
out[0xaf000:0xaf010]=authored[0xaf000:0xaf010]
with Path(z).open('xb') as f:f.write(out)
PY
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9
cd "$T"
env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM="$T/ram.bin" "$R/restored-gate/fm1-live-ba83317a" \
    --fwsc "$P" --mhz 360 --shot 3200 --flash "$Z" > run.log 2>&1
python3 - "$R" "$E" "$T/ram.bin" <<'PY'
from pathlib import Path
import json,struct,sys
r,e,p=sys.argv[1:];sys.path.insert(0,str(Path(r)/'restored-harness-inspect/helpers'))
from guarded_replay import ELF_RAM,symbols_and_sections
_,s,_=symbols_and_sections(Path(e))
m,osize,psize,*runs=struct.unpack_from('<19I',Path(p).read_bytes(),s['sj6_route_cue_receipt']-ELF_RAM)
assert m==0x535a4658,(hex(m),osize,psize)
for k,name in enumerate(('steady_full','low_fade_release','one_31_prefix_rewrite','recurrent_196_each_group')):
    t,mx,mn,d=runs[k*4:k*4+4]
    assert mn>0 and mn<=mx and t>0
    print(json.dumps(dict(fixture=name,owner_size=osize,part_size=psize,timer_hz=24000000,
            frames=256,owners=2,mean_us=t/(24*8),max_us=mx/24,min_us=mn/24,digest=f'{d:08x}')))
PY
grep -Fx 'flash: 0 erases / programs' run.log
