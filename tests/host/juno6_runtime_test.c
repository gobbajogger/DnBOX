/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "../../firmware/src/app/original_runtime.h"
static original_runtime_part p;
static original_runtime_workspace w;
/* Count sounding voices of a pitch, including pedal holds and release tails. */
static unsigned sounding_pitch(unsigned note)
{
    unsigned count = 0;
    for (unsigned i = 0; i < JUNO6_VOICES; ++i) {
        const juno6_voice_t *voice = &p.dsp.juno.synth.voice[i];
        count += voice->stage != 0 && voice->note == note;
    }
    return count;
}

static void check_rekey(unsigned program, unsigned pedal)
{
    uint8_t cell[32];
    float left[256], right[256];
    assert(original_sound_init(cell, ORIGINAL_SOUND_JUNO, program));
    assert(original_runtime_init(&p, cell, 120, 0, 0) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 67, 92) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 60, 100) == ORIGINAL_RUNTIME_OK);
    for (unsigned i = 0; i < 8; ++i)
        assert(original_runtime_render(&p, &w, left, right, 256) == ORIGINAL_RUNTIME_OK);
    if (pedal) assert(original_runtime_param(&p, 17, 1.f) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 60, 0) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_render(&p, &w, left, right, 64) == ORIGINAL_RUNTIME_OK);
    assert(sounding_pitch(60) == 1);
    assert(original_runtime_key(&p, ORIGINAL_INPUT_KEYS, 60, 83) == ORIGINAL_RUNTIME_OK);
    assert(sounding_pitch(60) == 1 && sounding_pitch(67) == 1);
    assert(original_runtime_retire(&p) == ORIGINAL_RUNTIME_OK);
}

int main(void){
 for (unsigned program=0;program<4;++program)
     for (unsigned pedal=0;pedal<2;++pedal) check_rekey(program,pedal);
 uint8_t cell[32];float l[256],r[256],energy=0;
 assert(original_sound_init(cell,ORIGINAL_SOUND_JUNO,0));
 assert(original_runtime_init(&p,cell,120,0,0)==ORIGINAL_RUNTIME_OK);
 for(unsigned i=0;i<6;i++)assert(original_runtime_key(&p,ORIGINAL_INPUT_KEYS,48+3*i,100)==1);
 for(unsigned i=0;i<6;i++)assert(p.dsp.juno.synth.voice[i].gate);
 for(unsigned b=0;b<10;b++){assert(original_runtime_render(&p,&w,l,r,256)==1);for(unsigned i=0;i<256;i++){assert(isfinite(l[i])&&isfinite(r[i]));energy+=fabsf(l[i]);}}
 assert(energy>1);
 assert(original_runtime_param(&p,17,1)==1);
 assert(original_runtime_release_source(&p,ORIGINAL_INPUT_KEYS)==1);
 assert(juno6_active(&p.dsp.juno.synth));
 assert(original_runtime_param(&p,17,0)==1);
 assert(original_runtime_all_off(&p)==1);
 assert(!p.dsp.juno.synth.damper);
 for(unsigned b=0;b<2;b++){assert(original_runtime_render(&p,&w,l,r,256)==1);for(unsigned i=0;i<256;i++)assert(l[i]==0 && r[i]==0);}
 assert(!original_runtime_active(&p));
 assert(original_runtime_retire(&p)==1);
 printf("juno production runtime: six voices, sustain, release, render PASS; backend=%zu juno=%zu\n",sizeof(original_runtime_backend),sizeof p.dsp.juno);
}
