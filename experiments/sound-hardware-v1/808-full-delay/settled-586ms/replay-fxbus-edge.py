#!/usr/bin/env python3
"""Host-only 808 long-delay interpolation regression: optimized vs scalar DSP.

Uses the pinned 26,112-cell candidate function unchanged. Generates the scalar
reference from the function's original per-sample fallback. This is a kernel
comparison, NOT an original 8W8 effects-on render or FM1 timing qualification.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

SOURCE_SHA = 'e82e6af2f8739c8020cc54ca4c9c3e1002a1822c'

PREFIX = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define FX_808_DELAY_FRAMES 26112
#define D9_SR 44100.f
#define D8_QUIET (FX_808_DELAY_FRAMES+1)
typedef struct {float prev, high;} d9_biquad_t;
typedef struct {
  float dl_time_ms, dl_level, dl_fdbk, dl_tc;
  float dcur,lp; int32_t w,dl_quiet;
  d9_biquad_t dl_hp;
  int16_t *dbuf;
} fxbus_t;
/* These deterministic stubs isolate the history/interpolator comparison;
   do not use them to claim fidelity of the complete high-pass or 8W8 graph. */
static float d9_biquad_tick(d9_biquad_t *f,float v) {
 float r=v-f->prev+f->high*.875f;f->prev=v;f->high=r;return r;
}
static float fm_fabsf(float x){return fabsf(x);}
static int32_t sat12_store(int16_t *cell,float v){
 int32_t k=(int32_t)(v*2048.f);if(k>32767)k=32767;
 if(k< -32767)k=-32767;*cell=(int16_t)k;return k;
}
'''

SUFFIX = r'''
static uint32_t state=0xb3cc187u;
static uint32_t rnd(void){state=state*1664525u+1013904223u;return state;}
static float mag(void){return (float)(int32_t)rnd()*0x1p-31f;}
static fxbus_t a,b;
static int16_t ar[FX_808_DELAY_FRAMES],br[FX_808_DELAY_FRAMES];
static float input[256],la[256],lb[256],ra[256],rb[256];
int main(void){
 unsigned cases=0,fast=0,fallback=0,quiet=0,stereo=0,mono=0;
 unsigned samples=0;
 for(unsigned trial=0;trial<1024;trial++)for(unsigned mode=0;mode<8;mode++){
   unsigned n=(rnd()%256)+1;
   for(int i=0;i<FX_808_DELAY_FRAMES;i++)ar[i]=(int16_t)((int32_t)(rnd()%4096)-2048);
   memcpy(br,ar,sizeof ar);
   for(int i=0;i<256;i++){
     input[i]=mag()*.25f;
     la[i]=lb[i]=mag()*.1f;ra[i]=rb[i]=mag()*.1f;
   }
   memset(&a,0,sizeof a);
   a.dbuf=ar;
   switch(mode){
    case 0:a.w=FX_808_DELAY_FRAMES-1;break;
    case 1:a.w=0;break;
    case 2:a.w=FX_808_DELAY_FRAMES-n/2;break;
    case 3:a.w=n/2;break;
    default:a.w=(int32_t)(rnd()%FX_808_DELAY_FRAMES);break;
   }
   a.dcur=16384.f+(float)(rnd()%(FX_808_DELAY_FRAMES-16384-256))
          +(float)(rnd()%512)*0x1p-9f;
   if(mode==4) a.dcur=300.f+(float)(rnd()%16000);
   a.dl_time_ms=a.dcur/44.1f;
   if(mode==2) a.dl_time_ms*=1.065f;
   if(mode==3) a.dl_time_ms*=.888f;
   a.dl_fdbk=(float)(rnd()%128)/127.f;
   a.dl_tc=.06f+(float)(rnd()%128)/200.f;
   a.dl_level=(float)(rnd()%128)/127.f;
   a.lp=mag()*.04f;a.dl_hp.prev=mag()*.03f;
   a.dl_hp.high=mag()*.03f;
   if(mode>=5)a.dl_quiet=D8_QUIET+(int32_t)(rnd()%16);
   const float *send=(mode==6 || mode==7)?NULL:input;
   if(mode==5)for(unsigned i=0;i<n;i++)input[i]=0.f;
   float gain=(float)(rnd()%128)/127.f;
   b=a;b.dbuf=br;
   float target=a.dl_time_ms*.001f*D9_SR;
   int is_quiet=(a.dl_quiet>=D8_QUIET && (send==NULL || mode==5));
   if(is_quiet)quiet++;
   else if(a.dcur>=16384.f && a.dcur+(target-a.dcur)*.0008f==a.dcur)fast++;
   else fallback++;
   float *right_a=(mode&1)?NULL:ra,*right_b=(mode&1)?NULL:rb;
   if(right_a)stereo++;else mono++;
   ref_delay(&a,send,la,right_a,(int)n,gain);
   fxbus_808_delay(&b,send,lb,right_b,(int)n,gain);
   if(memcmp(la,lb,sizeof la)||memcmp(ra,rb,sizeof ra)||
      memcmp(ar,br,sizeof ar)){
      fprintf(stderr,"PCM/history mismatch at trial=%u mode=%u frames=%u\n",trial,mode,n);return 2;
   }
   b.dbuf=a.dbuf;
   if(memcmp(&a,&b,sizeof a)){
      fprintf(stderr,"DSP state mismatch at trial=%u mode=%u frames=%u\n",trial,mode,n);return 3;
   }
   cases++;samples+=n;
 }
 printf("{\"cases\":%u,\"frames\":%u,\"stereo_cases\":%u,\"mono_cases\":%u,"
        "\"settled_cases\":%u,\"dynamic_cases\":%u,\"quiet_cases\":%u,"
        "\"pcm_ring_state_mismatches\":0}\n",cases,samples,stereo,mono,fast,fallback,quiet);
}
'''


def git_blob_sha(data):
    return hashlib.sha1(b'blob ' + str(len(data)).encode() + b'\0' + data).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--kernel', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--cc', default='cc')
    args = parser.parse_args()
    original = args.kernel.read_bytes()
    actual_sha = git_blob_sha(original)
    if actual_sha != SOURCE_SHA:
        raise SystemExit(f'wrong source blob: {actual_sha}, expected {SOURCE_SHA}')
    kernel = original.decode()
    split = kernel.index('    /* At this magnitude')
    per_sample = kernel.index('    } else for(int i=0;i<frames;i++){')
    scalar = (kernel[:split] + kernel[per_sample:].replace('    } else for', '    for', 1))
    scalar = scalar.replace('void fxbus_808_delay(', 'void ref_delay(', 1)
    args.out.mkdir(parents=True,exist_ok=True)
    with tempfile.TemporaryDirectory() as tmp:
        src = Path(tmp)/'compare.c'
        src.write_text(PREFIX + scalar + '\n' + kernel + '\n' + SUFFIX)
        results = []
        for level in ('-O0','-O2'):
            exe=Path(tmp)/('test'+level[-1])
            cmd=[args.cc,level,'-std=c11','-ffp-contract=off',
                 '-fsanitize=undefined','-fno-sanitize-recover=undefined',
                 str(src),'-lm','-o',str(exe)]
            subprocess.run(cmd,check=True,capture_output=True,text=True)
            run=subprocess.run([str(exe)],capture_output=True,text=True,check=True)
            results.append({'optimization':level,**json.loads(run.stdout)})
    receipt={
        'description':'host kernel-only parity of candidate private 808 delay vs its scalar per-sample branch',
        'source':'experiments/sound-hardware-v1/808-full-delay/settled-586ms/settled-fxbus.c.inc',
        'source_git_blob_sha1':actual_sha,
        'ring_frames':26112,
        'sample_rate_hz':44100,
        'history_duration_ms':26112/44.1,
        'maximum_configurable_tap_ms':(26112-256)/44.1,
        'scope':'interpolation, ring history, output and state; fixture HPF is not original 8W8',
        'not_qualified':['original 8W8 effects-on output','firmware flash/pool fit','Elite/Pro emulation timing','full original 1994 ms echo'],
        'results':results,
    }
    (args.out/'receipt.json').write_text(json.dumps(receipt,indent=2,sort_keys=True)+'\n')
    print(json.dumps(receipt,indent=2,sort_keys=True))

if __name__=='__main__':main()
