/* SPDX-License-Identifier: GPL-3.0-only */
#include "exact-morph.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* Original Synth One phase/interpolation topology, Float32 tables, host only.
 * Only initialized and consumed on the single renderer thread. */
static float wave[52][4097];
static int loaded;
int exact_morph_load(void) {
    if(loaded)return 1;
    const char *path=getenv("S1_EXACT_MORPH_WAVES");
    FILE *f=path?fopen(path,"rb"):NULL;
    if(!f)return 0;
    const int ok=fread(wave,sizeof(float),52u*4097u,f)==52u*4097u&&fgetc(f)==EOF;
    if(fclose(f)||!ok)return 0;
    for(unsigned i=0;i<52;++i)for(unsigned j=0;j<4097;++j)
        if(!isfinite(wave[i][j]))return 0;
    loaded=1;return 1;
}

float exact_morph_sample(uint32_t *phase,float freq,float wtpos,int bandlimited) {
    static const float limits[13]={0.f,8.178783416748047f,16.357566833496094f,32.71513366699219f,
        65.43026733398438f,131.25f,262.5f,525.f,1050.f,2205.f,4410.f,11025.f,22050.f};
    if(!loaded)abort();
    unsigned band=0;
    if(bandlimited)for(unsigned i=1;i<13;++i)if(freq<=limits[i]){band=i;break;}
    if(wtpos>1.f)wtpos-=(int)wtpos;
    const float wave_pos=(float)(band*4u)+wtpos*3.f;
    const unsigned row=(unsigned)wave_pos;
    const float mix=wave_pos-(float)row;
    const unsigned row2=row>=band*4u+3u?row:row+1u;
    const unsigned p=*phase>>12,q=(p+1u)&4095u;
    const float frac=(float)(*phase&4095u)*(1.f/4096.f);
    const float v1=(1.f-mix)*wave[row][p]+mix*wave[row2][p];
    const float v2=(1.f-mix)*wave[row][q]+mix*wave[row2][q];
    *phase=(*phase+(uint32_t)lrintf(freq*(16777216.f/44100.f)))&0xffffffu;
    return v1+(v2-v1)*frac;
}
