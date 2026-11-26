#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd);cd "$D"
B=${1:-../sj6-hp-connected-zero/host-donor/firmware/src/dsp}
B=$(cd "$B"&&pwd)
T=$(mktemp -d); trap 'rm -r "$T"' EXIT
mkdir "$T/dsp"
for f in dnbx_core.c dnbx_core.h juno6.c juno6_factory.inc;do cp "$B/$f" "$T/dsp/";done
cp -as "$B/." "$T/dsp" 2>/dev/null || true
(cd "$B";sha256sum -c "$D/preimages")
python3 patch.py "$T/dsp"
for f in dnbx_core.c dnbx_core.h juno6_factory.inc;do diff -u --label "a/firmware/src/dsp/$f" --label "b/firmware/src/dsp/$f" "$B/$f" "$T/dsp/$f" || [[ $? == 1 ]];done > candidate.patch
sed -i '/s->sj6_mode_test.in_tile=(uint8_t)/a\            extern void diagnostic_mode(unsigned,unsigned,float);\n            diagnostic_mode(k,s->sj6_ota,g);' "$T/dsp/dnbx_core.c"
sed -i '/s->sj6_ota=s->sj6_mode_test.request;/,+4{ /lv->filter_first=lv->filter_second=lv->ota_tail/!b; n; a\            extern void diagnostic_reset(const dnbx_part *);diagnostic_reset(s);
}' "$T/dsp/dnbx_core.c"
for v in 0 1;do
 cc -O2 -std=c99 -ffp-contract=off -Wno-attributes -ffunction-sections -fdata-sections -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DDNBX_HOST_SJ6_FILTER_MODE=$v -I"$T/dsp" lens.c "$T/dsp/dnbx_core.c" "$T/dsp/juno6.c" -Wl,--gc-sections -lm -o "$T/lens"
 "$T/lens" |tee "host-$v.txt"
done
