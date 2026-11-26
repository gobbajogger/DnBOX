#!/bin/bash
set -euo pipefail
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/sj6-hp-leaves.XXXXXXXX")
trap 'rm -rf "$tmp"' EXIT
if [[ "$(uname -s)" == Darwin ]]; then shared=-dynamiclib; else shared=-shared; fi
cc -O2 -fPIC "$shared" -ffp-contract=off "$here/leaf.c" -o "$tmp/leaf.so"
python3 -B "$here/verify.py" "$tmp/leaf.so" "$here/chain" "$here/stems"
