#!/bin/bash
# Source and link size only. Does not build or invoke the test harness or packaging.
set -euo pipefail
here=$(realpath "$(dirname "$0")")
if [[ -f "$here/SOURCE_ONLY" ]]; then
  cat "$here/SOURCE_ONLY" >&2
  exit 42
fi
if [[ -f "$here/OBJECTS_ONLY" && "${DNBOX_CONTROL_BUILD_ONLY:-0}" != 1 ]]; then
  echo "Fit is locked until both complete object builds succeed." >&2
  exit 42
fi
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
# Materialize the same source layout before either phase. In the objects-only
# phase the linker wrapper stops after compilation and never evaluates sizes.
python3 "$here/compact-sequencer-layout.py"
link="$here/all-controls-fit-first/link-fit.py"
if [[ "$profile" == slot3 ]]; then
 patch --fuzz=0 --no-backup-if-mismatch -s -p1 < "$here/combined-slot3-fit/slot3-fit-guard.patch"
 link="${FORWARD_SLOT3_LINK:-$here/combined-slot3-fit/link-slot3.py}"
 if [[ "${DNBOX_FITTAB_TRIAL:-0}" == 1 ]]; then
   patch --fuzz=0 --no-backup-if-mismatch -s -p1 < "$here/fittab-guard.patch"
   link="$here/link-fittab-forward.py"
 fi
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
 "-DX0X_SJ6_LFO1_WAVE_NATIVE=1",
 "-DX0X_SJ6_LFO_TRIGGER_NATIVE=1",
 "-DX0X_S1_KEYBOARD_HOLD=1",
 "-DX0X_S1_SCALE_NATIVE=1", "-DX0X_SJ6_SEQ_DEST_NATIVE=1",
 "-DX0X_S1_USER_SCALE=1", "-DX0X_S1_HALFTIME=1", "-DX0X_SJ6_AUX_SEQ_NATIVE=1",
 "-DX0X_SJ6_ARP_ADAPTER_NATIVE=1",
 "-DX0X_S1_DELAY_HISTORY_NATIVE=1",
 "-DX0X_SJ6_RANDOM5_6_NATIVE=1", "-DX0X_SJ6_VINTAGE_NATIVE=1", "-DX0X_SJ6_SCALE_NATIVE=1", "-DX0X_SJ6_VELOCITY_SETTINGS=1",
 "-DX0X_MIX_DELAY_PACK14=1",
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
if [[ "${DNBOX_FITTAB_EXTRA_COLD:-0}" == 1 ]]; then
  test "${DNBOX_FITTAB_TRIAL:-0}" = 1
  test "${DNBOX_CONTROL_BUILD_ONLY:-0}" != 1
  test "$link" = "$out/combined-link.py"
  (cd "$out"; patch --fuzz=0 --no-backup-if-mismatch -s -p1 < "$root/recovery/s1-cold-fit/guarded-cold-tables-only.patch")
fi
if [[ "${DNBOX_FITTAB_XDATA_TAIL:-0}" == 1 ]]; then
  test "$link" = "$out/combined-link.py"
  patch --fuzz=0 --no-backup-if-mismatch -s -p1 < "$root/recovery/cold-layout-fit-complete/xdata-tail-after-fittab.patch"
  python3 "$root/recovery/cold-layout-fit-complete/extend-link.py" "$link" "$out/combined-link.xdata.py"
  link="$out/combined-link.xdata.py"
fi
export JIELI_TOOLCHAIN="$root/prereqs/jieli-linux-toolchains-20260730.1" PYTHONPATH="$out/src" D9_LAB_OUT="$out/src/labout" PYTHONDONTWRITEBYTECODE=1
export X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_S1C=1 X0X_SYNTH_CONTROLS_INIT_ONLY=1 X0X_SYNTHONE_PATCH=1
for model in ${BUILD_MODELS:-elite pro}; do
 case "$model" in elite) board=smk37-elite;;pro) board=smk37;;*) exit 2;; esac
 src="$root/fw/build/$board/gen"; test -s "$src/x0x_drum_samples.h"
 mkdir -p "build/$board"; cp -a "$src" "build/$board/"
 python "$link" --board smk37 --model "$model" --sdk "$root/prereqs/sdk" >"$out/$model-link.log" 2>&1 || { tail -35 "$out/$model-link.log"; exit 1; }
 if [[ "${DNBOX_CONTROL_BUILD_ONLY:-0}" == 1 ]]; then
  grep '^compiled-controls' "$out/$model-link.log"
 else
  grep '^guarded' "$out/$model-link.log"
 fi
done
