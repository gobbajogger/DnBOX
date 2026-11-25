#!/bin/bash
set -euo pipefail
# Usage: restore-cold-room.sh <rehydration-root> <workspace-root>
R="$(realpath "$1")"; W="$(realpath "$2")"; P="$(cd "$(dirname "$0")"&&pwd)"
C="$R/checkpoints"; F="$C/2026-11-25-first-paired-guarded-fit"
N="$R/near-bound"; U="$R/cold-undo"; D="$R/dual-room"
test -d "$N/tree" && test ! -e "$U" && test ! -e "$D"
test "$(sha256sum "$N/tree/labout/elite/never-install.elf"|cut -d' ' -f1)" = 5bcd76a82e44a5ce135157c8ccac3ce21be212c3783d285772588a77585e5594
test "$(sha256sum "$N/tree/labout/pro/never-install.elf"|cut -d' ' -f1)" = 7145d630918e221c6ef64d7da5d976a3126c42ab4cbd56b0012e1f2b3e77f394
for n in prereqs all-controls-remote-join; do ln -sfn "$W/$n" "$R/$n"; done
for n in guarded-fit-miami-cap guarded-fit-miami-decimator; do ln -sfn "$F/$n" "$R/$n"; done
mkdir -p "$U"
cp -a --reflink=auto "$N/"{tree,run.sh,link-guarded-exact.py,replay*.sh} "$U/"
patch --batch -F0 -p1 -d "$U/tree" <"$C/2026-11-25-exact-heavy-scene/fold/fold-sparse.patch"
patch --batch -F0 -p1 -d "$U" <"$P/chicago-cold-speed.patch"
python3 "$P/gen-speed.py" "$U/tree"
for p in "$C/2026-11-25-cold-app-and-key-pool/cold/loaded-perf-all-cold.patch" "$C/2026-11-25-cold-app-and-key-pool/keys/packed-live-key-state.patch" "$C/2026-11-25-typed-cold-and-room-premerge/app/loaded-perf-ui-cold-dispatch-and-undo.patch"; do
  patch --batch -F0 -p1 -d "$U" <"$p"
done
export DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1
bash "$U/replay-near-maps.sh" >"$U/pair.log" 2>&1
(
 cd "$U/tree/labout"
 printf '%s  %s\n' '02ceb3c1c7a3b032e24a0a149daf3a00238f869c8628ba73975e8fe6815f1147' 'elite/never-install.elf' 'c2e88ec1e2f8efb87933a7c7f1f3eb25b4e945a2747a3de750cd0af8c5bd1dfe' 'pro/never-install.elf' |sha256sum -c
)
mkdir -p "$D"
cp -a --reflink=auto "$U/"{tree,run.sh,link-guarded-exact.py,replay*.sh} "$D/"
patch --batch -F0 -p1 -d "$D/tree" <"$C/2026-11-25-typed-cold-and-room-premerge/room/dual-room.patch"
bash "$D/replay-near-maps.sh" >"$D/pair.log" 2>&1
(
 cd "$D"
 sed -nE 's#  loaded-perf-undo-room/(tree/labout/(elite|pro)/([^ ]+\.(bin|elf))|link-guarded-exact.py|replay[^ ]*\.sh|run.sh|tree/firmware/[^ ]+) *$#  \1#p' "$C/2026-11-25-dual-room-real-fit/FIT.sha256" |sha256sum -c
)
