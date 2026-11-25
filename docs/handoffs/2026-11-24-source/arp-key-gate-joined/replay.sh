#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")"); a=$(dirname "$p")
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
patch -s --fuzz=0 --no-backup-if-mismatch -p1 <"$p/zero-on-clock.patch"
sha256sum --quiet -c "$p/POSTIMAGE.sha256"
cp "$p/ui_pad_key_gate.c" tests/host/
sed '/^static void run_free/,$d' "$a/sj6-zero-gate/ui_gate_zero.c" >tests/host/gate_helpers.inc
sed -i '/^int main(void)/i\#ifndef MIXED_ONLY' tests/host/s1_pb_mod_ui.c
echo '#endif' >>tests/host/s1_pb_mod_ui.c
CLOCK_KEYBOARD_ONLY=1 JOIN_TEST=ui_arp_keyboard_test.c sh "$p/run-new.sh" >out/key.log 2>&1 || { tail -35 out/key.log; exit 1; }
grep '^PASS' out/key.log
sh "$p/run-new.sh" >out/combined.log 2>&1 || { tail -35 out/combined.log; exit 1; }
grep '^PASS' out/combined.log
