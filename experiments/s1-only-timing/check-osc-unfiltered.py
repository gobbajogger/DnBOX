"""Extract the original and candidate oscillator and compare float/phase bits."""
from pathlib import Path
import subprocess
import sys
import tempfile

base, candidate = map(Path, sys.argv[1:])

def render(path, name):
    s = path.read_text()
    shape = s[s.index('static float blep('):s.index('typedef struct { int k; float frac; }')]
    morph = s[s.index('static float morph_step('):s.index('static float morph(')]
    return (shape + morph).replace('static float morph_step(', f'float {name}(')

common = """#include <stdint.h>
#define Q32 4294967296.0f
"""
test = r"""
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
float baseline(uint32_t *,float,int,float,int,uint32_t);
float candidate(uint32_t *,float,int,float,int,uint32_t);
static uint32_t next(uint32_t *state) {
    *state=*state*1664525u+1013904223u;
    return *state;
}
int main(void) {
    uint32_t seed=11;
    for (unsigned i=0;i<8000000u;i++) {
        uint32_t a=next(&seed), b=a, step=next(&seed);
        if ((i & 63u) == 0) a=b=(i<<20);
        float dt=(float)(next(&seed)>>8)/(2.f*16777216.f);
        int k=(int)((i>>1)%5)-1;
        float f=(float)(next(&seed)>>8)/16777216.f;
        if (!(i&7)) f=0.f;
        if ((i&7)==1) f=1.f;
        float x=baseline(&a,dt,k,f,i&1u,step);
        float y=candidate(&b,dt,k,f,i&1u,step);
        assert(a==b && !memcmp(&x,&y,sizeof x));
    }
    puts("8,000,000 exact oscillator output and phase checks");
}
"""
with tempfile.TemporaryDirectory() as td:
    d = Path(td)
    for path, name in ((base, "baseline"), (candidate, "candidate")):
        (d / f"{name}.c").write_text(common + render(path, name))
    (d / "test.c").write_text(test)
    subprocess.run(["cc", "-O2", "-ffp-contract=off", "baseline.c", "candidate.c",
                    "test.c", "-o", "check"], cwd=d, check=True)
    subprocess.run(["./check"], cwd=d, check=True)
