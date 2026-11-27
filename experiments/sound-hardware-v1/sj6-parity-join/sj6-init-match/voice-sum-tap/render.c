#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "original_runtime.h"
#include "cue.h"
#ifndef X0X_SUPERJ6_BUILTIN_INIT
#error requires the actual typed J6 runtime
#endif
#if !X0X_SJ6_NATIVE_ROOM
#error requires actual room receiver in the combined firmware path
#endif
static original_runtime_part p;
static original_runtime_workspace w;
static float left[FRAMES],right[FRAMES],t[2][FRAMES];
static unsigned ti;
static float mix[2][FRAMES]; static unsigned mi;
void dnbox_j6_mix_tap(float a,float b){
 if(mi>=FRAMES)abort();
 mix[0][mi]=a; mix[1][mi++]=b;
}

void dnbox_j6_tap(const float *l,const float *r,unsigned n){
 if(ti+n>FRAMES)abort();
 memcpy(t[0]+ti,l,n*sizeof *l); memcpy(t[1]+ti,r,n*sizeof *r); ti+=n;
}
#define REQUIRED(x) do{if(!(x)){fprintf(stderr,"guard %d failed\n",__LINE__);return 3;}}while(0)
int main(int argc,char **argv){
 REQUIRED(argc==4); unsigned probe=(unsigned)atoi(argv[3]); REQUIRED(probe<=3); unsigned velocity=(unsigned)atoi(argv[1]),mode=(unsigned)atoi(argv[2]); REQUIRED(velocity>0 && velocity<128 && (mode==0 || mode==3 || mode==6));
 uint8_t cell[ORIGINAL_SOUND_BYTES];
 REQUIRED(!original_sound_init(cell,ORIGINAL_SOUND_JUNO,255));
 REQUIRED(original_sound_init(cell,ORIGINAL_SOUND_JUNO,0));
 REQUIRED(original_runtime_init(&p,cell,120.f,0,0)==ORIGINAL_RUNTIME_OK);
 REQUIRED(p.kind==ORIGINAL_SOUND_JUNO && p.dsp.juno.synth.typed);
 for(unsigned l=0;l<DNBX_LAYERS;++l){
    dnbx_layer_params x;
    REQUIRED(dnbx_get_layer(&p.dsp.juno.synth.core,l,&x)==DNBX_OK);
    fprintf(stderr,"defaults %u amp %g %g %g %g layer gain %g wave %g cutoff %g resonance %g env %g\n",l,
         x.amp.attack_s,x.amp.decay_s,x.amp.sustain,x.amp.release_s,x.gain,x.waveform,x.cutoff_hz,x.resonance,x.filter_env_hz);
    if(probe&1) x.amp.sustain=1.f; /* native Init readback = 1; source pow(1, 2.5)=1 */
    if((probe&2)&&l==1) x.gain=0.f; /* native saw receiver off; only a controlled ablation */
    if(probe) REQUIRED(dnbx_set_layer(&p.dsp.juno.synth.core,l,&x)==DNBX_OK);
 }
 unsigned f=0,e=0;
 for(unsigned k=0;k<sizeof blocks/sizeof *blocks;k++){
  unsigned len=blocks[k]; REQUIRED(f+len<=FRAMES);
  while(e<sizeof notes/sizeof *notes && notes[e].at==f){
   unsigned note=notes[e].key,vel=notes[e++].vel;
   if(vel) vel=velocity;
   REQUIRED(original_runtime_sj6_key(&p,ORIGINAL_INPUT_USB,note,vel,mode)==ORIGINAL_RUNTIME_OK);
   REQUIRED(p.keys[ORIGINAL_INPUT_USB][note]==(vel!=0));
   fprintf(stderr,"event %u %u %u %u\n",f,note,vel,p.keys[ORIGINAL_INPUT_USB][note]);
  }
  REQUIRED(e==sizeof notes/sizeof *notes || notes[e].at>=f+len);
  REQUIRED(original_runtime_render(&p,&w,left+f,right+f,len)==ORIGINAL_RUNTIME_OK);
  f+=len;
 }
 REQUIRED(f==FRAMES && e==sizeof notes/sizeof *notes);
 for(unsigned i=0;i<128;i++)REQUIRED(p.keys[ORIGINAL_INPUT_USB][i]==0);
 for(unsigned i=0;i<FRAMES;i++)REQUIRED(isfinite(left[i])&&isfinite(right[i]));
 REQUIRED(ti==FRAMES);
 REQUIRED(mi==FRAMES);
 FILE *mf=fopen("mix.f32","wb"); REQUIRED(mf && fwrite(mix,sizeof mix,1,mf)==1 && fclose(mf)==0);

 FILE *fout=fopen("pre-effects.f32","wb"); REQUIRED(fout && fwrite(t,sizeof t,1,fout)==1 && fclose(fout)==0);
 REQUIRED(fwrite(left,sizeof(float),FRAMES,stdout)==FRAMES);
 REQUIRED(fwrite(right,sizeof(float),FRAMES,stdout)==FRAMES);
 fprintf(stderr,"pass unmodified compact Init, frames=%u events=%u\n",f,e);
 return 0;
}
