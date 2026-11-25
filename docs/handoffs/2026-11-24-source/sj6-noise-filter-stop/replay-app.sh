#!/bin/sh
set -eu
q=$(realpath "$(dirname "$0")"); a=$(dirname "$q"); p="$a/sj6-random-mode5"; join="$a/arp-key-gate-joined"
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
c="$a/all-typed-controls-join"; ref="$a/s1-a4-reference-ui"; clk="$a/s1-arp-clock-typed"
(cd "$p"; sha256sum --quiet -c DEPENDS.sha256)
t=$(mktemp -d "$q/replay.XXXXXX")
trap 'rm -rf -- "$t"' EXIT
sed "s|^p=.*|p='$c'|;s|\$p/isolated.txt|$t/path|" "$c/compose.sh" | TMPDIR="$t" sh -s "$b"
cd "$(cat "$t/path")"
sha256sum --quiet -c "$c/POSTIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/s1-a4-reference-receiver/reference.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$ref/a4-reference-ui.patch"
sha256sum --quiet -c "$ref/POSTIMAGE.sha256"
sha256sum --quiet -c "$clk/PREIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$clk/s1-arp-clock-typed.patch"
sha256sum --quiet -c "$clk/POSTIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$clk/original-clock-law.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$join/zero-on-clock.patch"
sha256sum --quiet -c "$join/POSTIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$p/random5.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-random2/random2.patch"
sha256sum --quiet -c "$a/sj6-random2/POSTIMAGE.sha256"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-generated-trigger/receiver.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-routed-osc/routed.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-routed-osc/app.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-routed-noise/noise-route.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-short-release-host/short-release.patch"
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$a/sj6-short-stop-host/stop.patch"
cp "$q/filter_stop_app.c" "$a/sj6-id201-colored-source/color-oracle.c" "$a/sj6-two-input-lp2/lp2.c" tests/host/
python - "$a/sj6-id201-noise-lifecycle/lifecycle.c" tests/host/noise_lifecycle_for_ui.h <<'PYR'
from pathlib import Path
import sys
base=Path(sys.argv[1]).read_text().split('int main(void)')[0]
base=base.replace('../sj6-id201-colored-source/color-oracle.c','color-oracle.c').replace('#include "../sj6-two-input-lp2/lp2.c"','#define render rendered_lp2_only\n#define energy rendered_lp2_energy\n#include "lp2.c"\n#undef render\n#undef energy')
Path(sys.argv[2]).write_text(base)
PYR
sed 's/-DX0X_SJ6_RANDOM5_PROBE=1/-DX0X_SJ6_RANDOM5_PROBE=1 -DDNBOX_SJ6_GENERATED_TRIGGER=1 -DDNBOX_SJ6_NOISE_ROUTE_WITNESS=1 -DDNBOX_SJ6_SHORT_RELEASE_WITNESS=1 -DDNBOX_SJ6_STOP_WITNESS=1/' "$p/run-new.sh" > "$t/test.sh"
sed '/^static void run_free/,$d' "$a/sj6-zero-gate/ui_gate_zero.c" >tests/host/gate_helpers.inc
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
mkdir "$t/cc"
export TMPDIR="$t/cc"
JOIN_TEST=filter_stop_app.c sh "$t/test.sh" >out/green.log 2>&1 || { grep -E "STOP o=|error:|runtime error:|FAIL|No space|undefined reference" out/green.log; tail -35 out/green.log; exit 1; }
grep '^PASS' out/green.log
