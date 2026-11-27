# SuperJ6 Room, sound-first portable full-rate candidate
This experiment preserves the eight independent 44.1 kHz Soundpipe RevSC line clocks, modulation, feedback, four cubic reads, damping and unmodified output arithmetic. Only persistent ring samples change from binary32 to independently addressed packed floating codes. No shared feedback clocks, no resampling, no coefficient/gain retune, no fit constraints. Against the pinned installed SuperJ6 Build9 signed ARM64 _sp_revsc_compute, even the portable Float32 upstream produces an extremely low nonzero arithmetic residual; that separately measured floor is not presented as bit-exact.

Smallest tested passing width is 17 bits (E5/M11, bias 23), actual 52,484 B packed history/owner with independent four-byte-aligned lines (+440 B unchanged host metadata, total 52,924 B excluding malloc/caller overhead). Original 98,760 B history plus metadata. Full test uses identical signed executable input, no phase/gain fitting, unchanged arrival, provisional worst of raw/early/late/>8k amplitude residual <= -60 dBr, late level within 0.1 dB. This is an engineering comparison threshold for this receipt, not a product gate or listening assertion.

Cue                     17-bit raw / late / >8k residue dBr
Type-II impulse          -76.59  -72.46  -74.16
Type-II dense            -78.92  -76.36  -68.31
Type-II chord / release  -78.12  -76.11  -60.53
Type-I chord / release   -82.62  -80.59  -60.81

18-bit uses 55,568 B history (+440 B host metadata), offers meaningful headroom; worst held-chord HF -66.31 dBr. 16-bit uses 49,392 B: raw and late waveform pass all four but HF residue exceeds the threshold on both chords (~-54.8 dBr). 15-bit misses dense HF and both chords. Exact arrival original and candidates 1981/1944/1945/1945. Late level deviates <0.0001 dB in 17-bit arm. Signed input/reference/output digests plus full numbers in portable-packed.final.jsonl. These are isolated wet leaf stimuli, not complete instrument or actual product-room wrapper qualification. Not a general lower bound or an on-target cycle/RAM fit. Nearest passing measurement only for this codec+stimulus set.

Run replay.sh D exact-revsc.c Soundpipe-header-root under pinned Unicorn 2.1.4/NumPy and project emu/macho PYTHONPATH. Private D/mac-arm64 must be provisioned already. Upstream original C pin at wire.py; metadata defaults and original controls set only by test. C address/word guard tested all active 12–20 widths, random unsigned symbols and actual-length wrap, plus every finite signed E5/B23 nonzero code roundtrip. check.py is separate optional actual installed roundtrip interception: baseline confirms hook no-change signed hash, replacement is external measured source arm, not resident portable C. Its result receipt is not needed for portable replay. No AU, PCM, executable, Git or product output included.

Root reran the complete compiler and reference sweep: all 32 signed reference and candidate output digests matched the worker receipt, with less than 3e-14 dB difference in host metric rounding. No firmware staging was done. Raw imported AU and all audio remain local.

### replay.sh

```bash
#!/bin/bash
# Exact real portable C candidate against authentic source; no firmware or devices.
set -euo pipefail
cd "$(dirname "$0")"
D=$(realpath "${1:?directory containing provisioned, SHA pinned mac-arm64}")
S=$(realpath "${2:?exact pinned upstream revsc.c}")
H=$(realpath "${3:?soundpipe headers root}")
: "${PYTHONPATH:?must include emu/macho bench and Unicorn 2.1.4}"
cc -O3 -ffp-contract=off -shared -fPIC -I"$H" -I"$H/lib/dr_wav" bridge.c "$S" -lm -o "$D/portable-revsc.so"
for m in 6 8 9 10 11 12 14; do
 python wire.py "$S" "$D/p$m.c" "$m"
 cc -O3 -ffp-contract=off -shared -fPIC -I"$H" -I"$H/lib/dr_wav" bridge.c "$D/p$m.c" codec.c -lm -o "$D/p$m.so"
done
python portable-packed.py "$D"
```

### wire.py

