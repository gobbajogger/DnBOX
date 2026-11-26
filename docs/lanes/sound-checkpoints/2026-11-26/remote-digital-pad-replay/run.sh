#!/usr/bin/env bash
set -euo pipefail
W=$(realpath "$(dirname "$0")")
D=$(realpath "$1")
(cd "$D" && sha256sum -c "$W/pins.txt")
(cd "$D/remote" && git hash-object app.mjs dnr3-link.mjs dnr3.mjs) | diff - "$W/website-git-blobs.txt"
python3 "$W/make.py" "$D" "$W/host-cable.c"
ln -sfn "$D/remote" "$W/remote"
(
 cd "$D"; export X0X_S1C=1 X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1
 . tests/host/original_build.sh
 cc -pipe -O1 -std=gnu99 -w -DX0X_HOST -DBOARD_SMK37=1 -Ifirmware/src -Ifirmware/src/dsp -Ibuild/gen $ORIGINAL_FLAGS -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_REMOTE_TYPED_SYNTH=1 "$W/host-cable.c" $ORIGINAL_SOURCES -lm -o "$W/typed-cable"
)
node "$W/replay.mjs" --dual
