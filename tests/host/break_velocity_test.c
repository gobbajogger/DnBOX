/* SPDX-License-Identifier: GPL-3.0-only */
/* Reuse original real-ADPCM fixtures and replay every original case. */
#define main original_break_performance_main
#include "break_performance_test.c"
#undef main
static void seq_setup(breaks_t *b, int velocity)
{
    breaks_init(b); brk_loop_t l={data,N,44100,1,8};
    breaks_set_loop(b,0,&l);breaks_set_loop(b,1,&l);
    breaks_velocity(b,velocity);breaks_pin(b,0);
    breaks_step(b,0,0,(float)N/16.0f,1);
}
int main(void)
{
    if(original_break_performance_main())return 1;
    breaks_t a,b,c;
    static float full[4096],half[4096],legacy[4096];
    seq_setup(&a,0);seq_setup(&b,127);seq_setup(&c,64);
    render(&a,legacy,4096,256);render(&b,full,4096,256);render(&c,half,4096,17);
    C(!memcmp(full,legacy,sizeof full));
    for(unsigned i=0;i<4096;i++)C(fabsf(half[i]-full[i]*(64.0f/127.0f))<1e-6f);
    C(c.v[c.cur].velocity==64);
    unsigned old=c.cur;breaks_velocity(&c,127);breaks_pin(&c,1);breaks_step(&c,1,0,(float)N/16.0f,1);
    C(c.v[old].velocity==64);C(c.v[c.cur].velocity==127);
    seq_setup(&a,64);seq_setup(&b,64);
    render(&a,full,4096,256);render(&b,half,4096,17);C(!memcmp(full,half,sizeof full));
    breaks_velocity(&a,1);breaks_live(&a,8,1);render(&a,legacy,256,256);
    C(a.v[a.cur].velocity==64);
    seq_setup(&a,1);seq_setup(&b,127);
    breaks_slice_on(&a,2);breaks_slice_on(&b,2);
    render(&a,legacy,4096,256);render(&b,full,4096,256);
    C(a.v[a.cur].velocity==0);C(b.v[b.cur].velocity==0);
    for(unsigned i=BRK_FADE+2;i<4096;i++)C(legacy[i]==full[i]);
    breaks_velocity(&a,-1);C(a.step_velocity==0);
    breaks_velocity(&a,300);C(a.step_velocity==127);
    printf("break velocity: %u checks, %u failures; live and fading owners independent\n",checks,failures);
    return failures?1:0;
}