```python
"""Isolated upstream Soundpipe resident packed-ring candidate, not product wiring."""
import hashlib, sys
from pathlib import Path
src,dst,mb=sys.argv[1:]
s=Path(src).read_text()
assert hashlib.sha256(s.encode()).hexdigest()== 'f13e4cf1619ccd630848e099517615e4b2942c12ef117f98d70dfda5700cb3ce'
mb=int(mb);assert 5<=mb<=14
marker='static int delay_line_max_samples(SPFLOAT sr, SPFLOAT iPitchMod, int n);'
addition=f'''
enum {{EB=5, MB={mb}, BIAS=23, WIDTH=EB+MB+1}};
extern uint32_t pack_float(float,unsigned,unsigned,unsigned);
extern float unpack_float(uint32_t,unsigned,unsigned,unsigned);
extern void write_code(uint8_t*,unsigned,unsigned,uint32_t);
extern uint32_t read_code(const uint8_t*,unsigned,unsigned);
static SPFLOAT load_tap(sp_revsc_dl *lp, unsigned i)
{{
    return unpack_float(read_code((const uint8_t*)lp->buf, i*WIDTH, WIDTH),EB,MB,BIAS);
}}
static void store_tap(sp_revsc_dl *lp,unsigned i,SPFLOAT x)
{{
    write_code((uint8_t*)lp->buf,i*WIDTH,WIDTH,pack_float(x,EB,MB,BIAS));
}}
'''
assert s.count(marker)==1
s=s.replace(marker,addition+marker)
s=s.replace('delay_line_max_samples(sr, iPitchMod, n) * (int) sizeof(SPFLOAT)','((delay_line_max_samples(sr, iPitchMod, n) * WIDTH + 31) / 32) * 4')
s=s.replace('sizeof(SPFLOAT) * lp->bufferSize','(WIDTH * lp->bufferSize + 7) / 8')
prev='''lp->buf[lp->writePos] = (SPFLOAT) ((n & 1 ? ainR : ainL)
                                 - lp->filterState);'''
assert s.count(prev)==1
s=s.replace(prev,'''store_tap(lp,lp->writePos,(SPFLOAT)((n & 1 ? ainR : ainL) - lp->filterState));''')
import re
s,n=re.subn(r'\(SPFLOAT\) \(lp->buf\[(readPos(?: [+-] [12])?)\]\)',r'load_tap(lp,\1)',s)
s,n2=re.subn(r'\(SPFLOAT\) lp->buf\[readPos\]', 'load_tap(lp,readPos)',s); assert n==4 and n2==4 and 'lp->buf[' not in s
# Each line starts four-byte aligned; the backing bytes are never dereferenced as Float32.
s=s.replace('p->delayLines[i].buf = (p->aux.ptr) + nBytes;', 'p->delayLines[i].buf = (SPFLOAT *)(void *)((uint8_t*)p->aux.ptr + nBytes);')
Path(dst).write_text(s)
```

### codec.c

```c
/* Individually addressed packed storage. No predictive dependency between four cubic taps. */
#include <stdint.h>
#include <string.h>
#include <assert.h>
uint32_t pack_float(float x,unsigned expbits,unsigned mantbits,unsigned bias){
 uint32_t u;memcpy(&u,&x,4);
 unsigned sign=(u>>31)<<(expbits+mantbits),eraw=(u>>23)&255u,max=(1u<<expbits)-1u;
 if(!eraw)return 0;
 int e=(int)eraw-127+(int)bias;
 if(e<=0)return 0;
 unsigned drop=23-mantbits,mag=u&0x7fffffu;
 unsigned m=(mag+((1u<<(drop-1))-1u)+((mag>>drop)&1u))>>drop;
 unsigned em=(unsigned)e*(1u<<mantbits)+m;
 if(em>=(max+1u)*(1u<<mantbits)) em=(max+1u)*(1u<<mantbits)-1u;
 return sign|em;
}
float unpack_float(uint32_t q,unsigned expbits,unsigned mantbits,unsigned bias){
 unsigned e=(q>>mantbits)&((1u<<expbits)-1u);
 if(!e)return 0.f;
 uint32_t u=((q>>(expbits+mantbits))<<31)|((e+127u-bias)<<23)|((q&((1u<<mantbits)-1u))<<(23-mantbits));
 float x;memcpy(&x,&u,4);return x;
}
void write_code(uint8_t *arena,unsigned j,unsigned bits,uint32_t q){
 unsigned j8=j>>3,off=j&7,n=(off+bits+7)/8;
 assert(n<=4&&q<(1u<<bits));
 uint32_t x=0;for(unsigned a=0;a<n;a++)x|=(uint32_t)arena[j8+a]<<(a*8);
 uint32_t mask=((1u<<bits)-1u)<<off;
 x=(x&~mask)|(q<<off);
 for(unsigned a=0;a<n;a++)arena[j8+a]=(uint8_t)(x>>(a*8));
}
uint32_t read_code(const uint8_t *arena,unsigned j,unsigned bits){
 unsigned j8=j>>3,off=j&7,n=(off+bits+7)/8;
 uint32_t x=0;for(unsigned a=0;a<n;a++)x|=(uint32_t)arena[j8+a]<<(a*8);
 return (x>>off)&((1u<<bits)-1u);
}
```

