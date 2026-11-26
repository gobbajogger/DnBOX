#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd); cd "$D"
B=${1:-../sj6-hp-connected-zero/host-donor/firmware/src/dsp}
B=$(cd "$B" && pwd)
pin=$(cut -d' ' -f1 donor.pin); echo "$pin  $B/dnbx_core.c"|sha256sum -c
T=$(mktemp -d); trap 'rm -r "$T"' EXIT
cp "$B/dnbx_core.c" "$T/candidate.c"
python3 stage.py "$T/candidate.c"
diff -u --label a/firmware/src/dsp/dnbx_core.c --label b/firmware/src/dsp/dnbx_core.c "$B/dnbx_core.c" "$T/candidate.c" > candidate.patch || [[ $? == 1 ]]
sed -i '/if (osc_gain\[l\]!=1.f) x\*=osc_gain\[l\];/a\                extern float diagnostic_source[2][128];\n                diagnostic_source[l][k]+=velocity*amp*x;' "$T/candidate.c"
for m in 0 1; do
cc -O2 -std=c99 -ffp-contract=off -Wno-attributes -DX0X_HOST=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DDNBX_HOST_SJ6_SINE_PROFILE=$m -I"$B" "$T/candidate.c" run.c -lm -o "$T/run"
"$T/run" |tee "result-$m.txt"
done
python3 - <<'PY'
a=[r.split() for r in open("result-0.txt")];b=[r.split() for r in open("result-1.txt")]
assert len(a)==len(b)==22
g=1.2377909421920776
for i,(x,y) in enumerate(zip(a,b)):
    i%=11
    assert x[:4]==y[:4] and x[8:10]==y[8:10]
    if i in (1,2,3):
        for k in (4,5,6,10,11,12,13,14,15): assert abs(g*float(x[k])-float(y[k]))<1e-7
    elif i==9:
        assert abs(g*float(x[4])-float(y[4]))<1e-7
        assert x[5]==y[5]
    else:
        assert x==y,(x,y)
print('muted+silent, unprojected unity Sine, raw Sine legacy, triangle/saw/pulse byte-digest guards exact; mixed sibling saw RMS exact')
PY
