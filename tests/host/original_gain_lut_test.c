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
/* Compiled controls are shared by voices. Compare both live overlays with
 * the effective controls each engine would pass to the public gain API. */
static int check_gain_overlays(void)
{
 const int32_t active[4] = {0, -1, 3, 0};
 original_voice_gain initial = {0};
 initial.oscillator[0].current = .2f;
 initial.stereo[1].current = .4f;
 for (unsigned chicago = 0; chicago <= 1; chicago++) {
  original_voice_gain_controls program = {
   .level = {.5f, .2f, .3f, 1.f}, .output = .7f, .pan = -.4f,
   .curve = 2, .unison_flag = 255, .filter_gain = 1.f, .ms20 = 1
  };
  if (!chicago) {
   program.mod_level = .1f;
   program.mod_specific[2] = -.25f;
   program.mod_pan = .2f;
   program.spread = .65f;
  }
  original_chicago_voice_gain_controls compact = {0};
  memcpy(compact.level, program.level, sizeof compact.level);
  compact.pan = program.pan;
  compact.output = program.output;
  compact.curve = program.curve;
  compact.unison_flag = program.unison_flag;
  compact.ms20 = program.ms20;
  original_voice_gain_controls effective = program;
  effective.envelope = -.8f;
  effective.mod_gain = .4f;
  effective.velocity = .6f;
  if (!chicago) effective.unison_flag = 2;
  const unsigned char *overlay = chicago ? (const void *)&compact : (const void *)&program;
  size_t size = chicago ? sizeof compact : sizeof program;
  unsigned char saved[sizeof program];
  memcpy(saved, overlay, size);
  for (unsigned invalid = 0; invalid <= 1; invalid++) {
   original_voice_gain expected = initial;
   if (invalid) expected.stereo[1].current = NAN;
   original_voice_gain actual = expected, untouched = expected;
   int ref = original_voice_gain_prepare(&expected, &effective, active, 63);
   int ok = chicago ?
     original_voice_gain_prepare_chicago(&actual, &compact, active, 63, -.8f, .4f, .6f, 1.f) :
     original_voice_gain_prepare_miami(&actual, &program, active, 63, -.8f, .4f, .6f, 2);
   if (ref != !invalid || ok != ref || memcmp(&actual, &expected, sizeof actual) ||
       memcmp(saved, overlay, size) || (invalid && memcmp(&actual, &untouched, sizeof actual))) {
    fprintf(stderr, "Gain overlay differs for chicago=%u invalid=%u\n", chicago, invalid);
    return 0;
   }
  }
 }
 return 1;
}
int main(void){
 if(!check_oscillator_curves() || !check_gain_overlays())return 1;
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
