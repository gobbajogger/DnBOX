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
no_loss()    { git fetch -q origin integration/dnbox-consolidated || return 1
               git merge-base --is-ancestor "$BASE" HEAD || { echo "branch does not contain $BASE: merge it in (no rebase)"; return 1; }
               local del; del=$(git diff --name-only --diff-filter=D "$BASE" HEAD)
               [ -z "$del" ] && return 0
               local lf="docs/lanes/$LANE.md" miss=0
               while read -r f; do grep -qF -- "$f" "$lf" 2>/dev/null || { echo "deleted without reason in $lf: $f"; miss=1; }; done <<< "$del"
               return $miss; }
limits()     { local bad=0
               git diff --name-only "$BASE" HEAD | grep -E '^\.github/workflows/' && { echo "workflow file changed"; bad=1; }
               git ls-files | grep -iE '\.ipa$|SuperJ6.*AU.*extension.*\.zip' && { echo "SuperJ6 installer in Git"; bad=1; }
               git grep -nIE '(ghp_|gho_|github_pat_)[A-Za-z0-9_]{20,}|-----BEGIN [A-Z ]*PRIVATE KEY-----|AKIA[0-9A-Z]{16}' -- . ':!*.md' && { echo "possible Secret"; bad=1; }
               git diff "$BASE" HEAD -- firmware tools build.sh | grep -E '^\+.*(PSRAM|psram).*(=\s*1|ENABLE|enable|true)' && { echo "PSRAM enabled"; bad=1; }
               git ls-files -s | while read -r m o s p; do sz=$(git cat-file -s "$o"); [ "$sz" -ge 52428800 ] && { echo "≥50MB not in LFS: $p"; exit 1; }; done || bad=1
               return $bad; }
host_tests() { sh host/build_host.sh && sh tests/run_tests.sh | tee "$OUT/tests.txt"; local r=${PIPESTATUS[0]}
               grep -E '^\s*(FAIL|not-run)\b' "$OUT/tests.txt" && return 1; return $r; }
fw_build()   { MODEL=elite ./build.sh && cp -f build/x0x-smk37.fwsc "$OUT/elite.fwsc" 2>/dev/null; MODEL=pro ./build.sh || return 1; }
flashmap()   { python3 tools/gate_flashmap.py firmware/hal/fm1_flash_map.h firmware/src/storage.c; }
budget()     { # Expects the build to print XIP free and pool margin; wire the exact source on first run (integ owns this).
               local xip pool; xip=$(grep -hoiE 'xip[ _-]?free[^0-9-]*-?[0-9]+' build/*.log build/*.map 2>/dev/null | grep -oE '\-?[0-9]+$' | sort -n | head -1)
               pool=$(grep -hoiE 'pool[ _-]?margin[^0-9-]*-?[0-9]+' build/*.log 2>/dev/null | grep -oE '\-?[0-9]+$' | sort -n | head -1)
               [ -n "$xip" ] && [ -n "$pool" ] || { echo "budget numbers not found (NOT_RUN = FAIL): make tools/build.py print 'XIP free: <bytes>' and 'pool margin: <bytes>' per model"; return 1; }
               echo "XIP free (worst model) = $xip B (min $XIP_MIN); pool margin = $pool B (min $POOL_MIN)"
               [ "$xip" -ge "$XIP_MIN" ] && [ "$pool" -ge "$POOL_MIN" ]; }
ui_defects() { python3 tests/ui_defects/test_gate.py && { [ ! -f tests/ui_defects/first_slice/evaluate.py ] || python3 tests/ui_defects/first_slice/evaluate.py; }; }
drone()      { local n; n=$(git grep -niwI drone -- firmware web tests ':!*CHANGELOG*' ':!docs/history/*' | wc -l); echo "Drone mentions: $n"; [ "$LANE" = sound ] || [ "$LANE" = integ ] || return 0; [ "$n" -eq 0 ]; }

step clean-tree clean
step setup setup
step no-loss no_loss
step limits limits
if [ $void = 0 ]; then step host-tests host_tests; step build-elite-pro fw_build; step flashmap flashmap; step budget budget; step ui-defects ui_defects; fi
step drone drone
R=PASS; [ $fail = 1 ] && R=FAIL; [ $void = 1 ] && R=VOID
echo "DNBOX-GATE RESULT $R sha=$SHA lane=$LANE log=$LOG"
[ $R = PASS ] && exit 0; [ $R = VOID ] && exit 2; exit 1
