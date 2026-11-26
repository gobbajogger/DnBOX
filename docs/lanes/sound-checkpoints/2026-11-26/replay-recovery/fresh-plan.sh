#!/bin/bash
# Rebuild a fresh *plan-only* directory, not an installable package.
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
if [[ $# != 3 ]]; then echo "usage: $0 BASE_GIT_CHECKOUT INPUTS_ARCHIVE EMPTY_DEST" >&2; exit 2; fi
F=$(realpath "$1"); A=$(realpath "$2"); R=$(realpath -m "$3"); mkdir -p "$R"
[[ -z $(ls -A "$R") ]] || { echo "destination not empty" >&2; exit 1; }
a=e7eff3df1fcd2530d8de421ed4f4724b9ecf3df69c9d1dcc689549a4a3551732
printf '%s  %s\n' "$a" "$A" | sha256sum -c --status -
tar xf "$A" -C "$R"
(cd "$R" && sha256sum -c --status SHA256SUMS)
cp -a "$D/git-deps/." "$R/"
base=73749f55d17a0643f1ddd072114172d29eecc2ae
mkdir -p "$R/fw" "$R/restored-gate" "$R/all-controls-remote-join"
git -C "$F" archive "$base" tools/fm1_rescue.py experiments/sound-hardware-v1/strict-timing/heavy.keys | tar xf - -C "$R/fw"
cp "$D/compile-flags.txt" "$R/all-controls-remote-join/"
unzip -p "$R/restored-harness-inspect/dot-gate-harness.zip" fm1-live-ba83317a >"$R/restored-gate/fm1-live-ba83317a"
chmod 755 "$R/restored-gate/fm1-live-ba83317a"
T=$(mktemp -d /tmp/dnbox-emulator-only-XXXXXXXX)
trap 'rm -rf "$T"' EXIT
for model in elite pro; do
    if [[ "$model" == elite ]]; then
        h=d6c0611a98d75755f2e57b2562fef99f4d4ae671652debcbc3c84469dd2b16b0
        s=d9953d21980f6ae7dc26296c8b513618f11389db87beb54430ff7c2d3377ff31
    else
        h=af6c3b4dcd3661210bfcd425844bfd169a0fceccdc8760d377187a05092a86a2
        s=487e879c06aa19fc67ed809f6cbfa15fc1d80759b90326c2e0c3cc39b3271600
    fi
    cp "$R/sealed/$model-h16-virtual-nor.bin" "$T/$model-widen-room.bin"
    bash "$R/widen-native-verification/run-loaded-widen.sh" --plan "$model" 0 \
        "$R/sealed/$model-h16-never-install.elf" "$h" "$T/$model-widen-room.bin" "$s"
done
