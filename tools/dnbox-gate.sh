#!/usr/bin/env bash
# DnBOX Local gate (firmware repo). One command:  tools/dnbox-gate.sh [<lane>]
# Runs at the checked-out HEAD on the agent's own machine. Never touches a device, never pushes.
# Result: last line "DNBOX-GATE RESULT PASS|FAIL|VOID sha=<40>"; exit 0 PASS, 1 FAIL, 2 VOID (setup missing).
# A step that cannot run is FAIL (NOT_RUN counts as FAIL). Do not weaken thresholds; fix the step instead.
# VOID must not hide an earlier FAIL: the overall result is FAIL if any stage failed.
# VOID only when the environment is genuinely missing; the setup step lists what is missing.
# Offline is the default: no git fetch, clone or ls-remote. Set DNBOX_GATE_ONLINE=1 to allow
# network Git. A skipped network step prints [SKIP] and the rest runs against local refs.
set -uo pipefail
cd "$(git rev-parse --show-toplevel)" || exit 2
LANE="${1:-unknown}"; SHA=$(git rev-parse HEAD); BASE=origin/integration/dnbox-consolidated
TS=$(TZ=Asia/Vladivostok date +%Y%m%d-%H%M); OUT=build/dnbox-gate; mkdir -p "$OUT"
LOG="$OUT/${SHA:0:8}-$TS.log"; exec > >(tee "$LOG") 2>&1
XIP_MIN=${XIP_MIN:-1024}; POOL_MIN=${POOL_MIN:-512}
fail=0; void=0
step() {
  local name=$1; shift
  echo "== $name"
  if "$@"; then
    echo "[PASS] $name"
  else
    rc=$?
    if [ "$rc" -eq 3 ]; then
      echo "[SKIP] $name"
    elif [ "$rc" -eq 2 ]; then
      echo "[VOID] $name"; void=1
    else
      echo "[FAIL] $name"; fail=1
    fi
  fi
}

# Forward toolchain, drum-reference and Chromium paths so child builds and tests see them.
pass_env() {
  export JIELI_TOOLCHAIN="${JIELI_TOOLCHAIN:-${X0X_TOOLCHAIN:-$HOME/.jieli/toolchain}}"
  export X0X_TOOLCHAIN="${X0X_TOOLCHAIN:-$JIELI_TOOLCHAIN}"
  export AC79_SDK="${AC79_SDK:-$HOME/fw-AC79_AIoT_SDK}"
  [ -n "${EIGHTW8_DIR:-}" ] && export EIGHTW8_DIR
  [ -n "${NINEW9_DIR:-}" ] && export NINEW9_DIR
  if [ -z "${DNBOX_CHROMIUM_EXECUTABLE:-}" ] && [ -n "${CHROMIUM_EXECUTABLE:-}" ]; then
    export DNBOX_CHROMIUM_EXECUTABLE="$CHROMIUM_EXECUTABLE"
  fi
  [ -n "${DNBOX_CHROMIUM_EXECUTABLE:-}" ] && export DNBOX_CHROMIUM_EXECUTABLE
  [ -n "${CHROMIUM_EXECUTABLE:-}" ] && export CHROMIUM_EXECUTABLE
}

echo "DNBOX-GATE repo=firmware lane=$LANE sha=$SHA started=$TS (UTC+10)"
clean()      { git diff --quiet && git diff --cached --quiet; }
setup()      { pass_env
               local miss=0
               local tc="${JIELI_TOOLCHAIN}/pi32v2/bin/clang"
               local boot="${AC79_SDK}/cpu/wl82/tools/uboot.boot"
               [ -x "$tc" ] || { echo "missing JieLi toolchain clang: $tc"; miss=1; }
               [ -f "$boot" ] || { echo "missing AC79 SDK uboot.boot: $boot"; miss=1; }
               if ! python3 tools/check_original_inputs.py >/dev/null; then
                 echo "missing original synth inputs (restore from Library, see docs/LIBRARY-INDEX.md)"
                 miss=1
               fi
               [ "$miss" -eq 0 ] || return 2; }
