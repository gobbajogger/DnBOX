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
static original_runtime_part p={.status=ORIGINAL_RUNTIME_OK,.kind=ORIGINAL_SOUND_JUNO};
static int16_t arena[2048],other[2048];
int main(void){
    const uint8_t q[]={3,42,63,64},bad[]={3,42,63,128},too_long[]={3,127,63,64};
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
    assert(original_runtime_delay_publish(&p,7,1,too_long,4));
    assert(original_runtime_prepare(&p,0)==ORIGINAL_RUNTIME_OK);
    assert(original_runtime_delay_result(&p)==3 && s->delay24.clock_refused && !memcmp(p.dsp.juno.lab_delay.applied,q,4));
    assert(original_runtime_prepare(&p,120)==ORIGINAL_RUNTIME_OK && !s->delay24.clock_refused);
    assert(original_runtime_delay_publish(&p,7,1,q,4));
    assert(original_runtime_retire(&p)==ORIGINAL_RUNTIME_OK);
    assert(!p.dsp.juno.lab_delay.arena && !p.dsp.juno.lab_delay.ready && !s->delay24.tap);
    assert(!original_runtime_delay_publish(&p,7,1,q,4));
    puts("21 assertions: real publish -> prepare -> refusal/recover -> retire passed");
}
