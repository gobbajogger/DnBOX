#!/bin/bash
set -euo pipefail
n=$(realpath "$(dirname "$0")"); root=$(realpath "$n/../..")
q="$root/full-gate-inputs/sj6-native-voice-trio"
s=$(mktemp -d "$root/.voice-dv-fit.XXXXXX")
trap 'rm -rf -- "$s"' EXIT
sed -e "s|^q=\$(realpath .*|q='$q'|" -e "s|^s=\$(mktemp .*|s='$s'|" -e '/^trap /d' -e '/^sh "\$lc\/run-juno/,$d' "$q/replay.sh" > "$s/trio.sh"
bash "$s/trio.sh"
t=$(<"$s/tree"); cd "$t"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 < "$q/positive-gate-on-carrier.patch"
sha256sum --quiet -c "$q/COMBINED.sha256"
sha256sum --quiet -c "$n/PREIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 < "$n/delta.patch"
sha256sum --quiet -c "$n/FIRMWARE.sha256"
POSITIVE_GATE=1 bash "$n/link-guarded.sh" "$root" "$t" "$s"
