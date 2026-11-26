#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd);R=$(realpath "$D/../..")
B="$R/rehydrated-reverb/delay-center-exact-wet/tree"
echo "d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625  $B/firmware/src/dsp/dnbx_core.c"|sha256sum -c
T=$(mktemp -d /tmp/arp-fold-XXXXXXXX)
echo "$T"
cp -as "$B"/* "$T/"
# Copy only modified regular files; all other repo source/asset reads remain original.
rm -r "$T/build";mkdir "$T/build"
ln -s "$R/sj6-compact-halfband" "$T/fold-proof"
cp --remove-destination "$D/fixture.c" "$T/tests/host/arp-fold-fixture.c"
sed -n -e '/^static int smk_pitch_parameter(int v)$/,/^}$/p' -e '/^static void smk_wheels(void)$/,/^}$/p' "$B/firmware/src/app/main_fm1.c" > "$T/tests/host/actual_smk_wheels.inc"
test -s "$T/tests/host/actual_smk_wheels.inc"
for name in smc_ui_test.c ui_arp_engine_test.c;do
 cp --remove-destination "$B/tests/host/$name" "$T/tests/host/$name"
 sed -i -E 's/pg.title\[([^]]+)\]/page_title(\&pg,\1)/g' "$T/tests/host/$name"
done
C="$T/firmware/src/dsp/dnbx_core.c"
E="$T/firmware/src/app/engine.c"
cp --remove-destination "$B/firmware/src/app/engine.c" "$E";cat "$D/observer.inc" >> "$E"
cp --remove-destination "$B/firmware/src/dsp/dnbx_core.c" "$C"
python "$R/sj6-compact-halfband/wire-fold-native-hoisted.py" "$C"
python "$R/sj6-compact-halfband/live/keep_clock-central.py" "$C"
sed -i '/^static float probe_shape(.*{$/a\    extern unsigned sj6_fold_test_samples; ++sj6_fold_test_samples;' "$C"
printf '\nunsigned sj6_fold_test_samples;\n' >> "$C"
F="$T/firmware/src/dsp/fold-data.h"
PYTHONPATH="$R/sj6-spectrum-anchor" python "$R/sj6-compact-halfband/fold.py" "$R/sj6-spectrum-anchor/a4.private.json" 32 8 > "$F"
cp "$R/sj6-wavetable-scope/selector.h" "$T/firmware/src/dsp/"
cd "$T"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_SYNTHONE_PATCH=1 X0X_S1C=1 X0X_ARP_HOOKS=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1
PIN="$R/all-controls-remote-join/compile-flags.txt";DEFS=$(tr "\n" " " < "$PIN")
for i in 0 1;do
 (X0X_HOST_DEFS="-DSJ6_WAVE_PROBE=$i $DEFS -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1" SMC_UI_TEST_FILE=tests/host/arp-fold-fixture.c SMC_UI_OUT="$T/run$i" sh tests/host/smc_ui_test.sh) >"$T/$i.log" 2>"$T/$i.err" || { tail -n 35 "$T/$i.log" "$T/$i.err";exit 1; }
done
cmp "$T/0.log" "$T/1.log"
tail -4 "$T/1.log";grep typed_fold_samples "$T"/{0,1}.err
