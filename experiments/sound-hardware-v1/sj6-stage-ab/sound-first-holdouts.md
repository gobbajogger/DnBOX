# Follow-up sound-first evidence: authored Delay holdout and full Sub key range

## Authored SuperJ6 material through the actual causal Delay C reference

Three original signed Brainwave output arms (both Main arms independently and combined) serve purely as realistic effect *input*. Each has a bounded signed held window and the whole saved phrase. Test at .25 / 1 / 3s at 44.1 kHz and two feedback gains, 36 isolated effect comparisons total. Same source, no gain/time fitting. Whole and irregularly segmented C renders remain byte-identical. Across all arms: worst full −37.683 dBr, worst late −32.956 dBr, absolute level deviations still below the earlier .5 dB limit. This validates a realistic stem beyond osc test tones, not whole-instrument parity or firmware fit. No signed AU renders are in this file.

At smaller sizes, ranking sparse DCT coefficients degraded the actual synthetic signed waveform. 128-frame tiles at 16/24/40 stored coefficients gave −3.76/−6.39/−10.71 dBr worst (pass criterion −30). 512-frame tiles at 64/96/160 gave approximately −7.4…−7.6 dBr and late level errors >3.5 dB, so extra lookback by itself does not solve transient/block boundary distortion. The exact C six-bit per-tile error-feedback candidates also fail: filter-shaped −25.663 dBr and forced high-band shift −23.266 dBr. This is not a universal information-theoretic lower bound. Use the seven-bit causal C reference and its saved scenarios as the comparison baseline, not stock wet mix or gain correction.

## Authentic Sub tuning: every MIDI key on native isolated compact core

Run all 128 note inputs with and without the pinned authentic profile, no edits to preset content. Native first-phase difference has the correct sign on every 95 authored rows; all 33 missing rows are bit-identical. Maximum observed cents error versus independent exp2 ratio of the original tuning values ≈0.0004265 cents, zero phase mismatches. This is the native PI32 core playing an authentic profile inside a one-task emulator program, not an installed production storage profile or whole-instrument pass. Reproduce the private table and startup payload as in sub-profile-native-active.md. Profile source/values and PCM remain local.


### heldout.py

```python
from pathlib import Path
import io,sys,tarfile,json,hashlib
import numpy as np
P=Path(__file__).resolve().parent
b=P/'verify-causal.py';exec(compile(b.read_text().split('def main():')[0],str(b),'exec'),globals())
base=P.parents[1]/'sj6-mixer-next/private/SJ6-Brainwave-Direct-AUv3-Fanin.tgz'
assert hashlib.sha256(base.read_bytes()).hexdigest()=='d3821515187f4c086cde7ecbff57bafbae216fbd86becd1ff356fb1fe4a5df9f'
rows=[]
with tarfile.open(base) as t:
 for v in ('all','osc1-only','osc2-only'):
    p='native-warm-brainwave-fanin-authored-'+v
    m=json.load(t.extractfile(p+'/capture.json'))
    src=np.load(io.BytesIO(t.extractfile(p+'/output.npy').read()),allow_pickle=False).astype('f4')
    assert m['rate']==44100 and hashlib.sha256(src.tobytes()).hexdigest()==m['pcm_sha256']
    for caption,audio in [
      ('held',src[:,24576:28672]*np.hanning(4096)[None,:]),
      ('phrase',src)]:
     for sec in (.25,1,3):
      tap=round(44100*sec)
      length=audio.shape[1]+4*tap+128
      for fb in (.499,.826):
       x=np.zeros((2,length),'f4');x[:,:audio.shape[1]]=audio
       d=lib.sj6_new(44100);ref=np.empty_like(x);lib.sj6_params(d,sec,fb,0.,0);lib.sj6_process(d,length,*x,*ref);lib.sj6_free(d)
       q=np.empty_like(x);mem=native(tap,fb,length,*x,*q)
       s=np.empty_like(x);assert segmented(tap,fb,length,*x,*s)==mem and s.tobytes()==q.tobytes()
       e,g=score(q.astype('f8'),ref.astype('f8'))
       off=tap*3+audio.shape[1]
       el,gl=score(q[:,off:].astype('f8'),ref[:,off:].astype('f8'))
       rows.append(dict(arm=v,cue=caption,delay=sec,feedback=fb,mem=mem,err=float(e.max()),
            level=float(abs(g).max()),tail_err=float(el.max()),tail_level=float(abs(gl).max())))
print(json.dumps(dict(inputs=len(rows),worst_full=max(rows,key=lambda z:z['err']),
           worst_tail=max(rows,key=lambda z:z['tail_err']),rows=rows),indent=2))
```

