#include "firmware/src/app/original_runtime.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Unrelated backend branches abort if accidentally reached. */
int original_chicago_backend_prepare(original_chicago_backend *x){(void)x;abort();}
int original_chicago_backend_tempo(original_chicago_backend *x,float b){(void)x;(void)b;abort();}
int original_miami_backend_prepare(original_miami_backend *x){(void)x;abort();}
int original_miami_backend_set_tempo(original_miami_backend *x,float b){(void)x;(void)b;abort();}
int original_chicago_backend_retire(original_chicago_backend *x){(void)x;abort();}
void juno6_delay24_process(juno6_t *, float *, float *, unsigned);
static original_runtime_part p={.status=ORIGINAL_RUNTIME_OK,.kind=ORIGINAL_SOUND_JUNO};
static int16_t arena[2048],other[2048];
int main(void){
    const uint8_t q[]={3,42,63,64},bad[]={3,42,63,128},top[]={3,127,63,64};
    juno6_t *s=&p.dsp.juno.synth;
    s->sample_rate=44100;
    assert(!original_runtime_delay_bind(&p,arena,7));
    s->typed=1;
    assert(original_runtime_delay_bind(&p,arena,7));
    assert(!original_runtime_delay_bind(&p,other,7));
    assert(!original_runtime_delay_publish(&p,6,1,q,4));
    assert(!original_runtime_delay_publish(&p,7,1,bad,4));
    assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK && !s->delay24.tap);
    assert(original_runtime_delay_publish(&p,7,1,q,4));
    assert(original_runtime_delay_result(&p)==1 && !s->delay24.tap);
    assert(!original_runtime_delay_publish(&p,7,1,q,4));
    assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_delay_result(&p)==2 && s->delay24.tap==arena);
    assert(s->delay24.sync && s->delay24.enabled && s->delay24.mix==64);
    assert(!memcmp(p.dsp.juno.lab_delay.applied,q,4));
    assert(original_runtime_delay_publish(&p,7,1,top,4));
    assert(original_runtime_prepare(&p,0)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_delay_result(&p)==3 && s->delay24.clock_refused && !memcmp(p.dsp.juno.lab_delay.applied,q,4));
    assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK && !s->delay24.clock_refused);
    /* All first-detent raw values must agree, without collapsing readback. */
    for (unsigned raw=0; raw<4; ++raw) {
        const uint8_t low[]={3,(uint8_t)raw,63,64};
        assert(original_runtime_delay_publish(&p,7,1,low,4));
        assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_delay_result(&p)==2);
        assert(s->delay24.target*s->delay24.stride==1378.125f);
        assert(!memcmp(p.dsp.juno.lab_delay.applied,low,4));
    }
    /* A live change within the same ring stride retimes without clearing. */
    float left[64]={.5f},right[64]={-.25f};
    juno6_delay24_process(s,left,right,64);
    unsigned filled=s->delay24.filled, stride=s->delay24.stride;
    assert(filled>0);
    assert(original_runtime_prepare(&p,100)==ORIGINAL_RUNTIME_OK);
    assert(s->delay24.target*stride>1653.749f && s->delay24.target*stride<1653.751f);
    assert(s->delay24.filled==filled && s->delay24.stride==stride && s->delay24.ramp[0]>0);
    assert(p.dsp.juno.lab_delay.applied[1]==3);
    assert(original_runtime_delay_publish(&p,7,1,top,4));
    assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_delay_result(&p)==2);
    assert(s->delay24.target*s->delay24.stride==132300.f);
    assert(original_runtime_prepare(&p,20)==ORIGINAL_RUNTIME_OK);
    assert(s->delay24.target*s->delay24.stride==132300.f);
    assert(!memcmp(p.dsp.juno.lab_delay.applied,top,4));
    const uint8_t free_zero[]={1,0,63,64};
    assert(original_runtime_delay_publish(&p,7,1,free_zero,4));
    assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK);
    assert(s->delay24.target*s->delay24.stride==1.f && !s->delay24.sync);
    assert(original_runtime_delay_publish(&p,7,1,q,4));
    assert(original_runtime_retire(&p)==ORIGINAL_RUNTIME_OK);
    assert(!p.dsp.juno.lab_delay.arena && !p.dsp.juno.lab_delay.ready && !s->delay24.tap);
    assert(!original_runtime_delay_publish(&p,7,1,q,4));
    puts("Real publish -> prepare -> Sync detents and clamp -> refusal/recover -> retire passed");
}
