#!/bin/sh
set -eu
export PATH=/usr/bin:/bin HOME=/home/tester CC=/usr/bin/gcc TMPDIR=/tmp PYTHONDONTWRITEBYTECODE=1
id
grep TracerPid /proc/self/status
cd /tmp
printf '#include <stdlib.h>\nint main(void){char *volatile p=malloc(123);if(!p)abort();p[0]=1;free(p);return 0;}\n' > clean.c
printf '#include <stdlib.h>\n__attribute__((noinline)) void leak(void){char *volatile p=malloc(123);if(!p)abort();p[0]=1;}\nint main(void){leak();return 0;}\n' > leak.c
for t in clean leak; do "$CC" -O1 -g -fsanitize=address,undefined -fno-sanitize-recover=all "$t.c" -o "$t"; done
export ASAN_OPTIONS=detect_leaks=1:exitcode=23:halt_on_error=1 LSAN_OPTIONS=detect_leaks=1:exitcode=23 UBSAN_OPTIONS=halt_on_error=1
./clean
echo I4_CLEAN_EXIT=0
rc=0; ./leak >leak.log 2>&1 || rc=$?
cat leak.log
test "$rc" = 23
grep -F '123 byte(s) leaked in 1 allocation(s)' leak.log
echo I4_LEAK_EXIT=$rc
cd /mnt/source
/usr/bin/python3 tests/program_values_compact_test.py
echo I4_COMPACT_EXIT=0