### bridge.c

```c
#include <stdlib.h>
#include <string.h>
#include "soundpipe.h"
int sp_auxdata_alloc(sp_auxdata *aux, size_t size){aux->ptr=calloc(1,size);aux->size=size;return SP_OK;}
int sp_auxdata_free(sp_auxdata *aux){free(aux->ptr);aux->ptr=NULL;return SP_OK;}
unsigned runportable(const float*x,float*y,unsigned n,float fb,float cut){
  sp_data clock={0};clock.sr=44100;
  sp_revsc *s=0;sp_revsc_create(&s);sp_revsc_init(&clock,s);
  s->feedback=fb;s->lpfreq=cut;
  for(unsigned j=0;j<n;j++)sp_revsc_compute(&clock,s,(float*)x+j,(float*)x+n+j,y+j,y+n+j);
  unsigned bytes=s->aux.size;sp_revsc_destroy(&s);return bytes;
}
```

### guard.c

```c
#include <stdio.h>
#include <math.h>
#include "codec.c"
static uint32_t rot(uint32_t s){s^=s<<13;s^=s>>17;s^=s<<5;return s;}
int main(void){
 for(unsigned mb=6;mb<=14;mb++){
  const unsigned n=4198, width=6+mb, sz=(n*width+7)/8, mask=(1u<<width)-1;
  uint8_t store[10500];uint32_t exact[4198];
  memset(store,0,sizeof store);memset(exact,0,sizeof exact);
  uint32_t r=0xa472cf67;
  assert(sz<sizeof(store));
  for(unsigned i=0;i<n*3;i++){
   unsigned k=i%n;
   for(unsigned j=0;j<3;j++){unsigned u=(k+n+j-1)%n; assert(read_code(store,u*width,width)==exact[u]);}
   r=rot(r);uint32_t q=r&mask;exact[k]=q;write_code(store,k*width,width,q);
   for(unsigned j=0;j<3;j++){unsigned u=(k+n+j-1)%n; assert(read_code(store,u*width,width)==exact[u]);}
  }
  for(unsigned i=0;i<n;i++)assert(read_code(store,i*width,width)==exact[i]);
  for(unsigned e=1;e<=31;e++)for(unsigned a=0;a<(1u<<mb);a++){
    unsigned q=(e<<mb)|a;float x=unpack_float(q,5,mb,23);
    assert(isfinite(x)&&pack_float(x,5,mb,23)==q);
    q|=1u<<(width-1);assert(pack_float(-x,5,mb,23)==q);
  }
 }
 puts("12..20 bit arbitrary slot overwrite and epoch wrap exact; all finite signed E5/B23 nonzero codes roundtrip");
}
```

### fixture-parent.py

