#!/usr/bin/env bash
# DnBOX Local gate (firmware repo). One command:  tools/dnbox-gate.sh [<lane>]
# Runs at the checked-out HEAD on the agent's own machine. Never touches a device, never pushes.
# Result: last line "DNBOX-GATE RESULT PASS|FAIL|VOID sha=<40>"; exit 0 PASS, 1 FAIL, 2 VOID (setup missing).
# A step that cannot run is FAIL (NOT_RUN counts as FAIL). Do not weaken thresholds; fix the step instead.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2
LANE="${1:-unknown}"; SHA=$(git rev-parse HEAD); BASE=origin/integration/dnbox-consolidated
TS=$(TZ=Asia/Vladivostok date +%Y%m%d-%H%M); OUT=build/dnbox-gate; mkdir -p "$OUT"
LOG="$OUT/${SHA:0:8}-$TS.log"; exec > >(tee "$LOG") 2>&1
XIP_MIN=${XIP_MIN:-1024}; POOL_MIN=${POOL_MIN:-512}
fail=0; void=0
step() { local name=$1; shift; echo "== $name"; if "$@"; then echo "[PASS] $name"; else rc=$?; [ $rc = 2 ] && { echo "[VOID] $name"; void=1; } || { echo "[FAIL] $name"; fail=1; }; fi; }

echo "DNBOX-GATE repo=firmware lane=$LANE sha=$SHA started=$TS (UTC+10)"
clean()      { git diff --quiet && git diff --cached --quiet; }
setup()      { [ -x "${JIELI_TOOLCHAIN:-$HOME/.jieli/toolchain}/pi32v2/bin/clang" ] && [ -f "${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}/cpu/wl82/tools/uboot.boot" ] \
               && python3 tools/check_original_inputs.py >/dev/null || { echo "missing JieLi toolchain/SDK or original synth inputs (restore from Library, see docs/LIBRARY-INDEX.md)"; return 2; }; }
no_loss()    { if [ -n "${DNBOX_BASE_SHA:-}" ]; then
                 [[ "$DNBOX_BASE_SHA" =~ ^[0-9a-f]{40}$ ]] || { echo "invalid connector base SHA"; return 1; }
                 [ "$(git cat-file -t "$DNBOX_BASE_SHA" 2>/dev/null)" = commit ] || { echo "connector base commit unavailable"; return 1; }
                 BASE=$DNBOX_BASE_SHA; echo "connector-pinned consolidated base=$BASE (caller must retain fresh remote receipt)"
               else git fetch -q origin integration/dnbox-consolidated || return 1; BASE=$(git rev-parse --verify "$BASE^{commit}") || return 1; echo "fetched consolidated base=$BASE"; fi
               python3 tools/dnbox_contains_base.py "$BASE" || { echo "branch does not contain $BASE: merge it in (no rebase)"; return 1; }
               local del; del=$(git diff --name-only --diff-filter=D "$BASE" HEAD) || return 1
               [ -z "$del" ] && return 0
               local lf="docs/lanes/$LANE.md" miss=0
               while read -r f; do grep -qF -- "$f" "$lf" 2>/dev/null || { echo "deleted without reason in $lf: $f"; miss=1; }; done <<< "$del"
               return $miss; }
limits()     { local bad=0
               python3 tools/dnbox-hard-limits.py || bad=1
               git diff --name-only "$BASE" HEAD | grep -E '^\.github/workflows/' && { echo "workflow file changed"; bad=1; }
               git ls-files | grep -iE '\.ipa$|SuperJ6.*AU.*extension.*\.zip' && { echo "SuperJ6 installer in Git"; bad=1; }
               git grep -lIE '(ghp_|gho_|github_pat_)[A-Za-z0-9_]{20,}|-----BEGIN [A-Z ]*PRIVATE KEY-----|AKIA[0-9A-Z]{16}' -- . && { echo "possible Secret (filename-only scan)"; bad=1; }
               git diff "$BASE" HEAD -- firmware tools build.sh | grep -iE '^\+.*((define[[:space:]]+[A-Za-z0-9_]*psram[A-Za-z0-9_]*[[:space:]]+(1|true|enable))|([A-Za-z0-9_]*psram[A-Za-z0-9_]*[[:space:]]*=[[:space:]]*(1|true|enable)))\b' && { echo "PSRAM enabled"; bad=1; }
               git ls-files -s | while read -r m o s p; do sz=$(git cat-file -s "$o") || exit 1; if [ "$sz" -ge 52428800 ]; then echo "≥50MB not in LFS: $p"; exit 1; fi; done || bad=1
               return $bad; }
