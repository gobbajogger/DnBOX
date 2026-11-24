#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Every reachable firmware screen renders: ui_screen_walk.c must finish, and the WALK
# inventory must equal tests/ui_defects/first_slice/screen_inventory.json (1545 states).
# Render presence only; not visual approval. The gate runs the same walk in ui-defects.
set -eu
cd "$(dirname "$0")/../.."
out=build/host/ui-screen-walk
rm -rf "$out"
python3 - "$out" <<'PY'
import sys
from pathlib import Path
sys.path.insert(0, 'tests/ui_defects/first_slice')
import screen_walk
screen_walk.run(Path('.'), Path(sys.argv[1]))
PY
