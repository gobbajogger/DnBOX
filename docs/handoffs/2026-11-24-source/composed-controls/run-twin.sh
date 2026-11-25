#!/usr/bin/env bash
set -euo pipefail
root=$(pwd -P)
o=$(cat recovery/fit/.current)
p=$(realpath -m "$o/$1")
mkdir -p "$p"
python recovery/source/pin-forward.py "$o/donor" "$p/SOURCE.sha256"
declare -a pid
for model in elite pro; do
  d="$p/$model"; mkdir -p "$d"
  (
  export BUILD_MODELS="$model" ADD_CONTROL_FLAGS="$(cat "$o/flags")"
  export FORWARD_SLOT3_LINK="$root/recovery/fit/link-slot3-forward.py" FORWARD_MANIFEST="$p/SOURCE.sha256"
  bash recovery/fit/build-both.sh "$root" "$o/donor" "$d" slot3 > "$d/driver.log" 2>&1
  ) &
  pid+=("$!")
done
status=0
for x in "${pid[@]}"; do wait "$x" || status=1; done
for model in elite pro; do echo "$model";tail -25 "$p/$model/driver.log";done
exit "$status"
