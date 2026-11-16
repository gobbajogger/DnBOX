#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# chicago-au-render publish race (deterministic, no timing).
# GNU ld unlinks its -o file, recreates it mode 0644, writes it and sets +x only
# at the end. If run_chicago_au_render.sh links straight onto the shared path,
# a parallel reader (chicago-au-battery) that execs it inside that window gets
# EACCES. A fake CC replays ld's sequence and, inside the window, execs the
# shared path itself: it must still run (the old or the new whole binary).
# Also: a temp name left by a SIGKILLed run (dead PID) is swept on the next run.
# Usage: chicago_au_render_publish_test.sh [script-under-test]
set -u
ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
SUT="${1:-$ROOT/tests/host/run_chicago_au_render.sh}"
T="$(mktemp -d "${TMPDIR:-/tmp}/x0x-au-publish.XXXXXX")"
trap 'rm -rf "$T"' EXIT
mkdir -p "$T/tests/host" "$T/build/chicago_au"
cp "$SUT" "$T/tests/host/run_chicago_au_render.sh"
PUB="$T/build/chicago_au/chicago_au_render"
printf '#!/bin/sh\necho old\n' > "$PUB"; chmod 755 "$PUB"
# A dead PID: a short-lived child that has exited and been reaped.
sh -c 'exit 0' & dead=$!; wait $dead
while kill -0 "$dead" 2>/dev/null; do dead=$((dead + 1)); done
: > "$T/build/chicago_au/.chicago_au_render.$dead"
cat > "$T/fake-ld-cc.sh" <<'CC'
#!/bin/sh
out=""; prev=""
for a in "$@"; do [ "$prev" = "-o" ] && out="$a"; prev="$a"; done
[ -n "$out" ] || exit 2
rm -f "$out"; : > "$out"; chmod 644 "$out"          # ld: unlink, recreate 0644 (no +x)
if r=$("$PROBE_PUB" 2>&1); then echo "window exec ok: $r" > "$PROBE_LOG"
else echo "window exec FAILED rc=$?: $r" > "$PROBE_LOG"; fi
printf '#!/bin/sh\necho new\n' > "$out"; chmod 755 "$out"   # ld: write, then +x last
CC
chmod 755 "$T/fake-ld-cc.sh"
fail=0
( cd "$T" && env -u CHICAGO_AU_RENDER_OUT CC="$T/fake-ld-cc.sh" PROBE_PUB="$PUB" PROBE_LOG="$T/probe.log" \
    sh tests/host/run_chicago_au_render.sh ) > "$T/render.log" 2>&1 || { echo "FAIL render script exit: $(cat "$T/render.log")"; fail=1; }
probe=$(cat "$T/probe.log" 2>/dev/null || echo "probe did not run")
case "$probe" in
    "window exec ok: old"|"window exec ok: new") echo "ok   shared renderer runs inside the link window ($probe)" ;;
    *) echo "FAIL shared renderer not runnable inside the link window ($probe)"; fail=1 ;;
esac
r=$("$PUB" 2>&1) && [ "$r" = new ] && echo "ok   published renderer is the new whole binary" || { echo "FAIL published renderer: $r"; fail=1; }
left=$(ls -A "$T/build/chicago_au" | grep -v '^chicago_au_render$' || true)
[ -z "$left" ] && echo "ok   no temp names left (stale PID $dead swept)" || { echo "FAIL temp names left: $left"; fail=1; }
[ $fail = 0 ] && echo "chicago-au-render publish: ok" || echo "chicago-au-render publish: FAIL"
exit $fail
