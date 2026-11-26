from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
tag='#define DEFAULT_SRATE'
pre=r'''
#include <stdint.h>
#define RECENT 8u
#define STEP 64u
typedef struct {uint32_t clock;float recent[RECENT], samples[];} Age;
static unsigned slots(unsigned n){return (n+STEP-1u)/STEP+2u;}
static unsigned bytes(unsigned n){return sizeof(Age)+slots(n)*sizeof(float);}
static void age_put(void *data,unsigned n,float x)
{
    Age *p=data; unsigned t=p->clock++;
    p->recent[t%RECENT]=x;
    if(t%STEP==0)p->samples[t/STEP%slots(n)]=x;
}
static float age_read(const void *data,unsigned n,unsigned i)
{
    const Age *p=data;unsigned end=p->clock-1, w=end%n;
    unsigned age=(w+n-i)%n;
    if(age>end)return 0.f;
    const unsigned t=end-age;
    if(age<RECENT)return p->recent[t%RECENT];
    unsigned k=t/STEP,b=slots(n);
    float L=p->samples[k%b];
    /* No uncommitted future anchor. Exact for the last eight samples. */
    if((k+1u)*STEP>end)return L;
    float R=p->samples[(k+1u)%b];
    return L+(R-L)*(float)(t%STEP)*(1.f/(float)STEP);
}
void report(sp_revsc* p)
{
    unsigned sum=0;
    for(unsigned i=0;i<8;i++)sum+=bytes(p->delayLines[i].bufferSize);
    fprintf(stderr,"{\"history_bytes\":%u,\"source_context_host\":%zu}\n",sum,sizeof(*p));
}
'''
s=s.replace(tag,pre+tag,1)
def replace(a,b):
    global s
    assert s.count(a)==1
    s=s.replace(a,b)
replace('nBytes += (delay_line_max_samples(sr, iPitchMod, n) * (int) sizeof(SPFLOAT));','nBytes += bytes(delay_line_max_samples(sr,iPitchMod,n));')
replace('memset(lp->buf, 0, sizeof(SPFLOAT) * lp->bufferSize);','memset(lp->buf,0,bytes(lp->bufferSize));')
replace('lp->buf[lp->writePos] = (SPFLOAT) ((n & 1 ? ainR : ainL)\n                                 - lp->filterState);','age_put(lp->buf,bufferSize,(SPFLOAT)((n&1?ainR:ainL)-lp->filterState));')
s,n=re.subn(r'\(SPFLOAT\)\s*\(lp->buf\[([^\]]+)\]\)',r'age_read(lp->buf,bufferSize,\1)',s)
s,m=re.subn(r'\(SPFLOAT\)\s*lp->buf\[([^\]]+)\]',r'age_read(lp->buf,bufferSize,\1)',s)
assert n==m==4,(n,m)
Path(sys.argv[2]).write_text(s)
