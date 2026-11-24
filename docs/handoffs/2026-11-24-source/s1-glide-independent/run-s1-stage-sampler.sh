#!/bin/sh
# Reuse a verified fresh late-host archive and its independent control.
set -eu
root=$1
old=$2
out=$3
proof="$root/s1-glide-independent"
fw="$old/fw"
cd "$fw"
export ATTRIBUTION_FIXTURE="$proof/late-host-profile.c"
export S1_STAGE_SAMPLER="$proof/s1-stage-sampler.c"
export S1_STAGE_SHADOW="$out/s1c-tagged.c"
python3 "$proof/s1-stage-shadow.py" firmware/src/dsp/s1c/s1c.c "$S1_STAGE_SHADOW"
cat > "$out/cc" <<'PY'
#!/usr/bin/env python3
import os
import sys
args = {
    "host/x0x_host.c": os.environ["ATTRIBUTION_FIXTURE"],
    "firmware/src/dsp/s1c/s1c.c": os.environ["S1_STAGE_SHADOW"],
}
os.execvp("cc", ["cc", "-Ihost", "-Ifirmware/src/dsp/s1c", *[args.get(a, a) for a in sys.argv[1:]],
                os.environ["S1_STAGE_SAMPLER"]])
PY
chmod 700 "$out/cc"
export X0X_S1C=1 X0X_SUPERJ6_FACTORY=1 X0X_SYNTHONE_PATCH=1 X0X_DEV_WOBBLE=0 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_S1C=1 -DX0X_ENGINE_PROFILE=0'
CC="$out/cc" X0X_HOST_OUT="$out/staged" sh host/build_host.sh > "$out/build.log" 2>&1
objects="$root/perf-final-proof/final1-pack/s1-63-miami.project-objects"
pack="$root/s1-fix-data/s1.pack"
i=1
while [ "$i" -le 32 ]; do
    period=1000
    if [ "$i" -gt 16 ]; then period=1700; fi
    printf 'run %s period %s\n' "$i" "$period"
    DNBOX_S1_SAMPLE_US="$period" "$out/staged" "$objects" "$pack" "$out/trace.pcm" "$out/trace.csv" "$out/trace.midi"
    cmp "$old/control.pcm" "$out/trace.pcm"
    cmp "$old/control.midi" "$out/trace.midi"
    i=$((i + 1))
done
