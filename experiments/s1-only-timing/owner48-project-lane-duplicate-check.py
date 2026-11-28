from pathlib import Path
import subprocess, sys, tempfile
s=Path(sys.argv[1]).read_text()
a=s.index('static __attribute__((noinline)) int project_lane_duplicate')
b=s.index('\n}',a)+2
preamble='''#include <assert.h>
#include <stdint.h>
#include <stdio.h>
typedef struct { uint8_t used,t,pat,part,v,i; } lane_t;
'''
test=r'''
static int old(const lane_t *lanes,int k,int mode)
{
    const lane_t *l=&lanes[k];
    for(int j=0;j<k;j++)
        if(lanes[j].used && lanes[j].t==l->t && lanes[j].pat==l->pat &&
            lanes[j].part==l->part && lanes[j].v==l->v && (!mode || lanes[j].i==l->i))return 1;
    return 0;
}
static uint32_t seed=1;
static uint8_t sample(void){seed=seed*1664525u+1013904223u;return seed>>24;}
int main(void){
    lane_t a[128];
    unsigned n=0;
    for(int r=0;r<9216;r++){
        for(unsigned j=0;j<128;j++) a[j]=(lane_t){sample()%3,sample()%5,sample()%6,sample()%4,sample()%4,sample()%4};
        unsigned base=sample()%128,mut=sample()%128;
        a[mut]=a[base];
        switch(r%7){
            case 0:a[mut].used=0;break;
            case 1:a[mut].t+=1;break;
            case 2:a[mut].pat+=1;break;
            case 3:a[mut].part+=1;break;
            case 4:a[mut].v+=1;break;
            case 5:a[mut].i+=1;break;
        }
        for(int k=0;k<128;k++)for(int m=0;m<=1;m++){
            assert(old(a,k,m)==project_lane_duplicate(a,k,m)); n++;
        }
    }
    printf("%u exact cold duplicate scan decisions\n",n);
}
'''
with tempfile.TemporaryDirectory() as d:
    c=Path(d)/'test.c'; o=Path(d)/'run'; c.write_text(preamble+s[a:b]+test)
    subprocess.run(['cc','-O2','-Wall','-Wextra','-Werror','-fsanitize=undefined','-fno-sanitize-recover=all',str(c),'-o',str(o)],check=True)
    subprocess.run([str(o)],check=True)
