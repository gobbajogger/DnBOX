#!/bin/bash
# Reconstruct source, test actual runtime ownership and link in disposable read-only lab.
set -euo pipefail
q=$(realpath "$(dirname "$0")")
root=$(realpath "${1:-$q/../..}"); f="$root/perf-final-proof"
p="$f/wheels-sj6-voice-trio-typed"; r="$f/wheels-page24-classic-random2-join"
cl="$f/typed-controls-native-admission"; ap="$f/s1-autopan-native-controls"
l="$f/init-fdata-readonly-lab"; lc="$f/wheels-sj6-voice-lifecycle"
s=$(mktemp -d "$root/.voice-native-replay.XXXXXX")
trap 'rm -rf -- "$s"' EXIT
sed -e "s|^p=\$(cd .*; p0=|p='$p'; p0=|" -e "s|^w=\$(mktemp .*|w='$s'|" -e '/^trap /d' -e "s|^TMPDIR=\"\$parent\" sh|TMPDIR='$s' sh|" -e '/^sh "\$p\/run-exact-813.sh"/,$d' "$p/replay.sh" >"$s/setup.sh"
bash "$s/setup.sh"
t=$(<"$s/tree")
cd "$t"
patch -s -R --fuzz=0 -p1 < "$p/voice-trio.patch"
patch -s --fuzz=0 -p1 < "$r/random2-join.patch"
sha256sum --quiet -c "$r/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$cl/native-trial-carrier.patch"
sha256sum --quiet -c "$cl/POSTIMAGE.sha256"
patch -s --fuzz=0 -p1 < "$q/trio-on-carrier.patch"
patch -s --fuzz=0 -p1 < "$lc/voice-lifecycle.patch"
patch -s --fuzz=0 -p1 < "$q/explicit-native.patch"
for u in tools experiments;do location=$(realpath "$u"); unlink "$u"; cp -a "$location" "$u"; done
patch -s --fuzz=0 -p1 < "$ap/autopan-port.patch"
patch -s --fuzz=0 -p1 < "$f/init-only-legacy-root-prune/prune.patch"
patch -s --fuzz=0 -p1 < "$l/boot.diagnostic.diff"; cp "$l/fdata_readonly_lab.h" firmware/src/app/
sed -n '/^python3 - "\$root\/perf-final-proof\/s1-mod-native-scale\/native-scale.patch"/,/^PY$/p' "$ap/native-compose.sh" >"$s/native-scale.sh"
source "$s/native-scale.sh"
printf '%s  %s\n' b81840d410000c50cd9dd0c05b8d656b961d868b4c7574d7f7f4abda1bde5ab4 "$ap/pilot-extra.patch" | sha256sum --quiet -c -
patch -s --fuzz=0 -p1 < "$ap/pilot-extra.patch"
cp "$q/lifecycle-fixture.c" tests/host/ui_juno_voice_lifecycle.c
sha256sum --quiet -c "$q/POSTIMAGE.sha256"
sh "$lc/run-juno-voice-lifecycle.sh" >"$s/host.log" 2>&1 || { tail -35 "$s/host.log"; exit 1; }
grep '^PASS' "$s/host.log"
bash "$q/link-guarded.sh" "$root" "$t" "$s"
