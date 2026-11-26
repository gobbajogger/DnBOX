#!/bin/bash
set -euo pipefail
R=$(cd "$(dirname "$0")/../.." && pwd); D=$R/widen-native-verification; cd "$R"
exec 9>/tmp/dnbox-gate-firmware.lock; flock -x -n 9
N="$R/rehydrated-reverb/near-bound/tree/labout"
T="$R/s1-organ-wet-recovery/native-widen-comp/tree/widen-native"
E=aae0d5d7b14825e3bdf0dfeeb65a254e80d67e5ff23e963b3b24e26144bd9a3a
P=829301bf3b49419b275635fe36ce67277c569a1fab2095ac1364553c683492fd
export PYTHONPATH="$R/restored-harness-inspect/helpers"
w=$(python3 "$PYTHONPATH/guarded_replay.py" --workspace "$R" --archive "$R/restored-harness-inspect/dot-gate-harness.zip" --elite "$T/elite/never-install.elf" --elite-sha "$E" --pro "$T/pro/never-install.elf" --pro-sha "$P")
c=$(head -1 "$D/cold.log")
for m in elite pro; do
    path="$T/$m/never-install.elf"
    [[ "$m" == elite ]] && h=$E || h=$P
    loc() { readelf -sW "$1" | awk '$8=="proj"{print $2,$3,$4,$7}'; }
    cmp <(loc "$path") <(loc "$N/$m/never-install.elf")
    [[ -s "$c/$m-offset--1/result.json" ]]
    seed="$w/$m-widen-room.bin"
    python3 room-native-recovery/seed.py "$w/$m-virtual-nor.bin" "$N/$m/never-install.elf" "$c/$m-offset--1/ram.bin" "$R/remote-typed-load/tree/build/smk37-elite/x0x.elf" "$R/remote-typed-load/loaded-smoke-elite/ram.bin" "$D/fixture" "$seed" --active
    s=$(sha256sum "$seed" | cut -c1-64)
    bash "$D/run-loaded-widen.sh" --plan "$m" 0 "$path" "$h" "$seed" "$s"
    bash "$D/run-loaded-widen.sh" --run "$m" 0 "$path" "$h" "$seed" "$s" | tee "$D/$m-comp.log"
done
