#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
export D9_LAB_S1_FDATA=1 DNBOX_CHICAGO_MOD_PALETTE=1 DNBOX_MIAMI_STOCK_CAP_COMPACT=1
python3 ../guarded-fit-miami-cap/gen-cap.py tree
exec bash replay-maps.sh