```python
import ctypes as C,numpy as np,hashlib,sys,json
from pathlib import Path
import emu,macho
from unicorn.arm64_const import UC_CPU_ARM64_MAX
macho.EXPECTED_SHA='6d163d79202e866d2fda96e36f752bd386641b5ae2d0b1f829a254d26b652850'
old=emu.Uc
def mk(*a,**k):u=old(*a,**k);u.ctl_set_cpu_model(UC_CPU_ARM64_MAX);return u
emu.Uc=mk
D=Path(sys.argv[1]);n=32768
A={'_sp_revsc_create':0x1003b3398,'_sp_revsc_init':0x1003b33c4,'_sp_revsc_compute':0x1003b3640,'_sp_revsc_destroy':0x1003b360c}
def original(x,cut,fb):
 e=emu.Emu(str(D/'mac-arm64')); assert all(e.im.sym_by_name[k]==v for k,v in A.items())
 sp,ind,f=e.calloc(256),e.calloc(8),e.calloc(16)
 e.w32(sp+8,44100);e.w32(sp+12,2)
 assert e.call(A['_sp_revsc_create'],ind)==1
 s=e.r64(ind);assert e.call(A['_sp_revsc_init'],sp,s)==1
 e.wf(s,float(fb));e.wf(s+4,float(cut));assert e.r64(s+0x1a8)==98760
 y=np.empty_like(x)
 for i in range(n):
  e.wf(f,float(x[0,i]));e.wf(f+4,float(x[1,i]))
  assert e.call(A['_sp_revsc_compute'],sp,s,f,f+4,f+8,f+12)==1
  y[:,i]=e.rf(f+8),e.rf(f+12)
 damp=e.rf(s+0x18)
 e.call(A['_sp_revsc_destroy'],ind);assert not e.unknown,e.unknown
 return y,damp
ptr=np.ctypeslib.ndpointer(dtype=np.float32,flags='C_CONTIGUOUS')
def model(x,div,damp,fb):
 f=C.CDLL(str(D/f'model{div}.so')).run;f.argtypes=[ptr,ptr,C.c_uint,C.c_float,C.c_float];f.restype=C.c_uint
 y=np.empty_like(x);size=f(x,y,n,fb,damp);assert size and (size<=700 or div=='float28') and np.isfinite(y).all()
 return y,size
def dbr(x,y):
 a=float(np.linalg.norm(x));b=float(np.linalg.norm(y))
 return 20*np.log10(a/b) if a and b else None
def digest(x):return hashlib.sha256(x.astype('<f4').tobytes()).hexdigest()
def describe(y):
 t=np.flatnonzero(np.any(y!=0,axis=0))
 z=np.abs(np.fft.rfft(y,axis=1))**2;hz=np.fft.rfftfreq(n,1/44100)
 return dict(first_nonzero=int(t[0]) if len(t) else None,nonzero=int(len(t)),signed_sha=digest(y),
        above_8k=float(z[:,hz>8000].sum()/z.sum()) if z.sum() else None)
i=np.zeros((2,n),'f4');i[0,0]=.25;i[1,37]=-.125
t=np.arange(n);a=(t<7424)
d=np.stack([((t*3023%65536)-32768)/196608.*a,((t*1597%65536)-32768)/196608.*a]).astype('f4')
sec=t/44100.;en=np.minimum(1.,sec/.010)*np.minimum(1.,np.maximum(0.,(.257-sec)/.040))
h=np.stack([(.135*np.sin(261.625*2*np.pi*sec)+.072*np.sin(880.*2*np.pi*sec)+.021*np.sin(12853.*2*np.pi*sec))*en,
 (.12*np.sin(329.627*2*np.pi*sec+.2)+.063*np.sin(1302.2*2*np.pi*sec)+.027*np.sin(9565.*2*np.pi*sec))*en]).astype('f4')
rows=[]
for name,x,cut,fb in [('impulse',i,4000,.7779527902603149),('impulse',i,20000,.7779527902603149),('dense',d,20000,.7779527902603149),('heldout chords + release',h,20000,.92)]:
 ref,damp=original(x,cut,fb);w=[]
 for div in (28,32,'float28'):
  y,s=model(x,div,damp,fb);r=ref.astype('f8');c=y.astype('f8')
  w.append(dict(div=div,bytes=s,**describe(y),raw_residual_dBr=dbr(c-r,r),early_raw_dBr=dbr((c-r)[:,:4096],r[:,:4096]),
          late_raw_dBr=dbr((c-r)[:,16000:],r[:,16000:]),late_level_delta_db=dbr(c[:,16000:],r[:,16000:])))
 rows.append(dict(cue=name,cut=cut,fb=fb,read_original_damping=damp,input_signed_sha=digest(x),original=describe(ref),arms=w))
 print(json.dumps(rows[-1]))
```

### portable-packed.py

