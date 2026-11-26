#include <stdint.h>
#include "phase-round.h"
extern const float direct_wave[3][4][4096];
float direct_sample(uint32_t *phase,float freq,float wtpos,int bandlimited){
    static const float limits[]={0.f,8.178783416748047f,16.357566833496094f,32.71513366699219f,65.43026733398438f,131.25f,262.5f,525.f,1050.f,2205.f,4410.f,11025.f,22050.f};
    unsigned band=0;
    if(bandlimited && freq<=limits[12]){
        unsigned lo=1,hi=12;
        while(lo<hi){const unsigned m=(lo+hi)>>1;if(freq<=limits[m])hi=m;else lo=m+1;}
        band=lo;
    }
    unsigned index=band==7?0:band==8?1:band==12?2:3;
    if(index>=3)return 999999.f;
    const float (*table)[4096]=direct_wave[index];
    const float pos=wtpos*3.f;
    const unsigned row=(unsigned)pos;
    const float mix=pos-(float)row;
    const unsigned row2=(row==3u?3u:row+1u);
    const unsigned p=*phase>>12, q=(p+1u)&4095u;
    const float frac=(float)(*phase&4095u)*(1.f/4096.f);
    const float a=(1.f-mix)*table[row][p]+mix*table[row2][p];
    const float b=(1.f-mix)*table[row][q]+mix*table[row2][q];
    *phase=(*phase+morph_round_phase24(freq*(16777216.f/44100.f)))&0xffffffu;
    return a+(b-a)*frac;
}
