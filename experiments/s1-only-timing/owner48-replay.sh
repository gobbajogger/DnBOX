#!/bin/sh
# Replay only on the pinned emulator, using a fresh regular virtual NOR file.
set -eu
[ "$#" = 6 ] || { echo "Usage: $0 SOURCE MODEL NEW_OUT PINNED_EMULATOR CONTROLLER_TEMPLATE RECEIPTS_DIR" >&2; exit 2; }
src=$(realpath "$1")
case "$2" in elite) tag=smk37-elite;; pro) tag=smk37;; *) exit 2;; esac
emu=$(realpath "$4")
template=$(realpath "$5")
checks=$(realpath "$6")
[ "$2" != elite ] || [ "$(sha256sum "$template" | awk '{print $1}')" = f5f025e9a7d5984760f4dcef38a2b15986f8d7859282f231357fc0e2fbfdf99d ]
[ "$2" != pro ] || [ "$(sha256sum "$template" | awk '{print $1}')" = f188a7ebe611701ecb17f8ebee7ed36655c8d83d39e39312015f3d22b9523bb8 ]
[ -f "$emu" ] && [ -f "$template" ]
[ "$(sha256sum "$emu" | awk '{print $1}')" = fdd1504d8801a69a95c24226357f4b202bbaa345565c80a1bbf861f69a287017 ]
mkdir -- "$3"
cd "$3"
mkdir work
cp "$src/build/$tag/x0x.elf" bench.elf
PYTHONPATH="$src/tools" python3 - "$src/build/x0x-$tag.fwsc" "$template" <<'PY'
import sys
from pathlib import Path
import add_ctl
raw, template = (Path(p).read_bytes() for p in sys.argv[1:])
ctl = add_ctl.controller_of(add_ctl.logical_of(template)[1])
Path("bench.fwsc").write_bytes(add_ctl.add(raw, ctl))
PY
# Ignore caller overrides; there is no keyboard input tape, injected MIDI, or skip.
env -i PATH="$PATH" HOME="$PWD" TMPDIR="$PWD" FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM=ram.bin \
    "$emu" --fwsc bench.fwsc --mhz 360 --usb --shot 8000 --flash virtual-nor.bin > run.log 2>&1
[ "$(wc -c < ram.bin)" = 1048576 ]
grep -q 'skipped 0 cycles' run.log
python3 "$checks/read-loaded-timing.py" "$PWD" > timing.json
python3 "$checks/loaded-witness.py" "$PWD" > witness.txt
python3 "$checks/read-48-fit.py" "$PWD" > display.txt
cat timing.json witness.txt display.txt
