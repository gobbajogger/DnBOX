/* Offline Build9 noise witness only. Admission supplies actual slot seed.
 * Each Noise voice advances once per owner draw, duplicated only after the
 * original mono carrier boundary. No guessed fader/amp/DFI or project write. */
#include <stdint.h>
#include <math.h>
#include <string.h>
typedef struct {
    float z1[3],z2[3],p0,p1;
    double frac;
    uint32_t seed;
    unsigned initial;
} noise;
static const uint32_t words[3][5]={
    {0x3f807dab,0xbffc18e2,0x3f776517,0xbffc1d1c,0x3f7857fa},
    {0x3f0b3e16,0xbe7f520b,0x3de8c715,0xbf6a6798,0x3ea5d41c},
    {0x3f2db1ba,0x3fadb1ba,0x3f2db1ba,0x3facd5bf,0x3eba36d2}
};
static float f(uint32_t w){float x;memcpy(&x,&w,4);return x;}
static uint32_t mix_seed(uint32_t x){
    if(!x)return UINT32_C(0x6d2b79f5);
    x=(x^(x>>16))*UINT32_C(0x7feb352d);
    x=(x^(x>>15))*UINT32_C(0x846ca68b);
    return x^(x>>16);
}
uint32_t noise_rearm_seed(uint32_t x){
    return mix_seed(x+UINT32_C(0x9e3779b9));
}
uint32_t noise_cold_slot_seed(uint32_t allocation_ordinal){
    /* Actual process-wide allocation ordinal, never part/note number. */
    return mix_seed(UINT32_C(0x6d2b79f5)+allocation_ordinal*UINT32_C(0x9e3779b9));
}
float noise_bipolar24(uint32_t n){
    if(n>UINT32_C(0xffffff))return NAN;
    const float k=(float)n;
    return (k*0x1p-23f-1.f)+k*0x1p-46f;
}
void noise_bind_seed(noise *s,uint32_t reached_seed){
    memset(s,0,sizeof *s);
    s->seed=reached_seed;
    s->initial=1;
}
float noise_raw(uint32_t *state){
    uint32_t x=*state;
    x^=x<<13; x^=x>>17; x^=x<<5;
    *state=x;
    /* k*2^-23−1 and k*2^-46 are individually exactly representable.
     * Adding them once reproduces the final original FMA rounding. */
    return noise_bipolar24(x>>8);
}
static float draw(noise *s,unsigned colored){
    float x=noise_raw(&s->seed);
    if(!colored)return x;
    for(unsigned i=0;i<3;i++){
        const float y=fmaf(f(words[i][0]),x,s->z1[i]);
        const float z=fmaf(f(words[i][1]),x,-(y*f(words[i][3])));
        s->z1[i]=s->z2[i]+z;
        s->z2[i]=fmaf(f(words[i][2]),x,-(y*f(words[i][4])));
        x=y;
    }
    return x*f(UINT32_C(0x3ee0c0aa));
}
float noise_next(noise *s,unsigned rate,unsigned colored){
    if(!colored||rate==44100)return draw(s,colored);
    if(rate!=48000)return NAN; /* no other config admitted */
    if(s->initial){
        s->p0=draw(s,1);s->p1=draw(s,1);s->initial=0;
    }
    while(s->frac>=1.){
        s->p0=s->p1;s->p1=draw(s,1);s->frac-=1.;
    }
    const float out=fmaf((float)s->frac,s->p1-s->p0,s->p0);
    s->frac+=44100./48000.;
    return out;
}