fetch_base() { if [ "${DNBOX_GATE_ONLINE:-0}" = "1" ]; then
                 git fetch -q origin integration/dnbox-consolidated || return 1
                 return 0
               fi
               echo "SKIP fetch of origin/integration/dnbox-consolidated: offline default (set DNBOX_GATE_ONLINE=1 to fetch)"
               return 3; }
resolve_base() { if git rev-parse -q --verify 'refs/remotes/origin/integration/dnbox-consolidated^{commit}' >/dev/null; then
                   BASE=origin/integration/dnbox-consolidated
                 elif git rev-parse -q --verify 'refs/heads/integration/dnbox-consolidated^{commit}' >/dev/null; then
                   BASE=integration/dnbox-consolidated
                 else
                   echo "no local integration/dnbox-consolidated ref; merge it in or run with DNBOX_GATE_ONLINE=1"
                   return 1
                 fi
                 echo "using BASE=$BASE"; }
no_loss()    { resolve_base || return 1
               git merge-base --is-ancestor "$BASE" HEAD || { echo "branch does not contain $BASE: merge it in (no rebase)"; return 1; }
               local del; del=$(git diff --name-only --diff-filter=D "$BASE" HEAD)
               [ -z "$del" ] && return 0
               local lf="docs/lanes/$LANE.md" miss=0
               while read -r f; do grep -qF -- "$f" "$lf" 2>/dev/null || { echo "deleted without reason in $lf: $f"; miss=1; }; done <<< "$del"
               return $miss; }
limits()     { resolve_base || return 1
               local bad=0
               git diff --name-only "$BASE" HEAD | grep -E '^\.github/workflows/' && { echo "workflow file changed"; bad=1; }
               git ls-files | grep -iE '\.ipa$|SuperJ6.*AU.*extension.*\.zip' && { echo "SuperJ6 installer in Git"; bad=1; }
               git grep -nIE '(ghp_|gho_|github_pat_)[A-Za-z0-9_]{20,}|-----BEGIN [A-Z ]*PRIVATE KEY-----|AKIA[0-9A-Z]{16}' -- . ':!*.md' && { echo "possible Secret"; bad=1; }
               # PSRAM grep check removed: the Elite/Pro fit test is the only memory gate.
               while read -r m o s p; do
                 sz=$(git cat-file -s "$o")
                 if [ "$sz" -ge 52428800 ]; then echo "≥50MB not in LFS: $p"; bad=1; fi
               done < <(git ls-files -s)
               return $bad; }
host_tests() { pass_env
               if ! git rev-parse -q --verify 'refs/tags/archive/build/0.0.22^{commit}' >/dev/null; then
                 if [ "${DNBOX_GATE_ONLINE:-0}" = "1" ]; then
                   echo "fetching tag archive/build/0.0.22 (missing from this clone)"
                   git fetch origin refs/tags/archive/build/0.0.22:refs/tags/archive/build/0.0.22 || return 1
                 else
                   echo "SKIP fetch of archive/build/0.0.22: offline default, and the tag is not in this checkout"
                   echo "host tests need refs/tags/archive/build/0.0.22 locally, or DNBOX_GATE_ONLINE=1"
                   return 1
                 fi
               else
                 echo "using local archive/build/0.0.22 (no fetch)"
               fi
               sh host/build_host.sh && sh tests/run_tests.sh | tee "$OUT/tests.txt"; local r=${PIPESTATUS[0]}
               grep -E '^\s*(FAIL|not-run)\b' "$OUT/tests.txt" && return 1; return $r; }
