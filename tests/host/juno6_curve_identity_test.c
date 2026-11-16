/* SPDX-License-Identifier: GPL-3.0-only */
/* Real production old/new audio plus physical control/state witnesses. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "juno6.h"
#include "x0x_param.h"
void ref_juno6_init(juno6_t*,float);
int ref_juno6_set(juno6_t*,unsigned,float);
int ref_juno6_note_on(juno6_t*,uint8_t,uint8_t);
void ref_juno6_note_off(juno6_t*,uint8_t);
void ref_juno6_render(juno6_t*,float*,unsigned);
static unsigned checks;
#define CHECK(x) do {checks++;if(!(x)){fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);return 1;}}while(0)
static uint32_t word(float f){union{float f;uint32_t u;}v;v.f=f;return v.u;}
int main(void)
{
    const unsigned blocks[]={1,17,64,257,32,511};unsigned frames=0;
    for(int state=0;state<32;state++) {
        juno6_t a,b;juno6_init(&a,44100);ref_juno6_init(&b,44100);
        for(unsigned id=0;id<18;id++) {float q=(float)((state*53+(int)id*31)%128)/127.f;CHECK(juno6_set(&a,id,q)==ref_juno6_set(&b,id,q));}
        CHECK(!memcmp(&a,&b,sizeof a));
        for(unsigned note=0;note<6;note++)CHECK(juno6_note_on(&a,40+note*7,30+note*16)==ref_juno6_note_on(&b,40+note*7,30+note*16));
        for(unsigned k=0;k<sizeof blocks/sizeof *blocks;k++) {
            float x[512],y[512];juno6_render(&a,x,blocks[k]);ref_juno6_render(&b,y,blocks[k]);frames+=blocks[k];
            CHECK(!memcmp(x,y,blocks[k]*sizeof *x));CHECK(!memcmp(&a,&b,sizeof a));
        }
        for(unsigned note=0;note<6;note++){juno6_note_off(&a,40+note*7);ref_juno6_note_off(&b,40+note*7);}
        float x[512],y[512];juno6_render(&a,x,512);ref_juno6_render(&b,y,512);frames+=512;
        CHECK(!memcmp(x,y,sizeof x));CHECK(!memcmp(&a,&b,sizeof a));
    }
    for(int raw=0;raw<128;raw++) {
        float q=(float)raw/127.f,v;int unit;
        juno6_t s;juno6_init(&s,44100);
        for(unsigned id=0;id<16;id++)CHECK(juno6_set(&s,id,q));
        const unsigned ids[]={1,2,4,5,6,7,8,9,10,11,12,13,14,15};
        const float expected[]={s.sub_level*100.f,35.f+s.cutoff*s.cutoff*12000.f,7000.f*s.macro[4],s.attack,s.decay,s.sustain*100.f,s.release,15.f+2000.f*s.hpf*s.hpf,.1f+8.f*s.macro[10]*s.macro[10],.2f*s.macro[11]*100.f,s.pulse_width*100.f,.1f+4.9f*s.macro[13],.5f*s.macro[14]*100.f,s.macro[15]};
        for(unsigned i=0;i<sizeof ids/sizeof *ids;i++){unit=juno6_param_si(ids[i],raw,&v);CHECK(unit!=X0X_U_NONE);CHECK(word(v)==word(expected[i]));}
        float wave[1];juno6_render(&s,wave,1);unit=juno6_param_si(10,raw,&v);CHECK(unit==X0X_U_HZ);CHECK(word(s.lfo_phase)==word(v/44100.f));
    }
    CHECK(juno6_param_si(16,0,(float[1]){0})==X0X_U_NONE);
    printf("Juno shared waveform/control PASS: %u checks, %u bit-identical audio frames, all128raw controls\n",checks,frames);return 0;
}
