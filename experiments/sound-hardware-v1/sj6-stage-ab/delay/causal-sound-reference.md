# Causal SuperJ6 Delay sound reference before firmware fit

This C candidate stores signed 7-bit mantissas with one independent scale byte per 32 half-rate samples per channel. A fixed 47-tap half-band FIR limits aliasing and reconstructs at full rate. Mature recorded tiles are read at an absolute synthetic tap, compensating the two FIR group delays (46 frames) before the normal feedback recurrence. The stereo channels do not share stored audio or gains. This reference is not the tiny firmware branch: it intentionally allocates the measured history before considering performance and storage. No live Sync edit, short sub-111-frame tap, pingpong, native timer or full preset assumption.

Actual source-linked causal C compared to the preserved full-rate Build9 behavior port: **240 stereo cues, 720 signed repeat windows**, 44.1/48 kHz, 0.25/1/3 seconds, 1/3/6/9 kHz and independent dense chords, .001 to .35 input level, .499/.826 feedback, repeats 1/2/4. Worst residual −31.154 dBr, max absolute level error 0.06588 dB; all pass the declared -30 dBr / .5 dB investigation threshold. Whole renders and irregular chunk renders are byte-identical across the full matrix. This is not a perceptual or production release threshold.

The packed ring at 44.1 kHz needs **10,150 / 40,136 / 120,060 B per stereo owner** at .25/1/3 seconds, plus filter/codec state. At 48 kHz the 3-second ring needs 130,616 B. Reference 8-bit offline block64 improves worst signed residual to −36.649 dBr but requires more history; 6-bit offline loses about 5 dB of residual margin and fails. Explicit scale is necessary: fixed μ-law looks good at moderate levels but collapses signed quiet repeats. The tiny native history sampled too slowly (3 kHz folds near 244 Hz); an algorithm must fix this error mechanism rather than only restoring level or onset.

This program is a host reference with a constant tap, allocated heap, per-channel loop and full FIR; porting should be a separate step after sound acceptance. Neither the ring size nor CPU is funded in the firmware. Keep the existing small branch unmodified.

Reproduce: put the audio behavioral C reference below at ../refine/sj6_stereo_delay.c, build it as ../refine/ref.so using -O3 -shared -fPIC -lm. Compile causal.c with the included halfband.inc and -O3 -std=c11 -fPIC -shared -ffp-contract=off -lm to causal.so; run verify-causal.py next to it, for full or --quick comparisons. No original binary or audio is included. The offline verify.py additionally reproduces the two adjacent precision screens (flags --bits 8 --block 64, or --bits 6 --block 32 --quick).

### causal.c

