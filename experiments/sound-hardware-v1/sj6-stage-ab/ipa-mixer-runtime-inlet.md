# Guarded runtime inlet for source-shaped SuperJ6 stereo mixing

This integrates the measured source mixer model into the actual host original-runtime render entry before its existing effects. It does not modify the native/stock mono fallback, existing master volume, presets, or production. The bench caller owns already-rendered independent stereo source streams and their clocks. The inlet is one-shot, refuses unsupported pan or bus volume, missing/overlapping buffers, inconsistent frame ownership, active arpeggiator event scheduling or unsupported active FX modulation. Do not use the model as a blanket gain for the compact mono path.

Six signed tests from the genuine Main receivers (two singles and combined, programmed master 1 and .5) match the model exactly through the gated host entry and tested effects-off setup. 256 signed words per arm. Twelve unsupported requests rejected without writing the output; feature disabled leaves the stock preprocessed code unchanged. This is integration of the inlet with a genuine receiver input, not terminal preset parity: the installed Apple oscillator mixer PCM is not directly observed and live Sub/Noise providers are not yet connected to this path.


### runtime.patch

```diff
--- a/firmware/src/app/original_runtime.c
+++ b/firmware/src/app/original_runtime.c
@@ -1302,6 +1302,11 @@
     if (p->status != ORIGINAL_RUNTIME_OK) return p->status;
     if (runtime_stale_tick(p, frames) != ORIGINAL_RUNTIME_OK) return p->status;
 #if X0X_ARP_ENGINE
+#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS && X0X_SUPERJ6_FACTORY
+    if(p->sj6_source_pending&&(arp_engaged(p)||p->arp.seq.sequencerLastNotesCount)){
+        p->sj6_source_pending=0;return ORIGINAL_RUNTIME_INVALID;
+    }
+#endif
     if (arp_engaged(p) || p->arp.seq.sequencerLastNotesCount) {
         /* S1Sequencer::process runs once per sample, as in S1DSPKernel; the
          * block is split where it turns notes on or off. */
@@ -1569,10 +1574,58 @@
 #endif
 #endif
 
+#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS && X0X_SUPERJ6_FACTORY
+/* External diagnostic provider owns rendering and event progress for this
+ * block. Never try to recover independent lanes from compact mono mix[2]. */
+static int sj6_source_block_render(original_runtime_part *p,float *left,float *right,unsigned frames)
+{
+    const sj6_source_block *b=p->sj6_source_pending;
+    if(!b)return -1;
+    p->sj6_source_pending=0;
+    if(p->kind!=ORIGINAL_SOUND_JUNO||!left||!right||left==right||!frames||frames>256u||
+       b->frames!=frames||!(b->trim>=0.f&&b->trim<=1.f))return 0;
+#if !X0X_SUPERJ6_BUILTIN_INIT
+    if(!p->dsp.juno.synth.typed)return 0;
+#endif
+#if X0X_SJ6_AUX_LFO_NATIVE
+    for(unsigned i=0;i<SJ6_AUX_FX_ROUTES;i++)
+       if(sj6_aux_fx_supported(sj6_aux_destination_id(juno6_aux_fx_route(&p->dsp.juno.synth,i))))
+          return 0; /* requires real time-tagged modulation from source owner */
+#endif
+    for(unsigned k=0;k<4;k++) {
+        if(!b->left[k]||!b->right[k]||b->volume[k]!=1.f||b->pan[k]!=0.f)return 0;
+        /* Refuse input/output overlap, including partial planar overlap. */
+        uintptr_t il=(uintptr_t)b->left[k],ir=(uintptr_t)b->right[k];
+        uintptr_t ol=(uintptr_t)left,orr=(uintptr_t)right,n=(uintptr_t)frames*sizeof(float);
+        #define SJ6_OVERLAP(x,y) ((x)>(y)?(x)-(y)<n:(y)-(x)<n)
+        if(SJ6_OVERLAP(il,ol)||SJ6_OVERLAP(il,orr)||SJ6_OVERLAP(ir,ol)||
+           SJ6_OVERLAP(ir,orr)||SJ6_OVERLAP(ol,orr))return 0;
+        #undef SJ6_OVERLAP
+    }
+    for(unsigned i=0;i<frames;i++){
+        float l=b->left[0][i],r=b->right[0][i];
+        for(unsigned k=1;k<4;k++){l+=b->left[k][i];r+=b->right[k][i];}
+        left[i]=l*b->trim;right[i]=r*b->trim;
+    }
+    return 1;
+}
+#endif
+
 static int render_backend(original_runtime_part *p, original_runtime_workspace *w,
     float *left, float *right, unsigned frames)
 {
     int ok;
+#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS && X0X_SUPERJ6_FACTORY
+    int source=sj6_source_block_render(p,left,right,frames);
+    if(source>=0){
+        if(source) sj6_run_effects(p,left,right,frames
+#if X0X_SJ6_AUX_LFO_NATIVE
+            ,0
+#endif
+        );
+        return runtime_result(p,source);
+    }
+#endif
     if(p->kind==ORIGINAL_SOUND_JUNO){
 #if X0X_SUPERJ6_FACTORY
 #if X0X_SJ6_AUX_LFO_NATIVE
--- a/firmware/src/app/original_runtime.h
+++ b/firmware/src/app/original_runtime.h
@@ -68,6 +68,10 @@
 #endif
 typedef struct {
     original_runtime_backend dsp;
+#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS
+    const struct sj6_source_block *sj6_source_pending; /* explicit one-shot bench owner */
+#endif
+
 #if X0X_SJ6_CLASSIC_I_PROBE
     classic_mode_i classic_i_effect;
     uint64_t classic_i_sample;
@@ -84,6 +88,13 @@
     uint8_t kind;
     int8_t status;
 } original_runtime_part;
+#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS
+typedef struct sj6_source_block {
+    const float *left[4],*right[4];
+    float volume[4],pan[4],trim;
+    unsigned frames;
+} sj6_source_block;
+#endif
 typedef union {
     float metal[4][256]; /* split 808 render, before the owning bass uses this space */
     struct { uint32_t left[256], right[256]; int16_t converted[2][256], scratch[4][256]; } chicago;
```

