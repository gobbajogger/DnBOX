#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")")
mode=${1:-green}
base=${DNBOX_EXACT_BASE:-"$d/../controls-init-only/source-mod-wheels"}
(cd "$base"; sha256sum --quiet -c ../MOD-WHEELS-POST.sha256)
w=$(mktemp -d "$d/build.XXXXXX")
trap 'rm -rf -- "$w"' EXIT
cp -a "$base"/{build,firmware,host,tests} "$w/"
for dir in assets ble tools experiments; do ln -s "$base/$dir" "$w/$dir"; done
cp "$d/wheel-fixture.c" "$w/tests/host/"
cd "$w"
if [[ $mode == green ]]; then patch -s -p1 --fuzz=0 < "$d/midi-bend.patch"; fi
sed -n '/^static int smk_pitch_parameter(int v)$/,/^}$/p; /^static void smk_wheels(void)$/,/^}$/p' firmware/src/app/main_fm1.c > tests/host/wheels.inc
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTH_CONTROLS_INIT_ONLY=1 -Itests/host -Wl,--wrap=s1_part_init -Wl,--wrap=juno6_init'
export SMC_UI_TEST_FILE=tests/host/wheel-fixture.c SAN_FLAGS='-fsanitize=undefined -fno-sanitize-recover=all'
SMC_UI_OUT="$w/host-out" sh tests/host/sj6-smc-ui.sh
