#!/bin/sh
set -eu
arg=$1
case "$arg" in /*) ;; *) arg="$PWD/$arg";; esac
dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
cd "$tmp"
swiftc -O "$dir/input.swift" -o stimuli
./stimuli
python3 "$dir/score.py" "$arg" "$tmp"