```python
from pathlib import Path
from unicorn import UC_HOOK_CODE
base=Path(__file__).resolve().with_name("fixture-parent.py")
exec(compile(base.read_text().split('rows=[]')[0],str(base),'exec'))
def errors(y,ref):
 a=y.astype('f8');b=ref.astype('f8');err=a-b
 bands=np.fft.rfftfreq(n,1/44100)>8000
 return dict(raw=dbr(err,b),late=dbr(err[:,16000:],b[:,16000:]),early=dbr(err[:,:4096],b[:,:4096]),
             late_level=dbr(a[:,16000:],b[:,16000:]),hf=dbr(np.fft.rfft(err,axis=1)[:,bands],np.fft.rfft(b,axis=1)[:,bands]),
             first=describe(a)['first_nonzero'])
cues=[('impulse',i,20000,.7779527902603149),('dense',d,20000,.7779527902603149),('held chord + release',h,20000,.92),('held chord type-I',h,4000,.92)]
libs = [C.CDLL(str(D/(f"p{m}.so" if m else "portable-revsc.so"))) for m in (0,6,8,9,10,11,12,14)]
for name,x,cut,fb in cues:
 ref,damp=original(x,cut,fb)
 for lib,m in zip(libs,(0,6,8,9,10,11,12,14)):
  f=lib.runportable;f.argtypes=[ptr,ptr,C.c_uint,C.c_float,C.c_float];f.restype=C.c_uint
  out=np.empty_like(x);size=f(x,out,n,fb,cut)
  score=errors(out,ref)
  passed=score['first']==describe(ref)['first_nonzero'] and max(score[k] for k in ('raw','early','late','hf'))<=-60 and abs(score['late_level'])<=.1
  print(json.dumps(dict(cue=name,cut=cut,ref=digest(ref),portable=digest(out),m=m,size=size,provisional_pass=bool(passed),**score)),flush=True)
```

### portable-packed.final.jsonl