### verify-dct.py

```python
"""Offline fidelity screen. No causal codec, firmware or product fit claimed."""
from pathlib import Path
import ctypes as C, numpy as np, json
from scipy.signal import firwin,resample_poly
from scipy.fft import dct,idct
P=Path(__file__).resolve().parent
lib=C.CDLL(str(P.parent/'refine/ref.so'))
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
lib.sj6_new.argtypes=[C.c_double];lib.sj6_new.restype=C.c_void_p
lib.sj6_params.argtypes=[C.c_void_p,C.c_double,C.c_float,C.c_float,C.c_int]
lib.sj6_process.argtypes=[C.c_void_p,C.c_size_t,fp,fp,fp,fp]
lib.sj6_free.argtypes=[C.c_void_p]
def candidate(x,rate,N,bits,block):
    h=firwin(47,rate/4,fs=rate,window=('kaiser',6))
    lo=resample_poly(x,1,2,axis=1,window=h)
    padded=np.pad(lo,((0,0),(0,(-lo.shape[1])%block))).reshape(2,-1,block)
    f=dct(padded,axis=2,norm='ortho')
    keep=bits
    cut=np.argpartition(np.abs(f),-keep,axis=2)[:,:,-keep:]
    select=np.take_along_axis(f,cut,axis=2)
    exp=np.ceil(12*np.log2(np.maximum(np.max(np.abs(select),axis=2),2**-20))).clip(-240,15)
    scale=(2**(exp/12))[:,:,None]
    symbols=np.round(np.clip(select/scale,-1.,1.)*127)
    coeff=np.zeros_like(f)
    np.put_along_axis(coeff,cut,symbols*(scale/127.),axis=2)
    u=idct(coeff,axis=2,norm='ortho').reshape(2,-1)[:,:lo.shape[1]]
    return resample_poly(u,2,1,axis=1,window=h)[:,:N]
def cue(rate,name,amp):
    n=round(rate*.098)
    t=np.arange(n)/rate
    if isinstance(name,int):
        l=np.sin(2*np.pi*name*t+.391)
        r=.66*np.sin(2*np.pi*(name*.87)*t+.653)
    else:
        f=np.array([199,401,691,1087,2303,3301,5119,7141,9017],dtype=float)
        l=(np.sin(2*np.pi*f[:,None]*t+np.arange(len(f))[:,None]*.477).sum(axis=0))/4.5
        r=.62*(np.cos(2*np.pi*(f*.979)[:,None]*t-np.arange(len(f))[:,None]*.843).sum(axis=0))/4.5
    return (amp*np.hanning(n)*np.stack((l,r))).astype('f4')
def score(x,y):
    d=x-y
    den=np.linalg.norm(y,axis=1)
    return (20*np.log10(np.maximum(np.linalg.norm(d,axis=1)/den,1e-18)),
            20*np.log10(np.linalg.norm(x,axis=1)/den))
def do(rate,sec,hz,amp,fb,bits,block):
    wait=round(rate*sec)
    w=cue(rate,hz,amp)
    length=4*wait+w.shape[1]+64
    x=np.zeros((2,length),'f4');x[:,:w.shape[1]]=w
    ref=np.empty_like(x)
    d=lib.sj6_new(rate); assert d
    lib.sj6_params(d,sec,fb,0.,0);lib.sj6_process(d,length,*x,*ref);lib.sj6_free(d)
    pred=np.zeros_like(x,dtype=float)
    # Four convergent chronological echo updates; no fifth echo in the scored windows.
    for j in range(4):
        sig=candidate(x+float(np.float32(fb))*pred,rate,length,bits,block)
        pred[:,wait:]=sig[:,:-wait]
    res=[];gain=[]
    for i in (1,2,4):
        sl=slice(wait*i-32,wait*i+w.shape[1]+32)
        r,g=score(pred[:,sl],ref[:,sl])
        res.append(float(r.max()));gain.append(float(abs(g).max()))
    return max(res),max(gain)
def main():
 import itertools,argparse
 p=argparse.ArgumentParser();p.add_argument('--bits',type=int,default=7)
 p.add_argument('--block',type=int,default=32);p.add_argument('--quick',action='store_true');z=p.parse_args()
 result=[]
 for rate,sec,hz,amp,fb in itertools.product([44100,48000],[.25,1.,3.],[1000,3000,6000,9000,'dense'],[.001,.02,.12,.35],[.499,.826]):
    if z.quick and (rate==48000 or amp not in (.001,.35)):continue
    r,g=do(rate,sec,hz,amp,fb,z.bits,z.block)
    result.append(dict(rate=rate,tap=sec,cue=hz,amplitude=amp,feedback=fb,worst_residual_dBr=r,worst_absolute_level_dB=g))
 worst=max(result,key=lambda r:r['worst_residual_dBr'])
 print(json.dumps(dict(bits=z.bits,block=z.block,cases=len(result),windows=3*len(result),worst=worst,
                       max_abs_level_dB=max(r['worst_absolute_level_dB'] for r in result),
                       pass_screen=all(r['worst_residual_dBr']<=-30 and r['worst_absolute_level_dB']<=.5 for r in result),
                       rows=result),indent=2))
if __name__=='__main__':main()
```

