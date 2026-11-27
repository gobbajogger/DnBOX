from pathlib import Path
import sys, re
p=Path(sys.argv[1]).read_text()
f=lambda n:re.search(r'(?m)^static (?:__attribute__\(\(noinline\)\) )?void '+n+r'\([^\n]*\)\n\{.*?^\}',p,re.S|re.M).group(0)
print('#include <stdint.h>\n#include <stdio.h>\n#include <string.h>\n#include <assert.h>\nstatic uint8_t ed_out[732];\nstatic uint32_t ed_n;')
print(f('ed_b'))
print(f('ed_u'))
print('''
static void baseline(uint32_t v, unsigned n) {
    switch (n) {
    case 5: ed_b(v); ed_b(v>>7); ed_b(v>>14); ed_b(v>>21); ed_b(v>>28); break;
    case 3: ed_b(v); ed_b(v>>7); ed_b(v>>14); break;
    case 2: ed_b(v); ed_b(v>>7); break;
    default: assert(0);
    }
}
int main(void) {
    uint8_t ref[732]; unsigned long cases=0; uint32_t rng=1;
    const uint32_t edges[]={0,1,0x7f,0x80,0x3fff,0x4000,0x1fffff,0x200000,0xfffffff,0x10000000,0xffffffff};
    for(int j=0;j<20000;j++) {
        rng=rng*1664525u+1013904223u;
        uint32_t v=j<(int)(sizeof edges/sizeof edges[0]) ? edges[j]:rng;
        for (unsigned n = 2; n<=5; n+=n==3?2:1) {
            for(unsigned k=725; k<=731; k++) {
                memset(ed_out,0x5a,sizeof ed_out); ed_n=k; baseline(v,n);
                const unsigned end=ed_n; memcpy(ref,ed_out,sizeof ref);
                memset(ed_out,0x5a,sizeof ed_out); ed_n=k; ed_u(v,(int)n);
                assert(end==ed_n && memcmp(ref,ed_out,sizeof ref)==0); cases++;
            }
        }
    }
    printf("%lu exact field/truncation cases\\n",cases);
    return 0;
}
''')
