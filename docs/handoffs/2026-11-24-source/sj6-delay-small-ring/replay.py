#!/usr/bin/env python3
"""Use the exact sealed DelaySync DSP source, varying only sample storage."""
import array, hashlib, math, os, pathlib, subprocess, tempfile
here=pathlib.Path(__file__).resolve().parent
patches=[here.parent/"sj6-delay-typed/delay-typed.patch",here.parent/"sj6-delay-sync118/delay-sync118.patch"]
for p, sha in zip(patches,[
    "c2339b36a78f3ffe935a92da6967f9bb6d06dcb37214e6561034e05d5bb210ae",
    "d7ed9b4454ead2386c9213e9be2838d31753c2574953e204c68df043d4dda312",
],strict=True):
    assert hashlib.sha256(p.read_bytes()).hexdigest()==sha,p
def additions(p,f):
    return p.read_text().split("\n+++ b/"+f+"\n")[1].split("\n--- a/")[0].splitlines()
f="firmware/src/dsp/juno6_factory.inc"
src="\n".join(x[1:] for x in additions(patches[0],f) if x.startswith("+"))+"\n"
differ=additions(patches[1],f)
old=[];new=[]
def apply():
    if old:
        a="\n".join(old)+"\n";b="\n".join(new)+"\n"
        global src
        assert src.count(a)==1,a
        src=src.replace(a,b)
for line in differ:
    if line.startswith("@@ "):
        apply();old=[];new=[]
    elif line:
        tag=line[0]
        if tag in " -":old.append(line[1:])
        if tag in " +":new.append(line[1:])
apply()
assert "#endif" in src
# Retain the exact same time/profile code and exact audio dry/wet/feedback ops.
# Bind each compact state's two-channel ring only to its private owner slice.
compact=src.replace("float *l,float *r,unsigned capacity","uint8_t *l,uint8_t *r,unsigned capacity").replace("l==r","l!=r")
a="float left=fmaf(frac,s->delay24.left[b]-s->delay24.left[a],s->delay24.left[a]);"
b="float right=fmaf(frac,s->delay24.right[b]-s->delay24.right[a],s->delay24.right[a]);"
for ch, needle in [("left",a),("right",b)]:
    assert compact.count(needle)==1
    q=int(ch=="right")
    compact=compact.replace(needle,f"""float {ch}_a=sj6_ring12_read(s->delay24.left,a,{q}),{ch}_b=sj6_ring12_read(s->delay24.left,b,{q});
        float {ch}=fmaf(frac,{ch}_b-{ch}_a,{ch}_a);""")
a="""s->delay24.left[at]=fmaf(s->delay24.feedback,left,l[i]);
        s->delay24.right[at]=fmaf(s->delay24.feedback,right,r[i]);"""
assert compact.count(a)==1
compact=compact.replace(a,"""float next_l=fmaf(s->delay24.feedback,left,l[i]),next_r=fmaf(s->delay24.feedback,right,r[i]);
        const int valid=isfinite(next_l)&&isfinite(next_r)&&fabsf(next_l)<=7.9375f&&fabsf(next_r)<=7.9375f;
        if(!valid)s->delay24.range_fault=1;
        sj6_ring12_write(s->delay24.left,at,valid?next_l:0.f,valid?next_r:0.f);""")
