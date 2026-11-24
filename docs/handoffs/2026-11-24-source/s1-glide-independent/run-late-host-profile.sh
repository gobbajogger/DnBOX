#!/bin/sh
set -eu
root=$1
source=$2
out=$3
proof="$root/s1-glide-independent"
test "$(git -C "$source" rev-parse 'HEAD^{tree}')" = b1eaf004c2ac01670b11ee7dbf3ad9e5620fca51
mkdir "$out/fw"
git -C "$source" archive HEAD | tar -x -C "$out/fw"
cd "$out/fw"
patch -s -p1 < "$proof/prefix-glide-lab.patch"
export ATTRIBUTION_FIXTURE="$proof/late-host-profile.c"
cat > "$out/cc" <<'PY'
#!/usr/bin/env python3
import os
import sys
args = [os.environ["ATTRIBUTION_FIXTURE"] if a == "host/x0x_host.c" else a for a in sys.argv[1:]]
os.execvp("cc", ["cc", "-Ihost", *args])
PY
chmod 700 "$out/cc"
export CC="$out/cc" X0X_S1C=1 X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 X0X_DEV_WOBBLE=0
export PYTHONDONTWRITEBYTECODE=1
objects="$root/perf-final-proof/final1-pack/s1-63-miami.project-objects"
factory="$root/s1-fix-data/s1.pack"
test "$(sha256sum "$objects" | cut -d' ' -f1)" = 2d78b4a7dd957b615ba9373cb6a0a4a5f2152f3ee2d2274f7fa37bb40fa466a3
test "$(sha256sum "$factory" | cut -d' ' -f1)" = c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44
for arm in control profile; do
    case "$arm" in
        control) defs='-DX0X_ENGINE_PROFILE=0' ;;
        profile) defs='-DX0X_ENGINE_PROFILE=1 -DATTRIBUTION=1 -Wl,--wrap=original_runtime_prepare -Wl,--wrap=original_runtime_render -Wl,--wrap=s1_part_render -Wl,--wrap=S1Sequencer_willFire' ;;
    esac
    X0X_HOST_DEFS="-DX0X_S1C=1 $defs" X0X_HOST_OUT="$out/$arm" sh host/build_host.sh > "$out/$arm.build.log" 2>&1
done
"$out/control" "$objects" "$factory" "$out/control.pcm" "$out/control.csv" "$out/control.midi"
for repeat in 1 2 3; do
    "$out/profile" "$objects" "$factory" "$out/profile-$repeat.pcm" "$out/profile-$repeat.csv" "$out/profile-$repeat.midi"
    cmp "$out/control.pcm" "$out/profile-$repeat.pcm"
    cmp "$out/control.midi" "$out/profile-$repeat.midi"
done
sha256sum "$out/control.pcm" "$out/control.midi"
