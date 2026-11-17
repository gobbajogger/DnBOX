/* SPDX-License-Identifier: GPL-3.0-only */
#include "original_runtime.h"
#include "chicago_fx2_compact.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static original_runtime_part inside, outside;
static original_chicago_fx2_compact fx;
static original_chicago_tuning tuning;
static original_runtime_workspace work;
int main(void) {
 assert(original_chicago_tuning_bind(&tuning,0,0,NULL,0x6d70));
 unsigned blocks=0;
 for(unsigned factory=0;factory<ORIGINAL_CHICAGO_BACKEND_FACTORIES;factory++) {
  uint8_t cell[32]; assert(original_sound_init(cell,ORIGINAL_SOUND_CHICAGO,factory));
  assert(original_chicago_fx2_compact_init(&fx));
  original_chicago_fx2_cold_owner owner=original_chicago_fx2_compact_owner(&fx);
  assert(original_runtime_init(&outside,cell,120,&tuning,&owner)==ORIGINAL_RUNTIME_OK);
  assert(original_runtime_init(&inside,cell,120,&tuning,NULL)==ORIGINAL_RUNTIME_OK);
  assert(original_runtime_key(&outside,ORIGINAL_INPUT_USB,60,100)==ORIGINAL_RUNTIME_OK);
  assert(original_runtime_key(&inside,ORIGINAL_INPUT_USB,60,100)==ORIGINAL_RUNTIME_OK);
  for(unsigned block=0;block<24;block++) {
   assert(original_runtime_prepare(&outside,120)==ORIGINAL_RUNTIME_OK);
   assert(original_runtime_prepare(&inside,120)==ORIGINAL_RUNTIME_OK);
   if(block==12) {
    assert(original_runtime_key(&outside,ORIGINAL_INPUT_USB,60,0)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&inside,ORIGINAL_INPUT_USB,60,0)==ORIGINAL_RUNTIME_OK);
   }
   float l[256]={0},r[256]={0},ol[256]={0},orr[256]={0};
   assert(original_runtime_render(&outside,&work,ol,orr,256)==ORIGINAL_RUNTIME_OK);
   assert(original_runtime_render(&inside,&work,l,r,256)==ORIGINAL_RUNTIME_OK);
   assert(!memcmp(l,ol,sizeof l)&&!memcmp(r,orr,sizeof r));blocks++;
  }
  assert(original_runtime_retire(&inside)==ORIGINAL_RUNTIME_OK);
  assert(original_runtime_retire(&outside)==ORIGINAL_RUNTIME_OK);
  /* Reuse the same address for another engine before re-entering Chicago. */
  assert(original_sound_init(cell,ORIGINAL_SOUND_MIAMI,0));
  assert(original_runtime_init(&inside,cell,120,&tuning,NULL)==ORIGINAL_RUNTIME_OK);
  assert(original_runtime_key(&inside,ORIGINAL_INPUT_USB,64,90)==ORIGINAL_RUNTIME_OK);
  float l[256]={0},r[256]={0};
  assert(original_runtime_render(&inside,&work,l,r,256)==ORIGINAL_RUNTIME_OK);
  assert(original_runtime_retire(&inside)==ORIGINAL_RUNTIME_OK);
 }
 printf("Chicago inline/external FX: factories=%u blocks=%u bit-exact; retire/reuse PASS\n",ORIGINAL_CHICAGO_BACKEND_FACTORIES,blocks);
 return 0;
}