compact=compact.replace("!s->delay24.clock_refused){","!s->delay24.clock_refused && !s->delay24.range_fault){")
prefix=r"""
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "sj6_delay_ring12.h"
#define X0X_HOST 1
#define X0X_SJ6_DELAY_WITNESS 1
#if COMPACT
 typedef uint8_t cell;
#else
 typedef float cell;
#endif
enum { SJ6_DELAY24=90,SJ6_DELAY27=91,SJ6_DELAY25=92,SJ6_DELAY26=93,SJ6_DELAYSYNC=94 };
typedef struct {
 int typed;float sample_rate;
 struct {
    cell *left,*right;
    float dry,from,target,feedback;
    unsigned tap,pos,elapsed,tick,capacity;
    float delay,delay_from,delay_target,fb_from,fb_target,bpm;
    unsigned time_raw,fb_raw,sync,clock_refused,delay_elapsed,fb_elapsed;
    unsigned enabled,mix,pending,range_fault;
 } delay24;
} juno6_t;
"""
main=r"""
enum {CAP=5514, FRAMES=60000, TOTAL=CAP*6};
static juno6_t s[2]={{.typed=1,.sample_rate=44100.f},{.typed=1,.sample_rate=44100.f}};
#if COMPACT
static uint8_t buffer[TOTAL+2];
static sj6_ring12 rings[2];
#else
static float buffer[2][2][CAP];
#endif
static void arm(void)
{
#if COMPACT
 memset(buffer,0xac,sizeof buffer);
 assert(!sj6_ring12_pair(rings,buffer+1,TOTAL-1,CAP));
 assert(!sj6_ring12_pair(rings,buffer+1,TOTAL,CAP+1));
 assert(!sj6_ring12_pair(NULL,buffer+1,TOTAL,CAP));
 for(unsigned i=0;i<sizeof buffer;i++)assert(buffer[i]==0xac);
 assert(!rings[0].data&&!rings[1].data);
 assert(sj6_ring12_pair(rings,buffer+1,TOTAL,CAP));
 assert(!sj6_ring12_pair(rings,buffer+1,TOTAL,CAP));
 assert(rings[1].data-rings[0].data==CAP*3);
 for(unsigned i=0;i<2;i++)assert(juno6_delay24_arm(s+i,rings[i].data,rings[i].data,CAP,0,10000));
#else
 for(unsigned i=0;i<2;i++)assert(juno6_delay24_arm(s+i,buffer[i][0],buffer[i][1],CAP,0,10000));
#endif
}
static void tick(unsigned owner,float *l,float *r,unsigned n)
{
 unsigned phase=s[owner].delay24.pos,peer=s[1-owner].delay24.pos;
 juno6_delay24_process(s+owner,l,r,n);
 assert(s[owner].delay24.pos==(phase+n)%CAP);
 assert(s[1-owner].delay24.pos==peer);
}
static void settle(void)
{
 for(unsigned i=0;i<10008;i++)for(unsigned v=0;v<2;v++){
  float l=0,r=0;tick(v,&l,&r,1);
 }
}
static void measure_cpu(void)
{
    float l[128],r[128];
    volatile float result=0.f;
    clock_t begin=clock();
    for(unsigned i=0;i<8000;i++)for(unsigned v=0;v<2;v++){
        for(unsigned k=0;k<128;k++){l[k]=.02f;r[k]=-.02f;}
        juno6_delay24_process(s+v,l,r,128);
        result+=l[0];
    }
    clock_t end=clock();
    assert(isfinite(result));
    fprintf(stderr,"%s host CPU only (UBSan, 2,048,000 owner frames): %.1f ms\n",
        COMPACT?"packed":"float",(double)(end-begin)*1000/CLOCKS_PER_SEC);
}
int main(void){
 arm();
 for(unsigned v=0;v<2;v++){
  assert(!juno6_delay25_profile(s+v,127,120,0));
  assert(!juno6_delay25_profile(s+v,127,120,1));
  assert(!juno6_delay25_profile(s+v,8,0,1));
  assert(!s[v].delay24.time_raw&&s[v].delay24.delay_target==1);
 }
 assert(juno6_delay25_profile(s,8,120,0));
 /* same raw can be retimed from free to division 1, then reject slower clocks */
 assert(juno6_delay25_clock(s,120,1)&&s[0].delay24.delay_target==2067.1875f);
 assert(!juno6_delay25_clock(s,30,1)&&s[0].delay24.delay_target==2067.1875f);
 assert(juno6_delay25_profile(s+1,9,120,0));
 assert(juno6_delay24_set(s,SJ6_DELAY26,103)&&juno6_delay24_set(s+1,SJ6_DELAY26,67));
 assert(juno6_delay24_set(s,SJ6_DELAY27,127)&&juno6_delay24_set(s+1,SJ6_DELAY27,100));
 settle();
 /* Refusal uses same normal prepare contract: persist requested current clock
  * elsewhere, withhold wet but continue updating past samples. */
 for(unsigned v=0;v<2;v++)assert(juno6_delay24_set(s+v,SJ6_DELAY24,1));
 settle();
 for(unsigned v=0;v<2;v++){
  assert(!juno6_delay25_profile(s+v,127,30,1));
  s[v].delay24.clock_refused=1;
  float l=v?0.f:.319117f,r=v?-.26367f:0.f;
  const float d=l,e=r;
  tick(v,&l,&r,1);
  assert(!memcmp(&l,&d,sizeof l)&&!memcmp(&r,&e,sizeof r));
 }
 assert(juno6_delay25_clock(s,120,1)&&juno6_delay25_clock(s+1,120,0));
 for(unsigned i=0;i<FRAMES;i++)for(unsigned v=0;v<2;v++){
  float l=0,r=0;
  tick(v,&l,&r,1);
  assert(v?l==0.f:r==0.f);
  const float audible=v?r:l;
  assert(!s[v].delay24.range_fault);
  assert(fwrite(&audible,4,1,stdout)==1);
 }
 measure_cpu();
 for(unsigned v=0;v<2;v++)assert(juno6_delay24_set(s+v,SJ6_DELAY24,0));
 settle();
 for(unsigned v=0;v<2;v++){
  const float d=.147583f,e=-.131835f;
  float l=d,r=e;
  tick(v,&l,&r,1);assert(!memcmp(&l,&d,4)&&!memcmp(&r,&e,4));
 }
#if COMPACT
 assert(buffer[0]==0xac&&buffer[TOTAL+1]==0xac);
 /* An out-of-profile sample is A/B observable; never silently wrap. */
 float l=8.f,r=0.f;tick(0,&l,&r,1);assert(s[0].delay24.range_fault&&!s[1].delay24.range_fault&&l==8.f);
#endif
}
"""
with tempfile.TemporaryDirectory() as tmp:
    t=pathlib.Path(tmp)
    ref=None
    for codec,code in ((0,src),(1,compact)):
        (t/"run.c").write_text(prefix+code+main)
        subprocess.run([os.getenv("CC","cc"),"-std=c99","-O2","-fsanitize=undefined","-fno-sanitize-recover=all","-I"+str(here),f"-DCOMPACT={codec}",str(t/"run.c"),"-lm","-o",str(t/"test")],check=True)
        out=array.array("f")
        out.frombytes(subprocess.check_output([str(t/"test")]))
        if not codec:
            ref=out
            continue
        for v,name in ((0,"A: Sync @120"),(1,"B: Free @raw9")):
            a=ref[v::2]; b=out[v::2]
            first=lambda x: next(i+1 for i,y in enumerate(x) if abs(y)>1e-5)
            assert first(a)==first(b) and max(map(abs,b))>.1
            snr=10*math.log10(sum(x*x for x in a)/sum((x-y)**2 for x,y in zip(a,b)))
            peak=max(abs(x-y) for x,y in zip(a,b))
            print(f"{name}: first echo {first(b)} frames (same float timing); wet SNR {snr:.2f} dB, max error {peak:.6f}")
        print("PASS packed 12-bit actual two-owner DSP trajectory: 33084 B stereo total vs float 88224 B; exact dry/clock refusal/history, opposite channels, canaries, range fault")
