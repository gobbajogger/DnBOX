#!/bin/sh
set -eu
p=$(realpath "$(dirname "$0")")
b=$(realpath "$1")
(cd "$p"; sha256sum -c --quiet DEPENDS.sha256)
trap 'if [ -s "$p/isolated.txt" ]; then t=$(cat "$p/isolated.txt"); case "$t" in */dnbox-s1-mod-sidecar.*) rm -rf -- "$t"; rm -f "$p/isolated.txt";; esac; fi' EXIT
test ! -e "$p/isolated.txt"
sh "$p/compose.sh" "$b"
t=$(cat "$p/isolated.txt")
(cd "$t"; sha256sum -c --quiet "$p/POSTIMAGE.sha256")
sh "$p/run-composed.sh" "$b" "$t"