### shape-filter.c

```c
/* Offline causal proof rig. Actual signed 7-bit / 8-bit mantissa ring with
 * one independent exponent per tile/channel. No DnBOX wiring or fit claim. */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "halfband.inc"
static const float ntf[8]={-0x1.6427d392076b3p-5f,0x1.5adcbbcfd9f3ap-5f,-0x1.491146f0ecb43p-5f,0x1.2e484cc42f8a9p-5f,-0x1.0e64d651e0548p-5f,0x1.d37f246520b72p-6f,-0x1.89398531ddbd2p-6f,0x1.3e7d1aeb51c09p-6f};

#define WIDTH 6u
#define MAXQ 31
#define TILE 32u
#define STRIDE ((TILE*WIDTH+7)/8+1u)
static float get(const uint8_t *ring,const float *scale,unsigned blocks,int t){
 if(t<0)return 0.f;
 unsigned n=(unsigned)t, slot=(n/TILE)%blocks, bit=(n%TILE)*WIDTH;
 const uint8_t *b=ring+STRIDE*slot;
 const unsigned off=(bit>>3), shift=bit&7u;
 unsigned v=(unsigned)b[off+1]>>shift;
 if(shift+WIDTH>8)v|=(unsigned)b[off+2]<<(8-shift);
 v&=(1u<<WIDTH)-1u;
 const int q=(int)v-(v>=(1u<<(WIDTH-1u))?(1<<WIDTH):0);
 return ((float)q*(1.f/31.f))*scale[b[0]];
}
static void pack(uint8_t *b,const float *v,const float *scale){
 float peak=0.f;
 for(unsigned i=0;i<TILE;i++)if(fabsf(v[i])>peak)peak=fabsf(v[i]);
 int e=(int)ceilf(12.f*log2f(fmaxf(peak,0x1p-20f)))+240;
 if(e<0)e=0;if(e>255)e=255;
 b[0]=(uint8_t)e;
 memset(b+1,0,STRIDE-1u);
 float err[8]={0};
 for(unsigned i=0;i<TILE;i++){
  float x=v[i];
  for(unsigned j=0;j<8;j++)x+=ntf[j]*err[j];
  long q=lrintf(x*(31.f/scale[e]));
  if(q>MAXQ)q=MAXQ;if(q< -MAXQ)q= -MAXQ;
  for(unsigned j=7;j>0;j--)err[j]=err[j-1];
  const float qe=x-((float)q*(1.f/31.f))*scale[e];
  const float clip=scale[e]*(.5f/31.f);
  err[0]=qe>clip?clip:qe< -clip? -clip:qe;
  const unsigned z=(unsigned)q&63u, bit=i*WIDTH, off=(bit>>3), shift=bit&7u;
  b[off+1]|=(uint8_t)(z<<shift);
  if(shift+WIDTH>8)b[off+2]|=(uint8_t)(z>>(8-shift));
 }
}
typedef struct { float writes[47],lo[TILE]; unsigned position; } owner;
static void process(owner *s,unsigned delay,float feedback,unsigned n,const float *in,float *out,
                    uint8_t *ring,const float *scale,unsigned blocks)
{
 for(unsigned j=0;j<n;j++){
   unsigned i=s->position++;
   const int t=(int)i-(int)delay+46; const float dry=in[j];
   float echo=0.f;
   if(t>=0){
    for(unsigned k=(unsigned)t&1u;k<47;k+=2)
     if(h[k]!=0.f)echo+=2.f*h[k]*get(ring,scale,blocks,(t-(int)k)/2);
   }
   out[j]=echo;
   unsigned w=i%47u;
   s->writes[w]=fmaf(feedback,echo,dry);
   if(!(i&1u)){
    float x=0.f;
    for(unsigned k=0;k<47;k++)if(h[k]!=0.f)x+=h[k]*s->writes[(w+47u-k)%47u];
    const unsigned m=i/2u;
    s->lo[m%TILE]=x;
    if(m%TILE==TILE-1u)pack(ring+STRIDE*((m/TILE)%blocks),s->lo,scale);
   }
 }
}
static unsigned run(unsigned delay,float feedback,unsigned n,const float *in_l,const float *in_r,
                    float *out_l,float *out_r,int segmented)
{
 if(delay<111u)return 0;
 const unsigned blocks=(delay/2u+64u+TILE-1u)/TILE;
 uint8_t *mem=calloc(2u*blocks,STRIDE);if(!mem)return 0;
 float scale[256];for(int i=0;i<256;i++)scale[i]=exp2f((i-240)/12.f);
 const float *in[2]={in_l,in_r};float *out[2]={out_l,out_r};
 for(unsigned c=0;c<2;c++){
  owner a={0};
  uint8_t *ring=mem+c*blocks*STRIDE;
  if(!segmented)process(&a,delay,feedback,n,in[c],out[c],ring,scale,blocks);
  else {
   unsigned at=0,j=0;static const unsigned widths[]={1,3,31,128,16,255,7,32,81};
   while(at<n){
    unsigned count=widths[j++%9];if(count>n-at)count=n-at;
    process(&a,delay,feedback,count,in[c]+at,out[c]+at,ring,scale,blocks);
    at+=count;
   }
  }
 }
 free(mem);return 2u*blocks*STRIDE;
}
unsigned delay_causal(unsigned d,float f,unsigned n,const float*l,const float*r,float*a,float*b)
{return run(d,f,n,l,r,a,b,0);}
unsigned delay_causal_segmented(unsigned d,float f,unsigned n,const float*l,const float*r,float*a,float*b)
{return run(d,f,n,l,r,a,b,1);}
```

