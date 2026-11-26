#!/bin/bash
set -eu
D=$(cd "$(dirname "$0")" && pwd);R=$(dirname "$D")
M=${1:?target model}
P="$R/rehydrated-reverb/delay-center-exact-wet"
V=${2:?0 or 1}
T="/tmp/dnbox-sj6-hbp-native-$M-$V"
mkdir -p "$T"
for dir in firmware tools assets build ble reference;do cp --reflink=auto -a "$P/tree/$dir" "$T/";done
cp -a "$P/tree/experiments" "$T/"
cd "$T"
B=firmware/src/dsp
echo "d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625  $B/dnbx_core.c"|sha256sum -c
cp "$D/native-full-probe.inc" firmware/src/app/native-probe.inc
if [ "$V" = 6 ]; then
 sed -i '1i #define SJ6_WAVE_PROBE 1' "$B/dnbx_core.c"
 python "$D/wire-fold-native.py" "$B/dnbx_core.c"
 cp "$D/../sj6-wavetable-scope/selector.h" "$B/"
 PYTHONPATH="$D/../sj6-spectrum-anchor" python "$D/fold.py" "$D/../sj6-spectrum-anchor/a4.private.json" 32 8 > "$B/fold-data.h"
fi
python3 - <<'PY'
from pathlib import Path
p=Path('firmware/src/app/original_runtime.c')
s=p.read_text();marker='static int render_backend(original_runtime_part *p, original_runtime_workspace *w,'
at=s.rindex(marker)
p.write_text(s[:at]+'#include "native-probe.inc"\n'+s[at:])
e=Path('firmware/src/app/engine.c')
x=e.read_text()
marker='X0X_COLD void engine_init(pattern_t *patterns, song_t *song, lane_t *lanes, const sound_t *base)\n{'
assert x.count(marker)==1
e.write_text(x.replace(marker,marker+'\n extern void sg6_zero_signed_cue(original_runtime_part *,original_runtime_part *,float (*)[256]);\n sg6_zero_signed_cue(&original_bass[0],&original_bass[1],eng_bass_buf);'))
PY
cat "$R/all-controls-remote-join/compile-flags.txt" > flags.txt
:
export JIELI_TOOLCHAIN="$R/prereqs/jieli-linux-toolchains-20260730.1" AC79_SDK="$R/prereqs/sdk" PYTHONPATH="$T"
export DNBOX_CONTROL_FLAGS="$T/flags.txt" D9_LAB_OUT="$T/out"
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1 X0X_ARP_HOOKS=1 X0X_REMOTE_DNR3=1 X0X_REMOTE_TYPED_SYNTH=0
export D9_LAB_XDATA_TAIL=1 D9_LAB_SLOT3_METADATA=1 DNBOX_TANH_SPLIT_LAB=1 DNBOX_TANH_REFILL_LAB=1 D9_LAB_S1_FDATA=1 DNBOX_CHICAGO_MOD_PALETTE=1 DNBOX_MIAMI_STOCK_CAP_COMPACT=1 D9_LAB_SPARE_TABLES=1 DNBOX_MIAMI_DECIMATOR_COMPACT=1 DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1
# A deliberately diagnostic-only image: no browser Remote synth controls.
# Do not require that absent metadata in the emulator-only map verifier.
sed '/\.rodata\.rd_panel_sj6_ref\.fixed/d' "$P/link-guarded-exact.py" > "$T/linked-emulator-only.py"
python3 "$T/linked-emulator-only.py" --board smk37 --model "$M" --sdk "$AC79_SDK"