host_tests() { sh host/build_host.sh && sh tests/run_tests.sh | tee "$OUT/tests.txt"; local r=${PIPESTATUS[0]}
               grep -E '^\s*(FAIL|not-run)\b' "$OUT/tests.txt" && return 1; return $r; }
fw_build()   { BUILD_RUN=$(mktemp -d "$OUT/build-${SHA:0:8}-XXXXXX") || return 1
               local model
               for model in elite pro; do
                 MODEL=$model ./build.sh >"$BUILD_RUN/$model.log" 2>&1 || { cat "$BUILD_RUN/$model.log"; return 1; }
                 cat "$BUILD_RUN/$model.log"
                 printf '%s %s\n' "$SHA" "$model" >"$BUILD_RUN/$model.source"
               done; }
flashmap()   { python3 tools/gate_flashmap.py firmware/hal/fm1_flash_map.h firmware/src/storage.c; }
budget()     { local model xip pool
               [ -n "${BUILD_RUN:-}" ] && [ -d "$BUILD_RUN" ] || { echo "no fresh model build receipts"; return 1; }
               for model in elite pro; do
                 [ "$(cat "$BUILD_RUN/$model.source" 2>/dev/null)" = "$SHA $model" ] || { echo "missing exact-head $model receipt"; return 1; }
                 xip=$(sed -nE 's/.*app code area [0-9]+ B, (-?[0-9]+) B free.*/\1/p' "$BUILD_RUN/$model.log" | sort -n | head -1)
                 pool=$(sed -nE 's/.*pool ([0-9]+) B of ([0-9]+).*/\1 \2/p' "$BUILD_RUN/$model.log" | awk '{print $2-$1-8192}' | sort -n | head -1)
                 [ -n "$xip" ] && [ -n "$pool" ] || { echo "$model budget numbers missing (NOT_RUN = FAIL)"; return 1; }
                 echo "$model XIP free=$xip B pool margin=$pool B sha=$SHA"
                 [ "$xip" -ge "$XIP_MIN" ] && [ "$pool" -ge "$POOL_MIN" ] || return 1
               done; }
ui_defects() { python3 tests/ui_defects/test_gate.py && python3 tools/dnbox-ui-gate.py --output-root "$OUT"; }
drone()      { local n; n=$(git grep -niwI drone -- firmware web tests ':!*CHANGELOG*' ':!docs/history/*' | wc -l); echo "Drone mentions: $n"; [ "$LANE" = sound ] || [ "$LANE" = integ ] || return 0; [ "$n" -eq 0 ]; }

step clean-tree clean
step setup setup
step no-loss no_loss
step limits limits
browser_safe=1
browser_policy() { python3 tests/browser-sandbox-policy.py || { browser_safe=0; return 1; }; }
step browser-sandbox-policy browser_policy
sanitizer_safe=1
sanitizer_policy() { python3 tools/dnbox-sanitizer-policy.py || { sanitizer_safe=0; return 1; }; }
step sanitizer-policy sanitizer_policy
if [ $void = 0 ]; then if [ $browser_safe = 1 ] && [ $sanitizer_safe = 1 ]; then step host-tests host_tests; else echo "[FAIL] host-tests NOT_RUN: browser or sanitizer policy unsafe or unavailable"; fail=1; fi; step build-elite-pro fw_build; step flashmap flashmap; step budget budget; step ui-defects ui_defects; fi
step drone drone
R=PASS; [ $fail = 1 ] && R=FAIL; [ $void = 1 ] && R=VOID
echo "DNBOX-GATE RESULT $R sha=$SHA lane=$LANE log=$LOG"
[ $R = PASS ] && exit 0; [ $R = VOID ] && exit 2; exit 1