### shape-highshift.c

```c
/* Offline causal proof rig. Actual signed 7-bit / 8-bit mantissa ring with
 * one independent exponent per tile/channel. No DnBOX wiring or fit claim. */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "halfband.inc"
static const float ntf[8]={-0x1.0a09457eca820p-1f,0x1.24674df9dd9b8p-1f,-0x1.231b869f0f377p-1f,0x1.06c2c63174df3p-1f,-0x1.aadf243619865p-2f,0x1.317653c6d6edep-2f,-0x1.6ef3fdac1317dp-3f,0x1.408a2fad496c4p-4f};

#define WIDTH 6u
#define MAXQ 31
#define TILE 32u
#define STRIDE ((TILE*WIDTH+7)/8+1u)
static float get(const uint8_t *ring,const float *scale,unsigned blocks,int t){
 if(t<0)return 0.f;
 unsigned n=(unsigned)t, slot=(n/TILE)%blocks, bit=(n%TILE)*WIDTH;
 const uint8_t *b=ring+STRIDE*slot;
 const unsigned off=(bit>>3), shift=bit&7u;
 unsigned v=(unsigned)b[off+1]>>shift;
 if(shift+WIDTH>8)v|=(unsigned)b[off+2]<<(8-shift);
 v&=(1u<<WIDTH)-1u;
 const int q=(int)v-(v>=(1u<<(WIDTH-1u))?(1<<WIDTH):0);
 return ((float)q*(1.f/31.f))*scale[b[0]];
}
static void pack(uint8_t *b,const float *v,const float *scale){
 float peak=0.f;
 for(unsigned i=0;i<TILE;i++)if(fabsf(v[i])>peak)peak=fabsf(v[i]);
 int e=(int)ceilf(12.f*log2f(fmaxf(peak,0x1p-20f)))+240;
 if(e<0)e=0;if(e>255)e=255;
 b[0]=(uint8_t)e;
 memset(b+1,0,STRIDE-1u);
 float err[8]={0};
 for(unsigned i=0;i<TILE;i++){
  float x=v[i];
  for(unsigned j=0;j<8;j++)x+=ntf[j]*err[j];
  long q=lrintf(x*(31.f/scale[e]));
  if(q>MAXQ)q=MAXQ;if(q< -MAXQ)q= -MAXQ;
  for(unsigned j=7;j>0;j--)err[j]=err[j-1];
  const float qe=x-((float)q*(1.f/31.f))*scale[e];
  const float clip=scale[e]*(.5f/31.f);
  err[0]=qe>clip?clip:qe< -clip? -clip:qe;
  const unsigned z=(unsigned)q&63u, bit=i*WIDTH, off=(bit>>3), shift=bit&7u;
  b[off+1]|=(uint8_t)(z<<shift);
  if(shift+WIDTH>8)b[off+2]|=(uint8_t)(z>>(8-shift));
 }
}
typedef struct { float writes[47],lo[TILE]; unsigned position; } owner;
static void process(owner *s,unsigned delay,float feedback,unsigned n,const float *in,float *out,
                    uint8_t *ring,const float *scale,unsigned blocks)
{
 for(unsigned j=0;j<n;j++){
   unsigned i=s->position++;
   const int t=(int)i-(int)delay+46; const float dry=in[j];
   float echo=0.f;
   if(t>=0){
    for(unsigned k=(unsigned)t&1u;k<47;k+=2)
     if(h[k]!=0.f)echo+=2.f*h[k]*get(ring,scale,blocks,(t-(int)k)/2);
   }
   out[j]=echo;
   unsigned w=i%47u;
   s->writes[w]=fmaf(feedback,echo,dry);
   if(!(i&1u)){
    float x=0.f;
    for(unsigned k=0;k<47;k++)if(h[k]!=0.f)x+=h[k]*s->writes[(w+47u-k)%47u];
    const unsigned m=i/2u;
    s->lo[m%TILE]=x;
    if(m%TILE==TILE-1u)pack(ring+STRIDE*((m/TILE)%blocks),s->lo,scale);
   }
 }
}
static unsigned run(unsigned delay,float feedback,unsigned n,const float *in_l,const float *in_r,
                    float *out_l,float *out_r,int segmented)
{
 if(delay<111u)return 0;
 const unsigned blocks=(delay/2u+64u+TILE-1u)/TILE;
 uint8_t *mem=calloc(2u*blocks,STRIDE);if(!mem)return 0;
 float scale[256];for(int i=0;i<256;i++)scale[i]=exp2f((i-240)/12.f);
 const float *in[2]={in_l,in_r};float *out[2]={out_l,out_r};
 for(unsigned c=0;c<2;c++){
  owner a={0};
  uint8_t *ring=mem+c*blocks*STRIDE;
  if(!segmented)process(&a,delay,feedback,n,in[c],out[c],ring,scale,blocks);
  else {
   unsigned at=0,j=0;static const unsigned widths[]={1,3,31,128,16,255,7,32,81};
   while(at<n){
    unsigned count=widths[j++%9];if(count>n-at)count=n-at;
    process(&a,delay,feedback,count,in[c]+at,out[c]+at,ring,scale,blocks);
    at+=count;
   }
  }
 }
 free(mem);return 2u*blocks*STRIDE;
}
unsigned delay_causal(unsigned d,float f,unsigned n,const float*l,const float*r,float*a,float*b)
{return run(d,f,n,l,r,a,b,0);}
unsigned delay_causal_segmented(unsigned d,float f,unsigned n,const float*l,const float*r,float*a,float*b)
{return run(d,f,n,l,r,a,b,1);}
```

