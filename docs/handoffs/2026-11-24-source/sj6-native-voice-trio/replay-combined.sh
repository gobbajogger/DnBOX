#!/bin/bash
# Host/no-device guarded link by default. --native uses the sole serialized emulator.
set -euo pipefail
q=$(realpath "$(dirname "$0")"); root=$(realpath "$q/../..")
g="$root/full-gate-inputs/sj6-native-positive-gate"; f="$root/full-gate-inputs/d9-native-reloc-wheel"
s=$(mktemp -d "$root/.voice-combined-replay.XXXXXX")
seed=''
trap 'rm -rf -- "$s"; if [[ -n "$seed" ]]; then rm -rf -- "$seed"; fi' EXIT
sed -e "s|^q=\$(realpath .*|q='$q'|" -e "s|^s=\$(mktemp .*|s='$s'|" -e '/^trap /d' -e '/^sh "\$lc\/run-juno/,$d' "$q/replay.sh" > "$s/setup.sh"
bash "$s/setup.sh"
t=$(<"$s/tree"); cd "$t"
patch -s --fuzz=0 -p1 < "$q/positive-gate-on-carrier.patch"
sha256sum --quiet -c "$q/COMBINED.sha256"
cp "$q/combined-gate-fixture.c" tests/host/ui_native_gate.c
bash "$q/run-combined-gate.sh" >"$s/host.log" 2>&1 || { tail -40 "$s/host.log"; exit 1; }
grep '^PASS' "$s/host.log"
if [[ "${1:-}" != --native ]]; then POSITIVE_GATE=1 bash "$q/link-guarded.sh" "$root" "$t" "$s"; exit 0; fi
rm -f out/test
KEEP_LINK=1 POSITIVE_GATE=1 bash "$q/link-guarded.sh" "$root" "$t" "$s"
ln -s "$t" "$s/src"; ln -s "$t/labout" "$t/gatelab"
sed 's/^int main(/int gate_seed_main(/' "$g/gate-project.c" > tests/host/gate-seed.inc
cp "$q/voice-seed-project.c" tests/host/
bash "$q/seed-project.sh" "$s" "$t"
bash "$q/combined-layout.sh" "$root" "$t" "$s"
seed=$(mktemp -d /tmp/dnbx-combined-voice-replay.XXXXXX)
for m in elite pro; do sha256sum "$t/labout/$m/never-install.elf"; grep '^guarded' "$s/$m-link.log">"$s/gate-$m.log"; done > "$t/labout/elf.sha256"
python "$f/seed-non-user.py" "$root" "$t/labout" "$seed"
for m in elite pro; do
 for o in 0 1; do
   python "$g/gate-native.py" "$root" "$s" "$seed" "$m" "$o"
   python -c 'import json,sys;assert json.load(open(sys.argv[1]))["flash_unchanged"]' "$s/native-$m-$o/result.json"
   rm -rf "$s/native-$m-$o"
   python "$q/voice-native.py" "$root" "$s" "$seed" "$m" "$o"
   rm -rf "$s/native-voice-$m-$o"
 done
done