### wire.py

```python
"""Produce an isolated runtime overlay. No write to exact donor."""
from pathlib import Path
import sys, hashlib
root=Path(sys.argv[1]); out=Path(sys.argv[2]);out.mkdir(parents=True,exist_ok=True)
base=root/'original_runtime.c'; s=base.read_text()
assert hashlib.sha256(base.read_bytes()).hexdigest()=='35efa9b84fb207b0f905771d3d0007beb7a51ca5eb1a27a4570a03505c5e8a4b'
h=(Path(sys.argv[3])/'original_runtime.h').read_text()
a='    original_runtime_backend dsp;'
assert h.count(a)==1
h=h.replace(a,a+'''
#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS
    const struct sj6_source_block *sj6_source_pending; /* explicit one-shot bench owner */
#endif
''')
a='typedef union {\n    float metal'
assert h.count(a)==1
h=h.replace(a,'''#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS
typedef struct sj6_source_block {
    const float *left[4],*right[4];
    float volume[4],pan[4],trim;
    unsigned frames;
} sj6_source_block;
#endif
'''+a)
out.joinpath('original_runtime.h').write_text(h)
a='static int render_backend(original_runtime_part *p, original_runtime_workspace *w,'
assert s.count(a)==1
s=s.replace(a,'''#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS && X0X_SUPERJ6_FACTORY
/* External diagnostic provider owns rendering and event progress for this
 * block. Never try to recover independent lanes from compact mono mix[2]. */
static int sj6_source_block_render(original_runtime_part *p,float *left,float *right,unsigned frames)
{
    const sj6_source_block *b=p->sj6_source_pending;
    if(!b)return -1;
    p->sj6_source_pending=0;
    if(p->kind!=ORIGINAL_SOUND_JUNO||!left||!right||left==right||!frames||frames>256u||
       b->frames!=frames||!(b->trim>=0.f&&b->trim<=1.f))return 0;
#if !X0X_SUPERJ6_BUILTIN_INIT
    if(!p->dsp.juno.synth.typed)return 0;
#endif
#if X0X_SJ6_AUX_LFO_NATIVE
    for(unsigned i=0;i<SJ6_AUX_FX_ROUTES;i++)
       if(sj6_aux_fx_supported(sj6_aux_destination_id(juno6_aux_fx_route(&p->dsp.juno.synth,i))))
          return 0; /* requires real time-tagged modulation from source owner */
#endif
    for(unsigned k=0;k<4;k++) {
        if(!b->left[k]||!b->right[k]||b->volume[k]!=1.f||b->pan[k]!=0.f)return 0;
        /* Refuse input/output overlap, including partial planar overlap. */
        uintptr_t il=(uintptr_t)b->left[k],ir=(uintptr_t)b->right[k];
        uintptr_t ol=(uintptr_t)left,orr=(uintptr_t)right,n=(uintptr_t)frames*sizeof(float);
        #define SJ6_OVERLAP(x,y) ((x)>(y)?(x)-(y)<n:(y)-(x)<n)
        if(SJ6_OVERLAP(il,ol)||SJ6_OVERLAP(il,orr)||SJ6_OVERLAP(ir,ol)||
           SJ6_OVERLAP(ir,orr)||SJ6_OVERLAP(ol,orr))return 0;
        #undef SJ6_OVERLAP
    }
    for(unsigned i=0;i<frames;i++){
        float l=b->left[0][i],r=b->right[0][i];
        for(unsigned k=1;k<4;k++){l+=b->left[k][i];r+=b->right[k][i];}
        left[i]=l*b->trim;right[i]=r*b->trim;
    }
    return 1;
}
#endif

'''+a)
a='''    float *left, float *right, unsigned frames)\n{\n    int ok;
    if(p->kind==ORIGINAL_SOUND_JUNO){'''
assert s.count(a)==1
s=s.replace(a,'''    float *left, float *right, unsigned frames)\n{\n    int ok;
#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS && X0X_SUPERJ6_FACTORY
    int source=sj6_source_block_render(p,left,right,frames);
    if(source>=0){
        if(source) sj6_run_effects(p,left,right,frames
#if X0X_SJ6_AUX_LFO_NATIVE
            ,0
#endif
        );
        return runtime_result(p,source);
    }
#endif
    if(p->kind==ORIGINAL_SOUND_JUNO){''')
a='    if (arp_engaged(p) || p->arp.seq.sequencerLastNotesCount) {'
guard='''#if defined(X0X_HOST) && X0X_SJ6_SOURCE_MIX_WITNESS && X0X_SUPERJ6_FACTORY
    if(p->sj6_source_pending&&(arp_engaged(p)||p->arp.seq.sequencerLastNotesCount)){
        p->sj6_source_pending=0;return ORIGINAL_RUNTIME_INVALID;
    }
#endif
'''
assert s.count(a)==1
s=s.replace(a,guard+a)
out.joinpath('original_runtime.c').write_text(s)
```

