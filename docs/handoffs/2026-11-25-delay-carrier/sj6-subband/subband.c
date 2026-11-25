#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "filters.inc"
enum { N=321, CELLS=696, STRIDE=16, FULL=11025, K=(FULL-(N-1))/STRIDE, FRACTION=(FULL-(N-1))%STRIDE };
_Static_assert(K+(N-1)/STRIDE<CELLS,"history must contain synthesis taps");
typedef struct {
    int16_t pcm[N][2];
    int8_t bands[CELLS][2][2];
    unsigned input,write,phase;
    float dry,fb;
} History;
_Static_assert(sizeof(History)<=4096,"same arena");
static int16_t pcm(float v) {
    return v<=-1.f?-32767:v>=1.f?32767:(int16_t)(v*32767.f);
}
static int8_t pack(float v) {
    float a=v<0?-v:v;
    unsigned high=127,low=0;
    while(high>low+1){
        unsigned m=(high+low)/2;
        if(compand[m]>a)high=m;else low=m;
    }
    unsigned u=(a-compand[low]<compand[high]-a)?low:high;
    return v<0?-(int)u:(int)u;
}
static float unpack(int v){return v>=0?compand[v]:-compand[-v];}
unsigned subband_size(void){return sizeof(History);}
void subband(unsigned frames,float fb,float dry,const float* il,const float *ir,float *l,float*r) {
    History s={.dry=dry,.fb=fb};
    unsigned t=0;
    for(;t<frames;t++) {
        float e[2]={0,0};
        if(s.phase==0){
            if(++s.write==CELLS)s.write=0;
        }
        unsigned synth_phase=(s.phase+STRIDE-FRACTION)&(STRIDE-1);
        unsigned k=s.write+CELLS-K-(s.phase<FRACTION);
        if(k>=CELLS)k-=CELLS;
        for(unsigned j=synth_phase;j<N;j+=STRIDE) {
            for(unsigned c=0;c<2;c++){
                e[c]+=filter0[j]*unpack(s.bands[k][c][0])+filter1[j]*unpack(s.bands[k][c][1]);
            }
            if(!k)k=CELLS;
            k--;
        }
        e[0]*=STRIDE; e[1]*=STRIDE;
        s.pcm[s.input][0]=pcm(il[t]+fb*e[0]);s.pcm[s.input][1]=pcm(ir[t]+fb*e[1]);
        if(s.phase==0){
            unsigned at=s.input;
            float per[2][2]={{0}};
            for(unsigned j=0;j<N;j++){
                for(unsigned c=0;c<2;c++){
                    per[c][0]+=filter0[j]*s.pcm[at][c];
                    per[c][1]+=filter1[j]*s.pcm[at][c];
                }
                if(!at)at=N;
                at--;
            }
            for(unsigned c=0;c<2;c++)for(unsigned b=0;b<2;b++)s.bands[s.write][c][b]=pack(per[c][b]*(1.f/32767.f));
        }
        l[t]=dry*il[t]+(1-dry)*e[0];r[t]=dry*ir[t]+(1-dry)*e[1];
        if(++s.input==N)s.input=0;
        s.phase=(s.phase+1)&15;
    }
}
