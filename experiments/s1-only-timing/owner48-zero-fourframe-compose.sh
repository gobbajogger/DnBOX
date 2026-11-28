#!/bin/sh
set -eu
[ "$#" = 2 ] || { echo "Usage: $0 REPOSITORY NEW_SCRATCH_DIR" >&2; exit 2; }
F=$(CDPATH='' cd -- "$(dirname -- "$0")" && pwd)
sh "$F/owner48-zero-compose.sh" "$@"
cd "$2/source"
for p in owner48-zero-ima-transitions owner48-zero-break-early owner48-peak-skip-forced-planner \
  owner48-zero-bfx-map owner48-zero-schmitt-lean
do
    git apply "$F/$p.patch"
done