### build.sh

```bash
#!/bin/sh
set -eu
base="$PWD/firmware-fit-next/tree/firmware/src"
donor="$PWD/sj6-fold32-recovery/exact-donor/firmware/src/app"
T="$(mktemp -d)"
trap 'rm -rf "$T"' EXIT
ln -s "$base/dsp" "$T/dsp"
ln -s "$base/x0x_cold.h" "$T/x0x_cold.h"
python sj6-runtime-source-mix/wire.py "$donor" "$T/app" "$base/app"
baseflags="-std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1"
inc="-I$base/../../experiments/original_synth -I$base/app -I$base/dsp/synthone -I$base/dsp"
cp "$donor/original_runtime.c" "$T/app/base.c"
cc $baseflags $inc -E -P "$T/app/base.c" > "$T/off-base.i"
cc $baseflags $inc -E -P "$T/app/original_runtime.c" > "$T/off-guard.i"
cmp "$T/off-base.i" "$T/off-guard.i"
cc -std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SJ6_SOURCE_MIX_WITNESS=1 -ffunction-sections -fdata-sections -O2 -ffp-contract=off -fPIC -I"$base/../../experiments/original_synth" -I"$base/app" -I"$base/dsp/synthone" -I"$base/dsp" -c "$T/app/original_runtime.c" -o "$T/runtime.o"
cat >>"$T/app/original_runtime.c" <<'C'
int sj6_probe(const sj6_source_block *b,float *l,float *r,unsigned n,float master)
{
    original_runtime_part p={0};
    original_runtime_workspace w;
    p.kind=ORIGINAL_SOUND_JUNO;
    p.status=ORIGINAL_RUNTIME_OK;
    p.dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
#if !X0X_SUPERJ6_BUILTIN_INIT
    p.dsp.juno.synth.typed=1;
#endif
    p.dsp.juno.synth.master_gain=master;
    p.sj6_source_pending=b;
    return render_backend(&p,&w,l,r,n);
}
C
flags="-std=gnu11 -DX0X_DB=1 -DX0X_HOST=1 -DX0X_SUPERJ6_FACTORY=1 -DX0X_SJ6_SOURCE_MIX_WITNESS=1 -O2 -ffp-contract=off -fPIC"
cc $flags -I"$base/../../experiments/original_synth" -I"$base/app" -I"$base/dsp/synthone" -I"$base/dsp" -c "$T/app/original_runtime.c" -o "$T/runtime.o"
cc $flags -I"$base/dsp" -c "$base/dsp/juno6.c" -o "$T/juno.o"
cc -shared "$T/runtime.o" "$T/juno.o" -lm -o "$T/owner.so"
ORIGINAL_SO="$T/owner.so" PYTHONPATH=/tmp/sj6-room-next-py:$PWD/ab-effects-readonly/sj6-fullbench/bench python sj6-runtime-source-mix/check.py
```

