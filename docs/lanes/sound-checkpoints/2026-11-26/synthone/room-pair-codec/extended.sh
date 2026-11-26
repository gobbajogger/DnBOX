#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd)
[[ $# == 1 && "$1" =~ ^[0-9]+-[0-9]+-[0-9]+(-cubic)?$ ]] || exit 2
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
L=$(python "$D/holdouts.py")
python - "$D/donor.sh" "$T/run.sh" "$D" "$L" "$1" <<'PY'
from pathlib import Path
import re,sys
f,o,d,ids,variant=sys.argv[1:]
s=Path(f).read_text()
s=s.replace('D=$(cd "$(dirname "$0")";pwd)',f'D={d}')
s=re.sub(r'--ids [\d ]+ --jobs 2','--ids '+ids+' --jobs 3',s)
s=re.sub(r'^for spec in .*;do$',f'for spec in {variant};do',s,flags=re.M)
Path(o).write_text(s)
PY
bash "$T/run.sh"
