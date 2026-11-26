#!/bin/bash
set -euo pipefail
src=${1:?pass the pinned authorized DnBOX donor dnbx_core.c}
headers=${2:?pass the matching DnBOX include directory}
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
out=$(mktemp -d "${TMPDIR:-/tmp}/verify-sj6-reducer.XXXXXXXX")
trap 'rm -rf "$out"' EXIT
python3 "$here/instrument.py" "$src" "$out/tapped.c"
cc -std=c99 -O2 -ffp-contract=off -DX0X_HOST -DX0X_SUPERJ6_BUILTIN_INIT=1 -I "$headers" "$out/tapped.c" "$here/probe.c" -o "$out/probe"
"$out/probe"
