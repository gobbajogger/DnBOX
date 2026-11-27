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
static float left[FRAMES],right[FRAMES];
#define REQUIRED(x) do{if(!(x)){fprintf(stderr,"guard %d failed\n",__LINE__);return 3;}}while(0)
int main(int argc,char **argv){
 REQUIRED(argc==2 && (!strcmp(argv[1],"0") || !strcmp(argv[1],"1")));
 const unsigned wet=argv[1][0]=='1';
 uint8_t cell[ORIGINAL_SOUND_BYTES];
 REQUIRED(!original_sound_init(cell,ORIGINAL_SOUND_JUNO,255));
 REQUIRED(original_sound_init(cell,ORIGINAL_SOUND_JUNO,0));
 REQUIRED(original_runtime_init(&p,cell,120.f,0,0)==ORIGINAL_RUNTIME_OK);
 REQUIRED(p.kind==ORIGINAL_SOUND_JUNO && p.dsp.juno.synth.typed);
 uint8_t room[SJ6_ROOM_COUNT]={0};
 room[SJ6_ROOM_ENABLE]=wet;room[SJ6_ROOM_SIZE]=CAND_SIZE;room[SJ6_ROOM_LOCUT]=CAND_LOCUT;room[SJ6_ROOM_MIX]=CAND_MIX;
 original_runtime_sj6_room(&p,room);
 REQUIRED(p.dsp.juno.room.armed && p.dsp.juno.room.enable==wet);
 unsigned f=0,e=0;
 for(unsigned k=0;k<sizeof blocks/sizeof *blocks;k++){
  unsigned len=blocks[k]; REQUIRED(f+len<=FRAMES);
  while(e<sizeof notes/sizeof *notes && notes[e].at==f){
   unsigned note=notes[e].key,vel=notes[e++].vel;
   REQUIRED(original_runtime_key(&p,ORIGINAL_INPUT_USB,note,vel)==ORIGINAL_RUNTIME_OK);
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
 REQUIRED(fwrite(left,sizeof(float),FRAMES,stdout)==FRAMES);
 REQUIRED(fwrite(right,sizeof(float),FRAMES,stdout)==FRAMES);
 fprintf(stderr,"pass runtime INIT=0 room-gate=%u room-size=%u locut=%u mix=%u frames=%u events=%u\n",
   wet,CAND_SIZE,CAND_LOCUT,CAND_MIX,f,e);
 return 0;
}