### check.py

```python
"""Run actual guarded render_backend with private genuine signed stereo fixture blocks."""
from pathlib import Path
import ctypes as C, os
p=Path('sj6-ipa-source-next/mix-driver.py')
q=Path('sj6-mix-boundary-recheck/wrapper.py')
exec(compile(q.read_text().split('def run(')[0],str(q),'exec'))
lib=C.CDLL(os.environ['ORIGINAL_SO'],mode=os.RTLD_LAZY)
F=C.c_float; P=C.POINTER(F)
class Block(C.Structure):
 _fields_=[('left',P*4),('right',P*4),('volume',F*4),('pan',F*4),('trim',F),('frames',C.c_uint)]
run=lib.sj6_probe
run.argtypes=[C.POINTER(Block),P,P,C.c_uint,F]
run.restype=C.c_int
stay=[]
for role in ('oscSampler1','oscSampler2'):
 d=sampler_owner(role); w=wrapped(d)
 for _ in range(128):w(128)
 e.call(0x100389c84,d.dsp,36,86,doubles=(0.,))
 for _ in range(40):a=w(128)
 stay.append(a)
assert not e.unknown
zero=np.zeros((2,128),dtype=np.float32)
def arr(a):return a.ctypes.data_as(P)
trim=np.float32(.8670601844787598)
checks={}
for name,t in [('first',[stay[0],zero,zero,zero]),('second',[zero,stay[1],zero,zero]),
               ('both',[*stay,zero,zero])]:
 b=Block((P*4)(*[arr(a[0]) for a in t]),(P*4)(*[arr(a[1]) for a in t]),(F*4)(1,1,1,1),(F*4)(),trim,128)
 for master in (1.,.5):
  x=np.zeros((2,128),dtype=np.float32)
  assert run(C.byref(b),arr(x[0]),arr(x[1]),128,master)==1
  expected=(t[0]+t[1]+t[2]+t[3])*trim*np.float32(master)
  assert np.array_equal(x.view('u4'),expected.view('u4')),np.max(abs(x-expected))
  checks[str((name,master))]=dict(words=x.size,peak=float(np.abs(x).max()))
 for mode in ('pan','volume','wrong count','partial overlap'):
  x=np.full((2,128),np.float32(23))
  if mode=='pan':b.pan[1]=.25
  if mode=='volume':b.volume[1]=.9
  if mode=='wrong count':b.frames=127
  if mode=='partial overlap':b.left[0]=arr(x[1][1:])
  assert run(C.byref(b),arr(x[0]),arr(x[1]),128,1.)==-2 and np.all(x==23)
  b.pan[1]=0;b.volume[1]=1;b.frames=128;b.left[0]=arr(t[0][0])
print(json.dumps(dict(scope=__doc__,test=checks,refusals=12),indent=2))
```

### result.json

```json
{
  "scope": "Current actual processEvents closure -> real DSP stereo sampler output before Apple mixer.",
  "test": {
    "('first', 1.0)": {
      "words": 256,
      "peak": 0.042636170983314514
    },
    "('first', 0.5)": {
      "words": 256,
      "peak": 0.021318085491657257
    },
    "('second', 1.0)": {
      "words": 256,
      "peak": 0.09088548272848129
    },
    "('second', 0.5)": {
      "words": 256,
      "peak": 0.045442741364240646
    },
    "('both', 1.0)": {
      "words": 256,
      "peak": 0.11201544106006622
    },
    "('both', 0.5)": {
      "words": 256,
      "peak": 0.05600772053003311
    }
  },
  "refusals": 12
}
```
