#!/usr/bin/env bash
set -euo pipefail
HERE=$(cd "$(dirname "$0")" && pwd)
SRC=${1:?stock source tree}; package=${2:?raw model fwsc}; ctl=${3:?model controller}; out=${4:?new output path}; elf=${5:?matching elf}
seed=${SEED:?set a clean native emulator NOR seed with stock companions}
emu=${EMULATOR:-/workspace/scratch/3a7051e761a0/restored-gate/fm1-live-ba83317a}
project=${PROJECT:-$HERE/project.bin}
mkdir "$out"
out=$(realpath "$out")
sha256sum "$package" "$ctl" "$seed" "$emu" "$project" "$elf" > "$out/inputs.sha256"
PYTHONPATH="$SRC/tools:$SRC/experiments/sound-hardware-v1/selected-native" python - "$out" "$package" "$ctl" "$seed" "$project" <<'PY'
import sys,add_ctl
from pathlib import Path
from fm1_rescue import package_flash_image
import run
o,p,c,s,j=map(Path,sys.argv[1:])
fw=add_ctl.add(p.read_bytes(),c.read_bytes())
(o/'bench.fwsc').write_bytes(fw)
s=bytearray(s.read_bytes()); p=package_flash_image(fw)
s[:len(p)]=p
s[0x9d000:0xab000]=b'\xff'*0xe000
run.install_project(s,j.read_bytes())
(o/'flash.bin').write_bytes(s)
PY
(cd "$out"
 env -i PATH="$PATH" HOME="$PWD" TMPDIR="$PWD" FM1_STRICT=1 FM1_BENCH_KEYS="3550:SEQPLAY,3600:SEQPLAY^" FM1_NO_SKIP=1 FM1_DUMPRAM=ram.bin FM1_USB_SEND="${MIDI-4400:913058;4403:913458;4406:913758;4409:913b58;4412:913e58;4415:914358}" "$emu" --fwsc bench.fwsc --mhz 360 --usb --lcd-only --shot 5300 --record - --flash flash.bin >/dev/null 2>run.log
)