```json
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "667d0695f8ea772e8137498f47561da08142aa8e54a8de28ae920822ac98aa5a", "m": 0, "size": 98760, "provisional_pass": true, "raw": -78.47314974534184, "late": -76.34560583432786, "early": -79.79756837794878, "late_level": 1.4642241855840585e-05, "hf": -75.30140103031853, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "af2c9c4dcd59d2518700cff27064510374929a79871c440c94dc33693d13cd0b", "m": 6, "size": 37056, "provisional_pass": false, "raw": -51.96505936324886, "late": -44.97066307175828, "early": -70.46632377567595, "late_level": -0.0008083729072716158, "hf": -52.28077917431587, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "2c399950a62b10625d7c1ab4a622c72a2f83e5bfd96dc51af191f34e3cb30861", "m": 8, "size": 43224, "provisional_pass": false, "raw": -64.28178252231328, "late": -57.095485044941945, "early": -77.97099848234053, "late_level": -0.0012722745753398093, "hf": -64.05494473289544, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "ed7b29fb0f2a1ca2bad993da917bdaf7aaeeac2c8a2867d772270f7f41c69cd5", "m": 9, "size": 46308, "provisional_pass": true, "raw": -70.04113275357406, "late": -63.3223960942804, "early": -79.20960161335398, "late_level": 0.0007696538087657847, "hf": -69.48973042683963, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "9effb4633c7f52e8f27a7f34d747b6513643cf29497ff628221e481b6100ac38", "m": 10, "size": 49392, "provisional_pass": true, "raw": -74.19796382191467, "late": -68.5675388804573, "early": -79.78760982520646, "late_level": 0.0005160812574329236, "hf": -72.75381326862895, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "9abe4028649383635d898c27956b8fd693f8ee450937fe3d3bc40d96963c50e1", "m": 11, "size": 52484, "provisional_pass": true, "raw": -76.58654074992056, "late": -72.45670115570812, "early": -79.78707722251434, "late_level": -0.00010609772212576624, "hf": -74.15914907618028, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "8a3c2662e1feee11b580b47f94530106b75c8c965b8bcefa0a04671ee065c4b6", "m": 12, "size": 55568, "provisional_pass": true, "raw": -77.95866684919305, "late": -74.87809316994594, "early": -79.79524938467796, "late_level": 1.4071447651429263e-05, "hf": -75.01310527175991, "first": 1981}
{"cue": "impulse", "cut": 20000, "ref": "ae2ccef9be39cc9b24c34ee910ebce173c5d86723ab02757fef9a6c546dbffc0", "portable": "81f83d8858dee80b2bf29df473aee12baa8cb7a39388f45e6b6fb8801e125bcc", "m": 14, "size": 61736, "provisional_pass": true, "raw": -78.40508573964601, "late": -76.24777745877266, "early": -79.79585022263495, "late_level": -6.759612605170028e-06, "hf": -75.26319609425006, "first": 1981}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "484d000ca2ac277d046eac41291c9aa30f3edffc050602bebb4b863788c23775", "m": 0, "size": 98760, "provisional_pass": true, "raw": -86.36037266988255, "late": -83.88355427739967, "early": -88.78622792353013, "late_level": 8.473322959097655e-06, "hf": -75.01361648220751, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "b2397443b6a23d422bbec967b532804ce3cd9296b3731544bdc6f3b3c40a1278", "m": 6, "size": 37056, "provisional_pass": false, "raw": -49.80444717032814, "late": -47.18626922758589, "early": -51.95900985681462, "late_level": -0.00020795189127508613, "hf": -39.14092355646523, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "b0e3acd63a305cd1588b1dfd0da65fe2d580e07497b50df5f29f59d92e021af8", "m": 8, "size": 43224, "provisional_pass": false, "raw": -62.03924784964221, "late": -59.17886292225769, "early": -64.71551584990577, "late_level": -0.00021324589928970174, "hf": -51.31470249630641, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "c7cd6c0a35dc289cdba95001c73a744448b1a5237243177e1a23cfb0c0d91807", "m": 9, "size": 46308, "provisional_pass": false, "raw": -67.85637809157768, "late": -65.1973014023799, "early": -69.8936067180647, "late_level": -4.589600582636617e-05, "hf": -57.210729239680056, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "c38b60637aab0c791f5f64306cf4a3794f6a7f13ad4562b82b385f09f7bee66b", "m": 10, "size": 49392, "provisional_pass": true, "raw": -73.5452260845429, "late": -70.9343650974665, "early": -75.17726748040053, "late_level": 2.055215557369382e-05, "hf": -63.04242700300964, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "07006381dad8ca4934ba9a575a82f3bc68315b44ee3b1d92f21072078b675c28", "m": 11, "size": 52484, "provisional_pass": true, "raw": -78.92469940227014, "late": -76.36225170353926, "early": -80.7258084870717, "late_level": 7.153886892768812e-07, "hf": -68.30528392210869, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "ad3e33b8c70ce89c02d983b6fea91a2027052cebc135136776624e6f18e9b2b1", "m": 12, "size": 55568, "provisional_pass": true, "raw": -83.19182547597686, "late": -80.68017191637232, "early": -85.54650538749826, "late_level": 1.699153717982167e-05, "hf": -72.07503034237116, "first": 1944}
{"cue": "dense", "cut": 20000, "ref": "7e2addbe86e25cf955110b4c25570170f4ca1969bf7dfa7a7bd8df0b3308e4d5", "portable": "c29f2d6c961e024cd3cd3299fb62060255ae5778e9f69f9c2996ce705537bb9a", "m": 14, "size": 61736, "provisional_pass": true, "raw": -86.06496590697574, "late": -83.57898740847368, "early": -88.55921354932747, "late_level": 6.535056665275585e-06, "hf": -74.74982036783365, "first": 1944}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "359ed348678459d15257e5b484b2b750d77775a654456de6004025e6ae01c276", "m": 0, "size": 98760, "provisional_pass": true, "raw": -96.51082598936055, "late": -95.17253898377061, "early": -94.41302477972597, "late_level": -2.139843237145126e-05, "hf": -75.99833507670328, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "5a088d5c1a13180b4d5f37caa169dcdf4bd87a3012ad7b5716982884f880c053", "m": 6, "size": 37056, "provisional_pass": false, "raw": -48.15917539343981, "late": -46.15134547572594, "early": -48.69002388733801, "late_level": -0.0006593300583036684, "hf": -30.73468720344946, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "116fe623d745314c127886772d3c2069108a8af44e6c6e20102dc5227865cfa1", "m": 8, "size": 43224, "provisional_pass": false, "raw": -60.16273793140763, "late": -58.131997145264506, "early": -60.79921235044061, "late_level": -4.169085359330184e-05, "hf": -42.79167248779787, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "f64114d7acfbc87f726113f7a3f38ab757d8f56a394ba5d5b31cf73bf4b85bee", "m": 9, "size": 46308, "provisional_pass": false, "raw": -66.2530320898362, "late": -64.25875945892197, "early": -66.77029845459347, "late_level": 0.00012176808201116367, "hf": -48.722672146038676, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "2d2d09a3fcf73dfe0c81a8b06be7adba469d52dc366dff5f1f691341ecc268f7", "m": 10, "size": 49392, "provisional_pass": false, "raw": -72.21307095094306, "late": -70.23230495001467, "early": -73.01753701856882, "late_level": -8.658615737639768e-05, "hf": -54.772862629064505, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "2ab595218ef9867abf4e4c69762f089ae3f421c974799c0c4d84f86b200e28bf", "m": 11, "size": 52484, "provisional_pass": true, "raw": -78.12264123059741, "late": -76.11382855120186, "early": -78.54225664555489, "late_level": -2.4663511699412244e-05, "hf": -60.53234093348771, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "e35fb4df27f131b0028c18824093b3c222e3bddecd12df662794953d344931cc", "m": 12, "size": 55568, "provisional_pass": true, "raw": -83.97510951812436, "late": -82.02284853739123, "early": -84.56530814951353, "late_level": -2.9671633021694404e-05, "hf": -66.30588153923944, "first": 1945}
{"cue": "held chord + release", "cut": 20000, "ref": "4984ee40679fbdd84522896cd02358dfff0ecfc77da2b1de9cf5277a0b98a12f", "portable": "92c0926da177aa3013cb58b07c3435ace7ccc180c533adac66add13de0a3e1f8", "m": 14, "size": 61736, "provisional_pass": true, "raw": -93.4399952960706, "late": -91.79256109044547, "early": -92.3239334121994, "late_level": -1.630353171154495e-05, "hf": -74.19235080031862, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "5173e9ea9bd2807d02c8a11573a7ef502323fc11d652329f8b80214550207215", "m": 0, "size": 98760, "provisional_pass": true, "raw": -100.75980999217734, "late": -98.15314160340887, "early": -99.94753812512785, "late_level": -2.179312110643731e-05, "hf": -81.45617814818777, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "94b1fdee45b4cde1078def5cd434350c9346187686e7111fbd2718e965eb6cd3", "m": 6, "size": 37056, "provisional_pass": false, "raw": -52.346891535500205, "late": -50.44994462155785, "early": -52.01423064900161, "late_level": -0.0006636493751638475, "hf": -30.75193253394047, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "caeb4d591bd956239814ebc874ae0aa2384ef460b7a2bc7af42147f3a76bcfa6", "m": 8, "size": 43224, "provisional_pass": false, "raw": -64.61970047405879, "late": -62.77992676021561, "early": -64.27025376289093, "late_level": 3.7629331840894764e-06, "hf": -42.798438584228, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "aca60329a2a41cf58e571ed11a1357ab95d90832a41c0578c20a3b2563a368e8", "m": 9, "size": 46308, "provisional_pass": false, "raw": -70.62314323324954, "late": -68.76825166867991, "early": -70.11574179630642, "late_level": 9.642336662241518e-06, "hf": -48.793967752533895, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "fa339f63b13171a7d1d75132624b7d6a1e0e828ca555a5a97a91b006f9bc222e", "m": 10, "size": 49392, "provisional_pass": false, "raw": -76.59276580911782, "late": -74.64056582785803, "early": -76.38025867409016, "late_level": 1.9343583933267348e-05, "hf": -54.77939264885042, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "fcaa179f0bc0c932fdac06db136e78d738b6f123b4598e4e5b2d2f11f2c3c7f4", "m": 11, "size": 52484, "provisional_pass": true, "raw": -82.62138199080485, "late": -80.58807499010084, "early": -82.1484496958472, "late_level": -1.702034944200502e-05, "hf": -60.814238248863965, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "dcdbedd444536d1da9683c8f404f0ac9b6b6063f8cf07c22c2f7ab0fc8d621d2", "m": 12, "size": 55568, "provisional_pass": true, "raw": -88.2771980856475, "late": -86.37779733628301, "early": -87.89745477115738, "late_level": -2.0559322449709136e-05, "hf": -66.7355713329932, "first": 1945}
{"cue": "held chord type-I", "cut": 4000, "ref": "3982a5903c4622d083394b81b7601594c21c401486cbe710b014352c0fd4934a", "portable": "187ed8e78d756e3ab4ad3687477d6ddd3f82590ca608a42313454d23af437d80", "m": 14, "size": 61736, "provisional_pass": true, "raw": -97.63839833038695, "late": -95.32739100420451, "early": -96.97202707066985, "late_level": -1.9632762286867527e-05, "hf": -76.93817395686392, "first": 1945}
```
