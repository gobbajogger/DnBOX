set -eu
for x in base new;do cp "$x-nor-pristine" "$x-nor.bin";done
flock -n -E 79 /tmp/dnbox-gate-firmware.lock bash -c '
set -e
for x in base new;do
 env FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM="$x.ram" timeout 120s "$1/restored-gate/fm1-live-ba83317a" --fwsc "$1/out/DnBOX-0.0.27-SMK37-elite-73749f55-test.fwsc" --flash "$PWD/$x-nor.bin" --mhz 360 --shot 230 >"$x.log" 2>&1
 cmp "$x-nor.bin" "$x-nor-pristine"
done' -- "$W"
