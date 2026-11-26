#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); R=$(dirname "$D")
T=$(mktemp -d);trap 'rm -rf "$T"' EXIT
(cd "$R";sha256sum -c "$D/source.sha256" >&2)
APP="$R/s1-v8-font-exact/tree/firmware/src/app"
cc -std=c99 -O2 -I"$APP" "$D/proof.c" -o "$T/proof"
"$T/proof"
