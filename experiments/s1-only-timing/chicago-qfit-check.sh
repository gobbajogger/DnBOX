#!/bin/sh
set -eu
if [ "$#" -ne 3 ]; then echo "usage: $0 <source tree> <baseline generated dir> <candidate generated dir>" >&2; exit 2; fi
S="$(realpath "$1")/experiments/original_synth"
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
CC=${CC:-cc}
CFLAGS="-DX0X_CHICAGO_FAST32=1 -DX0X_CHICAGO_FOLD_RESIDUAL_MIN=1"
$CC -O2 -Wall -Wextra -Werror $CFLAGS -I"$2" -Doriginal_chicago_packed_lookup=oldpair -Doriginal_chicago_packed_cutoff_a=oldonly -c "$S/chicago_packed.c" -o "$T/old.o"
$CC -O2 -Wall -Wextra -Werror $CFLAGS -I"$3" -c "$S/chicago_packed.c" -o "$T/new.o"
cat > "$T/bench.c" <<'C'
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int oldpair(const void *,uint32_t,float *,float *);
int oldonly(uint32_t,float *);
int original_chicago_packed_lookup(const void *,uint32_t,float *,float *);
int original_chicago_packed_cutoff_a(uint32_t,float *);
int main(void) {
    uint32_t checks=0;
    for(unsigned pass=0;pass<3;pass++) {
        for(uint32_t k=0;k<16385u;k++){
            uint32_t i=(k*(pass==0?1:pass==1?1063:12289))%16385;
            float ol[2],nw[2],a,b;
            assert(oldpair(0,i,ol,ol+1)==1);
            assert(original_chicago_packed_lookup(0,i,nw,nw+1)==1);
            assert(memcmp(ol,nw,sizeof ol)==0);
            checks+=2;
            assert(oldonly(i,&a)==1 && original_chicago_packed_cutoff_a(i,&b)==1);
            assert(memcmp(&a,&b,sizeof a)==0);
            checks++;
            assert(original_chicago_packed_lookup(0,i,nw,nw+1)==1);
            assert(memcmp(ol,nw,sizeof ol)==0);
            checks+=2;
        }
    }
    float a,b;
    assert(!original_chicago_packed_lookup(0,16385,&a,&b));
    assert(!original_chicago_packed_cutoff_a(16385,&a));
    printf("Exact FAST32 C comparison passed: %u float words; cache hits, paired and display-only calls.\n",checks);
}
C
$CC -O2 -Wall -Wextra -Werror "$T/bench.c" "$T/old.o" "$T/new.o" -o "$T/run"
"$T/run"