### sweep.c

```c
#include <stdint.h>
#include "dnbx_core.c"
#include "profile.private.h"
#include "fm1_time.h"

static dnbx_part s;
static float l[64],r[64];
static unsigned sweep_only;
static unsigned scenario(unsigned note,unsigned physical,float range,volatile float *res)
{
    if(dnbx_init_superj6(&s,44100.f)!=DNBX_OK)return 1;
    const uint8_t codes[2]={0,127};
    dnbx_set_sj6_sources(&s,0,codes);
    if(dnbx_store_raw(&s,86,range)!=DNBX_RAW_STORED_ONLY)return 1;
    int v=dnbx_note_on(&s,note,86);
    if(v<0||v>=DNBX_VOICES)return 1;
    float before=s.voice[v].sj6_sub_phase;
    if(dnbx_render_pitch_osc_levels_velocity(&s,l,r,1,1.f,.5f,.3f,0.f)!=DNBX_OK)return 1;
    float after=s.voice[v].sj6_sub_phase;
    float delta=after-before;if(delta<0.f)delta+=1.f;
    int mc=sj6_profile?sj6_sub_tune_mc(sj6_profile,physical):0;
    float expect=.5f*pitch_increment(&s,physical,(float)mc*.00001f);
    float exp_phase=before+expect;if(exp_phase>=1.f)exp_phase-=1.f;
    res[0]=delta*44100.f;res[1]=expect*44100.f;
    if(sweep_only){res[2]=0.f;return after!=exp_phase;}
    float peak=0.f;
    for(unsigned i=0;i<16;++i){
      if(dnbx_render_pitch_osc_levels_velocity(&s,l,r,64,1.f,.5f,.3f,0.f)!=DNBX_OK)return 1;
      for(unsigned j=0;j<64;j++)if(fm_fabsf(l[j])>peak)peak=fm_fabsf(l[j]);
    }
    res[2]=peak;
    return after!=exp_phase||peak==0.f;
}
void bench_start(void){
    volatile uint32_t *out=(void*)0x01c10000;
    volatile float *m=(volatile float*)(out+4);
    unsigned fail=0;out[0]=0;fm1_time_init();
    const unsigned k[]={32,36,55}; uint32_t t=fm1_ticks();
    /* Isolated program reset; production API still has no reset or rebind path. */
    sj6_profile=0;sj6_profile_closed=0;
    for(unsigned i=0;i<3;++i)fail+=scenario(k[i],k[i],.5f,m+3*i);
    sj6_profile=0;sj6_profile_closed=0;
    fail+=!dnbx_sj6_install_sub_tune_profile(&original_profile,sizeof original_profile);
    for(unsigned i=0;i<3;++i)fail+=scenario(k[i],k[i],.5f,m+9+3*i);
    fail+=scenario(48,36,0.f,m+18);
    fail+=dnbx_sj6_install_sub_tune_profile(&original_profile,sizeof original_profile);
    sweep_only=1;
    for(unsigned k=0;k<128;k++)fail+=scenario(k,k,.5f,m+21+3*k);
    sj6_profile=0;sj6_profile_closed=0;
    for(unsigned k=0;k<128;k++)fail+=scenario(k,k,.5f,m+405+3*k);
    out[1]=fail;out[2]=fm1_ticks()-t;out[3]=sizeof original_profile;out[0]=0x534a3654;
}
void *memset(void *p,int v,size_t n){volatile unsigned char *d=p;while(n--)*d++=(unsigned char)v;return p;}
void *memcpy(void *p,const void *s,size_t n){volatile unsigned char *d=p;const unsigned char *a=s;while(n--)*d++=*a++;return p;}
```

