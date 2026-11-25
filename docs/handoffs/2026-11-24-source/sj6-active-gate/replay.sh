#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p"); join="$a/arp-key-gate-joined"
b=$(realpath "${1:-$a/controls-init-only/source-mod-wheels}")
c="$a/all-typed-controls-join"; ref="$a/s1-a4-reference-ui"; clk="$a/s1-arp-clock-typed"
(cd "$p"; sha256sum --quiet -c DEPENDS.sha256)
t=$(mktemp -d "$p/replay.XXXXXX")
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
cp "$p/ui_active_gate.c" tests/host/
sed '/^static void run_free/,$d' "$a/sj6-zero-gate/ui_gate_zero.c" >tests/host/gate_helpers.inc
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
mkdir "$t/cc"
export TMPDIR="$t/cc"
if DEADLINE_FLAG=-DX0X_SJ6_GATE_DEADLINE_PROBE=0 sh "$p/run-new.sh" >out/red.log 2>&1; then
    echo 'Unexpected predecessor pass'; exit 1
fi
grep -F 'gates(parts_juno[v],note)==1' out/red.log
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$p/active-gate.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
sh "$p/run-new.sh" >out/green.log 2>&1 || { tail -35 out/green.log; exit 1; }
grep -E '^start owner|^PASS' out/green.log