budget_from_log() {
  # tools/build.py prints "XIP free: N B" and "pool margin: N B" for the model just built.
  local log=$1 model=$2 xip pool
  xip=$(grep -E '^XIP free: -?[0-9]+ B$' "$log" | tail -1 | awk '{print $3}')
  pool=$(grep -E '^pool margin: -?[0-9]+ B$' "$log" | tail -1 | awk '{print $3}')
  if [ -z "$xip" ] || [ -z "$pool" ]; then
    echo "$model budget numbers not found in $log (NOT_RUN = FAIL): tools/build.py must print 'XIP free: <bytes> B' and 'pool margin: <bytes> B'"
    return 1
  fi
  printf '%s\n' "$xip" > "$OUT/${model}-xip.txt"
  printf '%s\n' "$pool" > "$OUT/${model}-pool.txt"
  echo "$model: XIP free: $xip B (min $XIP_MIN); pool margin: $pool B (min $POOL_MIN)"
}
fw_build()   { pass_env
               MODEL=elite ./build.sh > "$OUT/elite-build.log" 2>&1
               local erc=$?
               cat "$OUT/elite-build.log"
               [ "$erc" -eq 0 ] || { echo "Elite build failed"; return 1; }
               [ -f build/x0x-smk37-elite.fwsc ] || { echo "missing build/x0x-smk37-elite.fwsc"; return 1; }
               cp -f build/x0x-smk37-elite.fwsc "$OUT/elite.fwsc" || return 1
               budget_from_log "$OUT/elite-build.log" elite || return 1

               MODEL=pro ./build.sh > "$OUT/pro-build.log" 2>&1
               local prc=$?
               cat "$OUT/pro-build.log"
               [ "$prc" -eq 0 ] || { echo "Pro build failed"; return 1; }
               [ -f build/x0x-smk37.fwsc ] || { echo "missing build/x0x-smk37.fwsc"; return 1; }
               cp -f build/x0x-smk37.fwsc "$OUT/pro.fwsc" || return 1
               budget_from_log "$OUT/pro-build.log" pro || return 1
             }
flashmap()   { python3 tools/gate_flashmap.py firmware/hal/fm1_flash_map.h firmware/src/storage.c; }
budget()     { local model xip pool bad=0
               for model in elite pro; do
                 if [ ! -f "$OUT/${model}-xip.txt" ] || [ ! -f "$OUT/${model}-pool.txt" ]; then
                   echo "$model budget numbers not found (NOT_RUN = FAIL): Elite and Pro must both print 'XIP free' and 'pool margin'"
                   return 1
                 fi
                 xip=$(cat "$OUT/${model}-xip.txt")
                 pool=$(cat "$OUT/${model}-pool.txt")
                 echo "$model: XIP free: $xip B (min $XIP_MIN); pool margin: $pool B (min $POOL_MIN)"
                 if [ "$xip" -lt "$XIP_MIN" ] || [ "$pool" -lt "$POOL_MIN" ]; then
                   echo "$model fit test failed: XIP free $xip B (min $XIP_MIN), pool margin $pool B (min $POOL_MIN)"
                   bad=1
                 fi
               done
               return $bad; }
ui_defects() { python3 tests/ui_defects/test_gate.py || return 1
               [ -f tests/ui_defects/first_slice/evaluate.py ] || return 0
               local render="${DNBOX_UI_RENDER_DIR:-$OUT/ui-render}"
               if [ ! -f "$render/source-closure.json" ]; then
                 echo "evaluate.py render receipts missing at $render/source-closure.json (NOT_RUN = FAIL)"
                 return 1
               fi
               # Force a real failure (not VOID): evaluate.py may exit 2 on a Python error.
               python3 tests/ui_defects/first_slice/evaluate.py --render-dir "$render" --output "$OUT/ui-eval.json" || return 1
             }
drone()      { local n; n=$(git grep -niwI drone -- firmware web tests ':!*CHANGELOG*' ':!docs/history/*' | wc -l); echo "Drone mentions: $n"; [ "$LANE" = sound ] || [ "$LANE" = integ ] || return 0; [ "$n" -eq 0 ]; }

step clean-tree clean
step setup setup
step fetch-base fetch_base
step no-loss no_loss
step limits limits
if [ $void = 0 ]; then step host-tests host_tests; step build-elite-pro fw_build; step flashmap flashmap; step budget budget; step ui-defects ui_defects; fi
step drone drone
R=PASS; [ $void = 1 ] && R=VOID; [ $fail = 1 ] && R=FAIL
echo "DNBOX-GATE RESULT $R sha=$SHA lane=$LANE log=$LOG"
[ $R = PASS ] && exit 0; [ $R = VOID ] && exit 2; exit 1
