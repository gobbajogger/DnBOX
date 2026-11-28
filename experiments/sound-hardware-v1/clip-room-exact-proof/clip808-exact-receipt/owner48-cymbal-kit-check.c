#include "drum808.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
void orig_drum808_init(drum808_t *);
void orig_drum808_trigger(drum808_t *,int,float);
void orig_drum808_set(drum808_t *,int,int,int);
void orig_drum808_render(drum808_t *,float *,float *,float *,int);
void orig_drum808_render_st_masked(drum808_t *,float *,float *,float *,float *,int,uint16_t);
void orig_drum808_render_st_lanes(drum808_t *,float *,float *,float *,float *,int,uint16_t,uint32_t);
void orig_drum808_metal_prepare(drum808_t *,float [4][256],int,uint16_t);
void orig_drum808_finish_rest(drum808_t *,const float [4][256],float *,float *,float *,float *,int,uint16_t);
static uint32_t seed=4553;
static uint32_t next(void){return seed=seed*1664525u+1013904223u;}
int main(void)
{
    drum808_t a,b;
    unsigned frames=0, spans=0;
    for(int i=0;i<20;i++){
        orig_drum808_init(&a);drum808_init(&b);
        for(int k=0;k<3200;k++){
            float x[4][256]={0},y[4][256]={0};
            int n=(next()>>16)%257,tr=(next()>>16)%D8_NUM,mode=(next()>>16)%4;
            uint16_t mask=k%7==0?(next()>>16)&0x7ff:0x7ff;
            if(k%4==0){
                float v=(next()>>24)*(1/255.0f);
                orig_drum808_trigger(&a,tr,v);drum808_trigger(&b,tr,v);
            }
            if(k%7==0){int j=(next()>>16)%drum808_nparams(tr),v=(next()>>24)&127;orig_drum808_set(&a,tr,j,v);drum808_set(&b,tr,j,v);}
            if(mode==0){orig_drum808_render(&a,x[0],x[2],x[3],n);drum808_render(&b,y[0],y[2],y[3],n);}
            if(mode==1){orig_drum808_render_st_masked(&a,x[0],x[1],x[2],x[3],n,mask);drum808_render_st_masked(&b,y[0],y[1],y[2],y[3],n,mask);}
            if(mode==2){
                orig_drum808_render_st_lanes(&a,x[0],x[1],x[2],x[3],n,mask,D8_LANES_BODY);
                drum808_render_st_lanes(&b,y[0],y[1],y[2],y[3],n,mask,D8_LANES_BODY);
                orig_drum808_render_st_lanes(&a,x[0],x[1],x[2],x[3],n,mask,D8_LANES_MID|D8_LANES_METAL);
                drum808_render_st_lanes(&b,y[0],y[1],y[2],y[3],n,mask,D8_LANES_MID|D8_LANES_METAL);
            }
            if(mode==3){
                float t[4][256]={0},u[4][256]={0};
                orig_drum808_metal_prepare(&a,t,n,mask);drum808_metal_prepare(&b,u,n,mask);
                assert(!memcmp(t,u,sizeof t));
                orig_drum808_render_st_lanes(&a,x[0],x[1],x[2],x[3],n,mask,D8_LANES_BODY);
                drum808_render_st_lanes(&b,y[0],y[1],y[2],y[3],n,mask,D8_LANES_BODY);
                orig_drum808_finish_rest(&a,t,x[0],x[1],x[2],x[3],n,mask);
                drum808_finish_rest(&b,u,y[0],y[1],y[2],y[3],n,mask);
            }
            assert(!memcmp(x,y,sizeof x));assert(!memcmp(&a,&b,sizeof a));
            frames+=n;++spans;
        }
    }
    printf("%u randomized 808 frames / %u spans: all four output buses and kit state bit-exact\n",frames,spans);
}
