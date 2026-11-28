#define X0X_HOST 1
#include D8_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>

static uint32_t rng=0xf7642331u;
static uint32_t rnd(void){return rng=rng*1664525u+1013904223u;}
int main(void)
{
    unsigned fast=0,checks=0;
    for (int test=0;test<90000;test++) {
        d8_bank_t a={0},b;
        float x[32]={0},y[32]={0},r[32]={0},s[32]={0};
        a.rng.s1=837798u; a.rng.s2=71238u; a.rng.s3=1323u;
        bank_ratio(&a, .25f + 3.75f*(rnd()>>16)/65535.f);
        a.drift_cnt=1+(rnd()>>24);
        switch(test%5){
        case 0:a.pend_wrap=rnd()>>26;break;
        case 1:a.drift_cnt=1+(rnd()>>30);break;
        default:break;
        }
        for(int i=0;i<6;i++){
            a.dt[i]=bank_dt1(&a,i);
            a.ph[i]=rnd();
        }
        if (test%2) {
            const unsigned i=(rnd()>>29)%6;
            const uint32_t edge=(rnd()>>31)?BANK_DUTY:0u;
            const int adjacent=(int)(rnd()>>30)-2;
            a.ph[i]=edge-(5u*a.dt[i])+adjacent;
        }
        a.dt_valid=1;
        b=a;
        const int m=4;
        const int cb=test&1;
        const int took=bank_quiet_four(&b,y,s,cb);
        assert(took || !memcmp(&a,&b,sizeof a));
        if (!took) continue;
        ++fast;
        bank_block(&a,x,r,m,cb);
        assert(!memcmp(&a,&b,sizeof a));
        assert(!memcmp(x,y,sizeof x) && !memcmp(r,s,sizeof r));
    }
    d8_bank_t a={0};
    a.rng.s1=837798u; a.rng.s2=71238u; a.rng.s3=1323u;
    bank_ratio(&a,1.f);
    for(int i=0;i<6;i++){a.dt[i]=bank_dt1(&a,i);a.ph[i]=i*0x40000000u;}
    a.dt_valid=1;
    a.drift_cnt=256;
    unsigned normal_fast=0;
    float x[4],y[4];
    for(checks=0;checks<10000;++checks){
        if(bank_quiet_four(&a,x,y,1)) ++normal_fast;
        else bank_block(&a,x,y,4,1);
    }
    printf("%u/90000 bounded test windows used shortcut and matched exactly; nominal 1x tuning %.1f%% windows eligible\n",fast,100.f*normal_fast/checks);
}
