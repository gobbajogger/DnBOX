#!/usr/bin/env python3
"""Compile the actual bounded SJ6 witness with only the sample cell adapter varied."""
import array, math, os, pathlib, re, subprocess, sys, tempfile
p = pathlib.Path(sys.argv[1])
source = p.read_text().split("#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)")[-1].rsplit("#endif",1)[0]
prefix = r"""
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum {SJ6_DELAY24=90,SJ6_DELAY27=91,SJ6_DELAY25=92,SJ6_DELAY26=93};
#if CODEC==0
typedef float cell; static float get(cell x){return x;}static cell put(float x){return x;}
#elif CODEC==3
typedef int16_t cell;
static float get(cell x){return x/32767.f;}
static cell put(float x){return (cell)lroundf(fmaxf(-1.f,fminf(1.f,x))*32767.f);}
#elif CODEC==4
typedef uint8_t cell;
static float get4(unsigned x){
    int q=(x&8)?-(int)(x&7):(int)(x&7);
    return copysignf(expm1f(fabsf(q/7.f)*5.54517744f)/255.f,q);
}
static unsigned put4(float x){
    unsigned q=(unsigned)lroundf(log1pf(255.f*fminf(1.f,fabsf(x)))*(7.f/5.54517744f));
    return (x<0.f?8:0)|q;
}
#else
typedef int8_t cell;
#if CODEC==1
static float get(cell x){return x/127.f;}
static cell put(float x){return (cell)lroundf(fmaxf(-1.f,fminf(1.f,x))*127.f);}
#else
static float get(cell x){return copysignf(expm1f(fabsf(x/127.f)*5.54517744f)/255.f,x);}
static cell put(float x){return (cell)lroundf(copysignf(log1pf(255.f*fminf(1.f,fabsf(x)))*22.902784275f,x));}
#endif
#endif
#if CODEC==4
#define CELLS(n) (((n)+1)/2)
static float get_at(cell *p,unsigned at){return get4((p[at/2]>>(4*(at&1)))&15);}
static void put_at(cell *p,unsigned at,float x){
    unsigned shift=4*(at&1);p[at/2]=(p[at/2]&~(15u<<shift))|(put4(x)<<shift);
}
#else
#define CELLS(n) (n)
static float get_at(cell *p,unsigned at){return get(p[at]);}
static void put_at(cell *p,unsigned at,float x){p[at]=put(x);}
#endif
typedef struct {
 int typed;float sample_rate;
 struct {
    cell *left,*right;
    float dry,from,target,feedback;
    unsigned tap,pos,elapsed,tick,capacity;
    float delay,delay_from,delay_target,fb_from,fb_target,bpm;
    unsigned time_raw,fb_raw,sync,clock_refused,delay_elapsed,fb_elapsed;
    unsigned enabled,mix,pending;
 } delay24;
} juno6_t;
"""
source=source.replace("float *l,float *r,unsigned capacity","cell *l,cell *r,unsigned capacity")
for ch, name in (("left","l"),("right","r")):
    source, n=re.subn(fr"s->delay24\.{ch}\[(a|b)\]", lambda m: f"get_at(s->delay24.{ch},{m[1]})",source)
    assert n==3
    lhs=f"s->delay24.{ch}[at]"
    line=f"{lhs}=fmaf(s->delay24.feedback,{ch},{name}[i]);"
    assert source.count(line)==1
    source=source.replace(line, f"put_at(s->delay24.{ch},at,fmaf(s->delay24.feedback,{ch},{name}[i]));")
main=r"""
enum {CAP=66151};
static cell a[CELLS(CAP)],b[CELLS(CAP)];
static void require(int ok){if(!ok)abort();}
static float block(juno6_t *s,float l,float expected,int stream){
 float r=0; juno6_delay24_process(s,&l,&r,1); require(r==0.f);
 if (stream) require(fwrite(&l,sizeof l,1,stdout)==1);
 else require(l==expected);
 return l;
}
int main(void){
 for(unsigned scenario=0;scenario<2;scenario++){
  memset(a,0,sizeof a);memset(b,0,sizeof b);
  juno6_t s={.typed=1,.sample_rate=44100.f};
  require(juno6_delay24_arm(&s,a,b,CAP,0,10000));
  require(juno6_delay24_set(&s,SJ6_DELAY25,scenario?1:127));
  require(juno6_delay24_set(&s,SJ6_DELAY26,103));
  require(juno6_delay24_set(&s,SJ6_DELAY27,127));
  for(unsigned i=0;i<10008;i++)block(&s,0,0,0);
  block(&s,.319117f,.319117f,0); /* verify actual dry equality with history running */
  require(juno6_delay24_set(&s,SJ6_DELAY24,1));
  if (!scenario) {
    /* Wet echo of an impulse injected while bypassed, no new note needed. */
    for(unsigned i=0;i<14*66150;i++)block(&s,0,0,1);
  }else{
    for(unsigned i=0;i<10008;i++)block(&s,0,0,1);
    for(unsigned i=0;i<12000;i++)block(&s,i?0:.319117f,0,1);
  }
  require(juno6_delay24_set(&s,SJ6_DELAY24,0));
  for(unsigned i=0;i<10008;i++)block(&s,0,0,1);
  block(&s,-.319117f,-.319117f,0);
 }
}
"""
edge=14*66150+10008
with tempfile.TemporaryDirectory() as td:
    t=pathlib.Path(td)
    (t/"w.c").write_text(prefix+source+main)
    for i, name in enumerate(("f32","s8 linear","s8 mu-law","s16 linear","s4 mu-law")):
        subprocess.run([os.getenv("CC","cc"),"-std=c99","-O2",f"-DCODEC={i}",str(t/"w.c"),"-lm","-o",str(t/"w")],check=True)
        x=array.array("f")
        x.frombytes(subprocess.check_output([str(t/"w")]))
        if i==0: ref=x
        scores=[]
        for lo,hi in ((0,edge),(edge,len(ref))):
            signal=sum(v*v for v in ref[lo:hi])
            err=sum((a-b)**2 for a,b in zip(ref[lo:hi],x[lo:hi]))
            scores.append(math.inf if not err else 10*math.log10(signal/err))
        bytes_per={"f32":4,"s16 linear":2}.get(name,1)
        buffer_bytes=66152 if i==4 else 2*66151*bytes_per
        print(f"{name:11s}: {buffer_bytes:6d} B mono-owner stereo; 1.5s {scores[0]:6.1f} dB, fractional {scores[1]:6.1f} dB SNR; dry/right exact")
