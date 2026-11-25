#!/bin/bash
# Source and link size only. Does not build or invoke the test harness or packaging.
set -euo pipefail
here=$(realpath "$(dirname "$0")")
root=$(realpath "$1"); donor=$(realpath "$2"); out=$(realpath "$3")
profile=${4:-slot3}
case "$profile" in fdata|slot3) ;; *) exit 2;; esac
manifest="$here/sj6-native-voice-delay-velocity/FIRMWARE.sha256"
if [[ -n "${FORWARD_MANIFEST:-}" ]]; then
 manifest=$(realpath "$FORWARD_MANIFEST")
 echo "Explicit forward source: $(sha256sum "$manifest" | cut -d' ' -f1); historical link receipts do not qualify it" >&2
fi
(cd "$donor"; sha256sum --quiet -c "$manifest")
test ! -e "$out/src"
mkdir -p "$out/src"
for f in firmware experiments tools; do cp -a "$(realpath "$donor/$f")" "$out/src/"; done
for f in assets ble; do ln -s "$(realpath "$donor/$f")" "$out/src/$f"; done
cd "$out/src"
patch --fuzz=0 --no-backup-if-mismatch -s -p1 < "$here/all-controls-fit-first/params-layout.patch"
link="$here/all-controls-fit-first/link-fit.py"
if [[ "$profile" == slot3 ]]; then
 patch --fuzz=0 --no-backup-if-mismatch -s -p1 < "$here/combined-slot3-fit/slot3-fit-guard.patch"
 link="${FORWARD_SLOT3_LINK:-$here/combined-slot3-fit/link-slot3.py}"
fi
if [[ -n "${ADD_CONTROL_FLAGS:-}" ]]; then
 python3 - "$link" "$out/combined-link.py" "$ADD_CONTROL_FLAGS" <<'PYFLAGS'
import re,sys
from pathlib import Path
source,path,flags=sys.argv[1:]
flags=flags.split()
allowed={
 "-DX0X_SJ6_NATIVE_ZERO_GATE=1", "-DX0X_S1_CLOCK_NATIVE=1",
 "-DX0X_S1_FENCED_CLOCK=1", "-DX0X_S1_CRUSH_LFO_PROBE=1",
 "-DX0X_S1_WET_GAIN_TEST=1", "-DX0X_SJ6_VCF_PORT=1",
 "-DX0X_S1_PHASER_NATIVE_TEST=1",
 "-DX0X_S1_DELAY_TIME_NATIVE=1", "-DX0X_SJ6_NATIVE_DELAY=1",
 "-DX0X_SJ6_LFO1_DELAY_PORT=1", "-DX0X_SJ6_NATIVE_UNISON=1", "-DX0X_SJ6_PITCH_ENV_NATIVE=1",
 "-DX0X_SJ6_BEND_TYPED_PROBE=1", "-DX0X_SJ6_MOD_TYPED_PROBE=1", "-DX0X_SJ6_ALT3_NATIVE=1",
 "-DX0X_SJ6_NATIVE_ROOM=1",
 "-DX0X_SJ6_NATIVE_J60=1", "-DX0X_SJ6_AUX_LFO_NATIVE=1", "-DX0X_SJ6_NOISE_SUB_NATIVE=1",
 "-DX0X_SJ6_OTA_NATIVE=1",
 "-DX0X_SJ6_NATIVE_LIMITER=1", "-DX0X_SJ6_NATIVE_CLASSIC=1", "-DX0X_SJ6_SEQ_SETTINGS_NATIVE=1",
}
if not all(x in allowed for x in flags):
 raise SystemExit("Unknown control flag; validate the port before adding it to this wrapper")
s=Path(source).read_text()
anchor="b.generate=lambda:None"
if s.count(anchor)!=1: raise SystemExit("Unknown linker wrapper")
Path(path).write_text(s.replace(anchor,"b.CFLAGS.extend("+repr(flags)+")\n"+anchor,1))
PYFLAGS
 link="$out/combined-link.py"
fi
export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$out/src" D9_LAB_OUT="$out/src/labout" PYTHONDONTWRITEBYTECODE=1
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
for model in elite pro; do
 case "$model" in elite) board=smk37-elite;;pro) board=smk37;; esac
 src="$root/fw/build/$board/gen"; test -s "$src/x0x_drum_samples.h"
 mkdir -p "build/$board"; cp -a "$src" "build/$board/"
 python "$link" --board smk37 --model "$model" --sdk "$root/prereqs/sdk" >"$out/$model-link.log" 2>&1 || { tail -35 "$out/$model-link.log"; exit 1; }
 grep '^guarded' "$out/$model-link.log"
done
