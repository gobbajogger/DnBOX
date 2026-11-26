#include "fm1_time.h"
enum{N=128,C=8,R=64};
struct item {uint32_t time,phase,hash;};
struct out {uint32_t magic;struct item row[C];};
static volatile struct out *const out=(volatile struct out *)0x1c10000;
static uint8_t stream[4096];
static float dry[N],right[N],rev[N],dly[N];
static d9_smp_t s;
static inline uint32_t word(float v){union{float f;uint32_t u;}x={.f=v};return x.u;}
void bench_start(void){
 const uint32_t inc[C]={1,1,1,0,2,1,2,1}, frac[C]={0,0x60000000u,0xe0000000u,0x60000000u,0,0,0x40000000u,0};
 static const int len[C]={2047,2047,2047,2047,2047,81,2047,2047};
 uint32_t rng=0x12345921;
 for(unsigned i=0;i<sizeof stream;i++){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;stream[i]=rng;}
 fm1_time_init();
 for(unsigned k=0;k<C;k++){
   uint32_t total=0,h=0x811c9dc5u,p=0;
   for(unsigned j=0;j<R;j++){
    unsigned char *p0=(unsigned char *)&s;for(unsigned x=0;x<sizeof s;x++)p0[x]=0;
    for(unsigned x=0;x<N;x++)dry[x]=right[x]=rev[x]=dly[x]=0;
    s.adpcm=stream;s.len=len[k];s.inc=inc[k];s.incf=frac[k];s.playing=1;s.mute=k==7?61:256;
    d9_env_set(&s.out,0.1f);d9_env_exp(&s.out,0.0001f,1712);
    float y[N];
    d9_bus_t b={.dry=dry,.dry_r=right,.rev=rev,.dly=dly,.pl=.62f,.pr=.41f,.srev=.33f,.sdly=.12f,.y=y};
    if(j&1) {d9_render_smp(&s,&b,N);s.mute=k==7?61:256;}
    uint32_t t=fm1_ticks();
    d9_render_smp(&s,&b,N);
    total+=fm1_ticks()-t;
    p=p*31+s.pos+s.frac+s.dk+s.pred+s.idx+s.h0+s.h1+s.huff_bit;
    for(unsigned x=0;x<N;x++){h=(h^word(dry[x]))*16777619u;h=(h^word(right[x]))*16777619u;h=(h^word(rev[x]))*16777619u;h=(h^word(dly[x]))*16777619u;}
   }
   out->row[k]=(struct item){total,p,h};
 }
 out->magic=0x53413146u;
}
