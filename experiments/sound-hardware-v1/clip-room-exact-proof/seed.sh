#!/usr/bin/env bash
# Only a local synthetic emulator image; never installs or connects to hardware.
set -euo pipefail
source_root=$(realpath "${1:?source root}")
model=${2:?elite or pro}
pkg=$(realpath "${3:?native package matching its source root/model}")
out=$(realpath -m "${4:?new scratch image}")
case "$model" in pro) tag='';; elite) tag='-elite';; *) exit 2;; esac
PYTHONPATH="$source_root/tools" python3 - "$pkg" "$out" "$source_root/build/smk37$tag" <<'PY'
from pathlib import Path
import sys
from fm1_rescue import package_flash_image
p,f,s=map(Path,sys.argv[1:])
data=package_flash_image(p.read_bytes())
assert len(data)==0x9d000
nor=bytearray(data+b'\xff'*(0x100000-len(data)))
for name, start, length in [('xdata',0xad000,8192),('fdata',0xe5000,61440),('fittab',0xfe000,4096)]:
    image=(s/(name+'.img')).read_bytes()
    assert len(image)==length
    nor[start:start+length]=image
nor[0xaf000:0xaf010]=b'SMK37DATAX0X\0\0\0\0'
with f.open('xb') as fd:
    fd.write(nor)
PY