### run-sweep.sh

```bash
#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd); R=$(cd "$D/../.."&&pwd)
T="$R/prereqs/jieli-linux-toolchains-20260730.1/pi32v2"; P="$R/sj6-sub-native-next/shared-inlet/tree/firmware/src/dsp"
"$T/bin/clang" -target pi32v2 -mllvm -fast-isel=false -mcpu=r3 -mfprev1 -Os -fno-builtin -ffp-contract=off -ffunction-sections -fdata-sections -idirafter "$T/include" -I"$P" -I"$P/.." -I"$R/fw/firmware/hal" -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_SJ6_NOISE_SUB_NATIVE=1 -DX0X_SJ6_OTA_NATIVE=1 -DX0X_SJ6_SUB_TUNE_PROFILE=1 -DDNBOX_TYPED_CONTROLS_NATIVE_TEST=1 -c "$D/sweep.c" -o "$D/sweep.o"
"$T/bin/ld" --gc-sections --allow-multiple-definition -T "$R/s1-render-next/bench.ld" "$D/"{start,sweep}.o -o "$D/sweep.elf"
"$T/bin/objcopy" -O binary -j .text "$D/sweep.elf" "$D/sweep.bin"
{ dd if=/dev/zero bs=288 count=1 status=none; cat "$D/sweep.bin"; } > "$D/sweep-padded.bin"
X=$(mktemp -d /tmp/dnbox-sub-sweep.XXXXXXXX); mkdir "$X/work"; exec 9>/tmp/dnbox-gate-firmware.lock; flock -x 9; cd "$X"
FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM="$X/ram.bin" "$R/restored-gate/fm1-live-ba83317a" --bin "$D/sweep-padded.bin" --entry 0x02000120 --mhz 360 --shot 1800 >run.log 2>&1
python - "$X" "$R" <<'PY' | tee "$D/sweep-result.json"
import sys,struct,json,math
from pathlib import Path
b=(Path(sys.argv[1])/'ram.bin').read_bytes(); magic,fail,t,sz=struct.unpack_from('<4I',b,0x10000)
assert magic==0x534a3654 and not fail and sz==388,(magic,fail,t,sz)
v=[struct.unpack_from('<3f',b,0x10010+12*i) for i in range(7,263)]
d=(Path(sys.argv[2])/'sj6-modern-private-recovery'/'arm64.private').read_bytes();o=0x7e3c78;n=struct.unpack_from('<Q',d,o+16)[0]
p=dict(struct.unpack_from('<Qd',d,o+32+i*16) for i in range(n))
fallback_equal=0;diff_sign=0;max_cents=0.;errse=[]
for i,(u,z) in enumerate(zip(v[:128],v[128:])):
    if i not in p:
        assert u[0]==z[0]
        fallback_equal+=1
    else:
        assert math.copysign(1,z[0]-u[0])==math.copysign(1,p[i])
        diff_sign+=1
        e=abs(1200*math.log2(u[0]/z[0])+p[i])
        max_cents=max(max_cents,e);errse.append((i,e))
assert max_cents < .08,(max_cents, sorted(errse,key=lambda w:-w[1])[:4])
print(json.dumps(dict(magic=hex(magic),mismatches=fail,profile_bytes=sz,keys=128,missing_keys_exact=fallback_equal,loaded_signs_correct=diff_sign,max_absolute_compared_offset_cents=max_cents,scope='native isolated first increment of each typed Sub voice; independent exp2 ratio from original assigned cents'),indent=2))
PY
```

### sweep-result.json

```json
{
  "magic": "0x534a3654",
  "mismatches": 0,
  "profile_bytes": 388,
  "keys": 128,
  "missing_keys_exact": 33,
  "loaded_signs_correct": 95,
  "max_absolute_compared_offset_cents": 0.00042647624611413804,
  "scope": "native isolated first increment of each typed Sub voice; independent exp2 ratio from original assigned cents"
}
```
