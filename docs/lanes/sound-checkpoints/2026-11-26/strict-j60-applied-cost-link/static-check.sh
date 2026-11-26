#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")";pwd); W=$(dirname "$D")
N="$W/prereqs/jieli-linux-toolchains-20260730.1/pi32v2/bin"
for m in elite pro;do
 old=; for arm in busy cost;do
  e="$W/warm-j60-applied-$arm-top/link-receipts/$m-on.never-install.elf"
  "$N/nm" -S "$e" | grep -E ' (run_job|fm1_alnk0_irq|plat_ticks24|lab_miss_busy)$'
  read -r a z < <("$N/nm" -S "$e"|awk '$4=="run_job"{print $1,$2}')
  c=$("$N/objdump" -d -start-address="0x$a" -stop-address="$(printf '0x%x' "$((16#$a+16#$z))")" "$e" |grep -c 'call .*<plat_ticks24')
  [[ -z "$old" || "$old" == "$c" ]]
  old="$c"; printf '%s %s run_job static timer-read callsites=%s\n' "$m" "$arm" "$c"
 done
done
