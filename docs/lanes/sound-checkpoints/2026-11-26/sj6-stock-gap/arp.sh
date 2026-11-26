#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd);R=$(realpath "$D/.."); A="$R/sj6-compact-halfband/arp"
B="$R/rehydrated-reverb/delay-center-exact-wet/tree"
echo "d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625  $B/firmware/src/dsp/dnbx_core.c"|sha256sum -c
T=$(mktemp -d "$D/arp.XXXXXX"); export TMPDIR="$T"; trap 'rm -rf "$T"' EXIT
echo "$T"
cp -as "$B/firmware" "$T/"
for q in assets reference host tools experiments; do ln -s "$B/$q" "$T/$q"; done
mkdir "$T/build" "$T/tests"
cp -as "$B/tests/host" "$T/tests/"
# Debug info is not consumed by the fixture; avoid a large local binary.
cat > "$T/cc-thin" <<'THIN'
#!/bin/bash
a=(); for i in "$@"; do [[ $i == -g ]] || a+=("$i"); done
exec /usr/bin/cc "${a[@]}"
THIN
chmod 700 "$T/cc-thin"
export CC="$T/cc-thin"
ln -s "$R/sj6-compact-halfband" "$T/fold-proof"
cp --remove-destination "$A/fixture.c" "$T/tests/host/arp-fold-fixture.c"
rm -f "$T/tests/host/actual_smk_wheels.inc"
sed -n -e '/^static int smk_pitch_parameter(int v)$/,/^}$/p' -e '/^static void smk_wheels(void)$/,/^}$/p' "$B/firmware/src/app/main_fm1.c" > "$T/tests/host/actual_smk_wheels.inc"
test -s "$T/tests/host/actual_smk_wheels.inc"
for name in smc_ui_test.c ui_arp_engine_test.c;do
 cp --remove-destination "$B/tests/host/$name" "$T/tests/host/$name"
 sed -i -E 's/pg.title\[([^]]+)\]/page_title(\&pg,\1)/g' "$T/tests/host/$name"
done
C="$T/firmware/src/dsp/dnbx_core.c"
E="$T/firmware/src/app/engine.c"
cp --remove-destination "$B/firmware/src/app/engine.c" "$E"; cat "$A/observer.inc" >> "$E"
cp --remove-destination "$B/firmware/src/dsp/dnbx_core.c" "$C"
printf '
unsigned sj6_fold_test_samples;
' >> "$C"
for q in juno6.h juno6.c; do cp --remove-destination "$B/firmware/src/dsp/$q" "$T/firmware/src/dsp/$q"; done
python3 "$D/wire.py" "$T/firmware/src/dsp/juno6.h" "$T/firmware/src/dsp/juno6.c"
cd "$T"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_SYNTHONE_PATCH=1 X0X_S1C=1 X0X_ARP_HOOKS=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1
PIN="$R/all-controls-remote-join/compile-flags.txt";DEFS=$(tr "\n" " " < "$PIN")
for i in 0 1;do
 (X0X_HOST_DEFS="-DSJ6_WAVE_PROBE=0 -DDNBX_SJ6_RECLAIM_STOCK_LAYOUT=$i $DEFS -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1" SMC_UI_TEST_FILE=tests/host/arp-fold-fixture.c SMC_UI_OUT="$T/run$i" sh tests/host/smc_ui_test.sh) >"$T/$i.log" 2>"$T/$i.err" || { tail -n 35 "$T/$i.log" "$T/$i.err";exit 1; }
rm -f "$T/run$i/test"
done
cmp "$T/0.log" "$T/1.log"
sha256sum "$T"/{0,1}.log; wc -l "$T"/{0,1}.log; tail -4 "$T/1.log";grep typed_fold_samples "$T"/{0,1}.err
