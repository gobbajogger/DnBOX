/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_pan_port.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
static float as_float(uint32_t x){union{uint32_t u;float f;}v={.u=x};return v.f;}
static uint32_t as_word(float x){union{uint32_t u;float f;}v={.f=x};return v.u;}
static const float lo=0x1.111112p-11f,hi=10.f;
static unsigned old_pos(float hz){return (unsigned)(127.f*powf((hz-lo)/(hi-lo),.4f)+.5f);}
int main(void){
 for(unsigned i=0;i<128;i++){
  assert(as_word(s1_pan_free_select(i))==as_word(lo+(hi-lo)*powf((float)i/127.f,2.5f)));
  assert(s1_pan_free_position(s1_pan_free_select(i))==i);
 }
 for(unsigned a=0;a<127;a++){
  uint32_t h=as_word(s1_pan_boundary_hz[a]);
  for(uint32_t j=h-64;j<=h+64;j++)assert(s1_pan_free_position(as_float(j))==old_pos(as_float(j)));
 }
 for(unsigned i=0;i<1048576;i++){
  const float f=as_float(as_word(lo)+(uint64_t)i*(as_word(hi)-as_word(lo))/1048575);
  assert(s1_pan_free_position(f)==old_pos(f));
  const float rates[]={11025.f,22050.f,44100.f,48000.f};
  for(unsigned j=0;j<4;j++){
   const float factor=(float)(16777216.0/(double)rates[j]);
   assert(s1_pan_q24_increment(f,factor)==(uint32_t)lrintf(f*factor));
  }
 }
 unsigned ties=0;
 for(unsigned i=0;i<1522;i++){
  assert(s1_pan_q24_increment((float)i+0.5f,1.f)==(uint32_t)lrintf((float)i+0.5f));ties++;
 }
 printf("128 exact detents; 16383 boundary neighbours; 1048576 float inputs; 4194304 rate steps; %u half ties\n",ties);
}
