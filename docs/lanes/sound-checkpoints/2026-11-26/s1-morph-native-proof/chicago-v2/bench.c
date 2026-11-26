#include <stdint.h>
#include <stddef.h>
void *memset(void *s,int v,size_t n){unsigned char *p=s;while(n--)*p++=(unsigned char)v;return s;}
void *memcpy(void *s,const void *v,size_t n){unsigned char *p=s;const unsigned char*q=v;while(n--)*p++=*q++;return s;}
#include "fm1_time.h"
#include "chicago_mp4.h"
#include "voice_gain.h"
#include "oscillator_motion.h"
extern int original_chicago_voice_span(original_voice_gain *,original_oscillators *,const original_prepared_oscillators *,original_ramp *,uint32_t *,float,original_chicago_mp4 *,uint32_t *,uint32_t *,uint32_t);
enum {n=64,loops=256};
struct out {uint32_t magic, clocks[4], hashes[4], errors;};
static volatile struct out *const out=(volatile struct out *)0x01c10000u;
void bench_start(void){
    fm1_time_init();
    unsigned err=0;
    for(unsigned mode=0;mode<4;mode++){
        uint32_t total=0,hash=2166136261u;
        original_prepared_oscillators p={0}; p.ready=1;p.controls.xmod=(mode&1u)?0.3f:0.f;
        for(unsigned i=0;i<4;i++){
            p.controls.increment[i]=13304001u+i*2405401u;
            p.controls.width[i]=.24f; p.controls.shape[i]=(uint8_t)i;
            p.correction[i]=.004f; p.triangle[i]=.15f;
        }
        for(unsigned j=0;j<loops;j++){
            original_voice_gain g={.oscillator={{.6f,.6f,0.f},{.5f,.5f,0.f},{0.f,0.f,0.f},{.4f,.4f,0.f}},.stereo={{.8f,.8f,0.f},{.6f,.6f,0.f}}};
            original_ramp ngr={.current=.09f,.target=.09f};
            original_oscillators ph={.phase={0x05afe462u+j,0x50345166u,0xa3a48900u,0xe7535221u}};
            original_chicago_mp4 f={.a={.67f,.67f,0.f},.e={-.2f,-.2f,0.f},.resonance={.2f,.2f,0.f},.prepared=1};
            if(mode&2u){f.a.target=.76f;f.a.delta=.001f;g.oscillator[1].delta=.0001f;g.oscillator[1].target=.502f;}
            uint32_t ns=12329u+17u*j,L[n],R[n];
            uint32_t start=fm1_ticks();
            int ok=original_chicago_voice_span(&g,&ph,&p,&ngr,&ns,1.f,&f,L,R,n);
            total+=fm1_ticks()-start;
            if(!ok){err++;continue;}
            for(unsigned i=0;i<n;i++){hash=(hash^L[i])*16777619u;hash=(hash^R[i])*16777619u;}
        }
        out->clocks[mode]=total;out->hashes[mode]=hash;
    }
    out->errors=err;out->magic=0x43394d50;
}
