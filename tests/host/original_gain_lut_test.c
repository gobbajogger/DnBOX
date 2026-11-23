/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../experiments/original_synth/voice_gain.c"
#include <stdio.h>
static uint32_t bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}
static int check(uint32_t d){
 float old=1.0f/(float)d,next=duration_inverse(d);
 if(bits(old)!=bits(next)){
  fprintf(stderr,"FAIL duration=%u old=%08x new=%08x\n",d,bits(old),bits(next));return 0;
 }
 return 1;
}
/* Curve 2 sets Chicago's oscillator level before its nonlinear filter. */
static int check_oscillator_curves(void)
{
 const int32_t active[4] = {1, 1, 1, 0};
 const float maximum = ORIGINAL_OSCILLATOR_GAIN_MAX;
 const float expected[3][4] = {
  {0.f, .25f * maximum, maximum, 0.f},
  {0.f, .5f * maximum, maximum, 0.f},
  {0.f, .75f, 1.f, 0.f}
 };
 for (unsigned curve = 0; curve < 3; ++curve) {
  original_voice_gain state = {0};
  original_voice_gain_controls controls = {
   .level = {0.f, .5f, 1.f, 1.f}, .curve = curve, .unison_flag = 255,
   .filter_gain = 1, .envelope = 1, .velocity = 1, .output = 1
  };
  if (!original_voice_gain_prepare(&state, &controls, active, 32)) return 0;
  for (unsigned oscillator = 0; oscillator < 4; ++oscillator) {
   if (state.oscillator[oscillator].target != expected[curve][oscillator]) {
    fprintf(stderr, "Unexpected oscillator %u target %.8g for gain curve %u\n",
            oscillator, state.oscillator[oscillator].target, curve);
    return 0;
   }
  }
  if (state.stereo[0].target != .25f || state.stereo[1].target != .25f) return 0;
 }
 return 1;
}
int main(void){
 if(!check_oscillator_curves())return 1;
 for(uint32_t d=1;d<=64;d++)if(!check(d))return 1;
 const uint32_t edge[]={0,65,66,127,128,255,256,44100,16777215,16777216,16777217,2147482880u,UINT32_MAX};
 for(unsigned i=0;i<sizeof edge/sizeof edge[0];i++)if(!check(edge[i]))return 1;
 uint32_t rng=0x37abcdefu;
 for(unsigned i=0;i<10000000u;i++){
  rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;
  if(!check(rng))return 1;
 }
 puts("PASS: all 64 table entries, 13 duration edges, 10000000 random integer durations; const=256 B");
 return 0;
}
