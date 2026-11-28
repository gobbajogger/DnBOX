#!/usr/bin/env bash
set -euo pipefail
# Use the parent lazy composition, then inject only the selected test.
here=$(cd "$(dirname "$0")" && pwd)
src=$(realpath "${1:?clean pinned source}")
board=${2:?pro or elite}
variant=${3:?base, pan, sub, lfo, combined, or table}
case "$variant" in base|pan|sub|lfo|combined|table) ;; *) exit 2;; esac
export DSP_TRIAL_DIR="$here" DSP_TRIAL="$variant"
temp=$(mktemp "$here/../.build-dsp-trial.XXXXXX")
trap 'rm -f "$temp"' EXIT
awk '
/^MODEL=/ {print "case \"$DSP_TRIAL\" in\n  pan) git apply \"$DSP_TRIAL_DIR/pan-zero.patch\";;\n  sub) git apply \"$DSP_TRIAL_DIR/sub-zero.patch\"; flags+=\" -DDNBOX_S1_DORMANT_SUB=1\";;\n  lfo) git apply \"$DSP_TRIAL_DIR/lfo-pair.patch\";;\n  combined) git apply \"$DSP_TRIAL_DIR/pan-zero.patch\"; git apply \"$DSP_TRIAL_DIR/sub-zero.patch\"; git apply \"$DSP_TRIAL_DIR/lfo-pair.patch\"; flags+=\" -DDNBOX_S1_DORMANT_SUB=1\";;\n  table) git apply \"$DSP_TRIAL_DIR/full-sine-table.patch\";;\nesac"}
{print}' "$here/../build-overlap.sh" >"$temp"
bash "$temp" "$src" "$board" lazy
