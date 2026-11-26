#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); R=$(dirname "$D")
BASE=${1:-$R/sj6-hp-connected-zero/host-donor}
T=$(mktemp -d); trap 'rm -r "$T"' EXIT
mkdir -p "$T/firmware/src/dsp"
cp "$BASE/firmware/src/dsp/"dnbx_core.{c,h} "$T/firmware/src/dsp/"
(cd "$T";sha256sum -c "$D/preimages.txt";patch -s --fuzz=0 -p1 < "$D/candidate.patch")
for s in 0 1;do
 cc -std=c99 -O2 -ffp-contract=off -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DDNBX_HOST_SJ6_FIXED_SOURCE_ROUTE="$s" -I"$T/firmware/src/dsp" -I"$BASE/firmware/src/dsp" "$D/probe.c" "$T/firmware/src/dsp/dnbx_core.c" -lm -o "$T/probe"
 "$T/probe"
done
