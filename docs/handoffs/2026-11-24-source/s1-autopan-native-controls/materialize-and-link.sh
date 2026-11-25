#!/bin/bash
set -euo pipefail
d=$(realpath "$(dirname "$0")"); root=$(realpath "$1"); carrier=$(realpath "$2"); out=$(realpath "$3"); m=$4
w=$(mktemp -d "$root/.s1-pan-source.XXXXXX")
trap 'if [[ -s "$w/exact" ]];then t=$(<"$w/exact");case "$t" in "$root"/perf-final-proof/dnbox-s1-mod-sidecar.*) rm -rf -- "$t";;esac;fi;rm -rf -- "$w"' EXIT
export DEST="$w/exact" TMPDIR="$w"
q="$root/perf-final-proof/wheels-page24-classic-random2-join"
python3 - "$q/replay.sh" "$w/replay.sh" "$q" <<'PY'
import pathlib,sys
src,out,folder=map(pathlib.Path,sys.argv[1:]);s=src.read_text()
s=s[:s.index('cp "$p/ui_random2.c"')]
s=s.replace('p=$(cd "$(dirname "$0")"&&pwd);',f'p="{folder}";',1)
s=s.replace('t=$(<"$w/tree");cd "$t"','t=$(<"$w/tree"); printf "%s\\n" "$t" > "$DEST"; trap \'rm -rf -- "$w"\' EXIT; cd "$t"',1)
out.write_text(s)
PY
bash "$w/replay.sh" "$root/perf-final-proof/controls-init-only/source-mod-wheels"
bash "$d/native-compose.sh" "$root" "$(<"$DEST")" "$carrier" "$out" "$m"
