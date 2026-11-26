#!/bin/bash
set -euo pipefail
here=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
base=$(dirname "$here")
root="${1:-"$HOME/DnBOX-AU-Bench"}"
app="$root/DnBOX AUv3 Bench.app"
if [[ -e "$app" ]]; then printf 'Refusing to replace an existing bench: %s\n' "$app" >&2; exit 1; fi
umask 077
mkdir -p "$root/requests" "$root/jobs"
stage=$(mktemp -d "$root/.build.XXXXXXXX")
trap 'rm -rf "$stage"' EXIT
new="$stage/DnBOX AUv3 Bench.app"
mkdir -p "$new/Contents/"{MacOS,Resources}
cp "$here/Info.plist" "$new/Contents/Info.plist"
cp "$base/sj6-direct-auv3-master/s4-inputs.json" "$new/Contents/Resources/s4-inputs.json"
python3 "$here/compose.py" --master "$base/sj6-direct-auv3-master/main.swift" \
 --fanin "$base/sj6-direct-auv3-fanin/main.swift" --static "$base/sj6-direct-auv3-osc2-static/main.swift" \
 --physical "$base/sj6-direct-auv3-physical-cutoff/main.swift" --dynamic "$base/sj6-direct-auv3-dynamic-cutoff/main.swift" \
 --host "$here/queue.swift" > "$stage/main.swift"
swiftc "$stage/main.swift" -O -o "$new/Contents/MacOS/Capture"
codesign --force --sign - "$new"
mv -n "$new" "$app"
printf '%s\n' "$app"