```c
/* Offline causal proof rig. Actual signed 7-bit / 8-bit mantissa ring with
 * one independent exponent per tile/channel. No DnBOX wiring or fit claim. */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "halfband.inc"
#define WIDTH 7u
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
 return ((float)q*(1.f/63.f))*scale[b[0]];
}
static void pack(uint8_t *b,const float *v,const float *scale){
 float peak=0.f;
 for(unsigned i=0;i<TILE;i++)if(fabsf(v[i])>peak)peak=fabsf(v[i]);
 int e=(int)ceilf(12.f*log2f(fmaxf(peak,0x1p-20f)))+240;
 if(e<0)e=0;if(e>255)e=255;
 b[0]=(uint8_t)e;
 memset(b+1,0,STRIDE-1u);
 for(unsigned i=0;i<TILE;i++){
  long q=lrintf(v[i]*(63.f/scale[e]));
  if(q>63)q=63;if(q< -63)q= -63;
  const unsigned z=(unsigned)q&127u, bit=i*WIDTH, off=(bit>>3), shift=bit&7u;
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

### halfband.inc

```c
static const float h[47]={-0x1.afb258927df8dp-13f,0x0.0p+0f,0x1.758607fa625cep-11f,0x0.0p+0f,-0x1.b4512681494a4p-10f,0x0.0p+0f,0x1.ab95363893754p-9f,0x0.0p+0f,-0x1.795d60803f779p-8f,0x0.0p+0f,0x1.36ca25afee8e2p-7f,0x0.0p+0f,-0x1.e91432f3f5edfp-7f,0x0.0p+0f,0x1.773a5fc3946a6p-6f,0x0.0p+0f,-0x1.1f84132885eccp-5f,0x0.0p+0f,0x1.c99dffa901fd3p-5f,0x0.0p+0f,-0x1.9ec4a97f4b1c2p-4f,0x0.0p+0f,0x1.4447aca34a6f9p-2f,0x1.0002ba1d86088p-1f,0x1.4447aca34a6f9p-2f,0x0.0p+0f,-0x1.9ec4a97f4b1c2p-4f,0x0.0p+0f,0x1.c99dffa901fd3p-5f,0x0.0p+0f,-0x1.1f84132885eccp-5f,0x0.0p+0f,0x1.773a5fc3946a6p-6f,0x0.0p+0f,-0x1.e91432f3f5edfp-7f,0x0.0p+0f,0x1.36ca25afee8e2p-7f,0x0.0p+0f,-0x1.795d60803f779p-8f,0x0.0p+0f,0x1.ab95363893754p-9f,0x0.0p+0f,-0x1.b4512681494a4p-10f,0x0.0p+0f,0x1.758607fa625cep-11f,0x0.0p+0f,-0x1.afb258927df8dp-13f};
```

### verify-causal.py

```python
"""Offline fidelity screen. No causal codec, firmware or product fit claimed."""
from pathlib import Path
import ctypes as C, numpy as np, json
from scipy.signal import firwin,resample_poly
P=Path(__file__).resolve().parent
lib=C.CDLL(str(P.parent/'refine/ref.so'))
fp=np.ctypeslib.ndpointer(dtype=np.float32,ndim=1,flags='C_CONTIGUOUS')
lib.sj6_new.argtypes=[C.c_double];lib.sj6_new.restype=C.c_void_p
lib.sj6_params.argtypes=[C.c_void_p,C.c_double,C.c_float,C.c_float,C.c_int]
lib.sj6_process.argtypes=[C.c_void_p,C.c_size_t,fp,fp,fp,fp]
lib.sj6_free.argtypes=[C.c_void_p]
native=C.CDLL(str(P/'causal.so')).delay_causal
native.argtypes=[C.c_uint,C.c_float,C.c_uint,fp,fp,fp,fp]
native.restype=C.c_uint
segmented=C.CDLL(str(P/'causal.so')).delay_causal_segmented
segmented.argtypes=native.argtypes
segmented.restype=C.c_uint
def candidate(x,rate,N,bits,block):
    h=firwin(47,rate/4,fs=rate,window=('kaiser',6))
    lo=resample_poly(x,1,2,axis=1,window=h)
    padded=np.pad(lo,((0,0),(0,(-lo.shape[1])%block))).reshape(2,-1,block)
    exp=np.ceil(12*np.log2(np.maximum(np.max(np.abs(padded),axis=2),2**-20))).clip(-240,15)
    scale=(2**(exp/12))[:,:,None]
    maxval=(1<<(bits-1))-1
    q=np.round(np.clip(padded/scale,-1.,1.)*maxval)
    u=((q/maxval)*scale).reshape(2,-1)[:,:lo.shape[1]]
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
    pred=np.empty_like(x)
    assert native(wait,fb,length,*x,*pred)==2*((wait//2+64+31)//32)*29
    cut=np.empty_like(x)
    assert segmented(wait,fb,length,*x,*cut)==2*((wait//2+64+31)//32)*29
    assert cut.tobytes()==pred.tobytes()
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

### verify.py

```python
"""Offline fidelity screen. No causal codec, firmware or product fit claimed."""
from pathlib import Path
import ctypes as C, numpy as np, json
from scipy.signal import firwin,resample_poly
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
    exp=np.ceil(12*np.log2(np.maximum(np.max(np.abs(padded),axis=2),2**-20))).clip(-240,15)
    scale=(2**(exp/12))[:,:,None]
    maxval=(1<<(bits-1))-1
    q=np.round(np.clip(padded/scale,-1.,1.)*maxval)
    u=((q/maxval)*scale).reshape(2,-1)[:,:lo.shape[1]]
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

### sj6_stereo_delay.c

```c
/* Host-only behavioral port of the Build9 StereoDelay core. */
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct sj6_delay {
    float *ring;
    unsigned capacity, write;
    float read, feedback, dry, right_last, right_send;
    int ping;
    double rate;
};

void *sj6_new(double rate) {
    if (!(rate >= 8000 && rate <= 384000)) return NULL;
    struct sj6_delay *x = calloc(1, sizeof(*x));
    if (!x) return NULL;
    x->capacity=(unsigned)(rate*3.0);
    x->ring=calloc(2*x->capacity,sizeof(float));
    if (!x->ring) { free(x); return NULL; }
    x->rate=rate;
    return x;
}

void sj6_free(void *ptr) {
    struct sj6_delay *x=ptr;
    if(x){free(x->ring);free(x);}
}

void sj6_clear(void *ptr) {
    struct sj6_delay *x=ptr;
    memset(x->ring,0,2*x->capacity*sizeof(float));
    x->right_last=0;
}

void sj6_params(void *ptr, double sec, float feedback, float dry, int ping) {
    struct sj6_delay *x=ptr;
    /* The AU wrapper accepts seconds; its inner core accepts milliseconds. */
    const double ms=(double)(float)sec*1000.0;
    float d=(float)(x->rate*fmin(3000.,fmax(0.,ms))/1000.);
    float cap=(float)x->capacity;
    x->read=(float)x->write-d;
    if(x->read<0.f) x->read+=cap;
    if(x->read>=cap) x->read-=cap;
    x->feedback=fminf(1.f,fmaxf(0.f,feedback));
    x->dry=fminf(1.f,fmaxf(0.f,dry));
    x->ping=ping;
    x->right_send=ping?powf(x->feedback,.25f):0.f;
}

void sj6_process(void *ptr, size_t n, const float *l, const float *r,
                 float *ol, float *orr) {
    struct sj6_delay *x=ptr;
    float *hl=x->ring,*hr=hl+x->capacity;
    unsigned pos=x->write;
    float read=x->read,last=x->right_last;
    const float wet=1.f-x->dry;
    while(n--){
        unsigned a=(unsigned)read,b=a+1==x->capacity?0:a+1;
        float f=read-(float)a;
        float dl=fmaf(1.f-f,hl[a],f*hl[b]);
        float dr=fmaf(1.f-f,hr[a],f*hr[b]);
        float inl=*l++,inr=*r++;
        if(x->ping){
            hl[pos]=fmaf(x->feedback,last,(inl+inr)*.5f);
            hr[pos]=dl*x->right_send;
        }else{
            hl[pos]=fmaf(x->feedback,dl,inl);
            hr[pos]=fmaf(x->feedback,dr,inr);
        }
        last=dr;
        *ol++=fmaf(wet,dl,x->dry*inl);
        *orr++=fmaf(wet,dr,x->dry*inr);
        if(++pos==x->capacity) pos=0;
        read+=1.f;
        if(read>=(float)x->capacity) read-=(float)x->capacity;
    }
    x->write=pos;x->read=read;x->right_last=last;
}
```

```json
{
  "causal-stream-full.json": {
    "bits": 7,
    "block": 32,
    "cases": 240,
    "windows": 720,
    "worst": {
      "rate": 48000,
      "tap": 1.0,
      "cue": "dense",
      "amplitude": 0.35,
      "feedback": 0.826,
      "worst_residual_dBr": -31.15424156188965,
      "worst_absolute_level_dB": 0.002942697610706091
    },
    "max_abs_level_dB": 0.06587783992290497,
    "pass_screen": true
  },
  "format6-quick.json": {
    "bits": 6,
    "block": 32,
    "cases": 60,
    "windows": 180,
    "worst": {
      "rate": 44100,
      "tap": 1.0,
      "cue": "dense",
      "amplitude": 0.35,
      "feedback": 0.826,
      "worst_residual_dBr": -26.33942717197877,
      "worst_absolute_level_dB": 0.2551016230076466
    },
    "max_abs_level_dB": 0.34754444713768934,
    "pass_screen": false
  },
  "format8-full.json": {
    "bits": 8,
    "block": 64,
    "cases": 240,
    "windows": 720,
    "worst": {
      "rate": 48000,
      "tap": 1.0,
      "cue": "dense",
      "amplitude": 0.35,
      "feedback": 0.826,
      "worst_residual_dBr": -36.649220241951255,
      "worst_absolute_level_dB": 0.006984859844449001
    },
    "max_abs_level_dB": 0.06251017201776235,
    "pass_screen": true
  }
}
```
