#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
export D9_LAB_SPARE_TABLES=1 DNBOX_MIAMI_DECIMATOR_COMPACT=1
python3 ../guarded-fit-miami-decimator/gen-decimator.py tree
exec bash replay-all-maps.sh
