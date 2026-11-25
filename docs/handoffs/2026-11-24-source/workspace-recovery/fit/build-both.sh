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
 link="$here/combined-slot3-fit/link-slot3.py"
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
