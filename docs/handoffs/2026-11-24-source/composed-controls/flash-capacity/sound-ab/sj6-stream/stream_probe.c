/* Live-causal experiment: 4-bit IMA history with restart checkpoints.
 * Not a firmware patch: this host probe tests the first worthwhile signals,
 * actual recirculation, and data/working-set accounting. */
#include <stdint.h>
#include <stddef.h>
#ifndef STREAM_NATIVE_LAYOUT_ONLY
#include <math.h>
#include <stdlib.h>
#endif
#define N 96
#define B 39
#define FRAME (B*N)
#define BLOCK_BYTES (N+6)
#ifndef PRE
#define PRE 19
#endif
#define TAPS 8
#define PENDING ((PRE-1)/6+1)
#define STRIDE 6
#define PI 3.14159265358979323846

typedef struct { int16_t val; uint8_t idx; } Ima;
static const int steps[89]={
7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,
107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,
724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,
3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,
13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
static int clip(int x,int lo,int hi){return x<lo?lo:x>hi?hi:x;}
static int decode(Ima *s,int code) {
    static const int delta[8]={-1,-1,-1,-1,2,4,6,8};
    const int step=steps[s->idx], a=code&7;
    int v=(step>>3)+(a&4?step:0)+(a&2?step>>1:0)+(a&1?step>>2:0);
    s->val=clip(s->val+(code&8?-v:v),-32768,32767);
    s->idx=clip(s->idx+delta[a],0,88);
    return s->val;
}
static int encode(Ima *s,int v){
    int step=steps[s->idx],d=v-s->val,n=0;
    if(d<0){n=8;d=-d;}
    if(d>=step){n|=4;d-=step;}
    if(d>=step>>1){n|=2;d-=step>>1;}
    if(d>=step>>2)n|=1;
    decode(s,n);return n;
}
typedef struct {
    uint8_t ring[B][BLOCK_BYTES];
    int32_t next[PENDING][2];
    int16_t cached[TAPS][2];
    Ima enc[2],dec[2];
    int write,read,frame,cache_pos,worst,del_pos;
    float dry,fb;
} Stream;
#ifdef STREAM_NATIVE_LAYOUT_ONLY
static const float kernel_native[STRIDE][TAPS]=
#include "interp-table.inc"
;
uint8_t stream_layout[sizeof(Stream)];
typedef struct { Stream s; } Host;
#else
typedef struct {
    Stream s;
    float kernel[STRIDE][TAPS];
} Host;
#endif
static int16_t pcm(float x){if(!(x<1.f))return 32767;
    if(x<=-1.f)return -32768;
    return (int16_t)clip((int)(x*32768.f+(x<0?-.5f:.5f)),-32768,32767);}
static void wr(Stream *s,int16_t l,int16_t r){
    int n=s->write%N;
    uint8_t *p=s->ring[(s->write/N)%B];
    if(!n)for(int c=0;c<2;c++){
        int v=s->enc[c].val;
        p[c*3]=(uint8_t)v;p[c*3+1]=(uint8_t)(v>>8);p[c*3+2]=(uint8_t)s->enc[c].idx;
    }
    p[6+n]=(uint8_t)(encode(s->enc,l)|(encode(s->enc+1,r)<<4));s->write++;
}
static void seek(Stream *s,int n){
    s->read=n;
    const uint8_t *p=s->ring[(n/N)%B];
    for(int c=0;c<2;c++){
        s->dec[c].val=(int16_t)(p[c*3]|(p[c*3+1]<<8));
        s->dec[c].idx=p[c*3+2];
    }
}
static void wet_at(Host *h,int base,int frac,float *l,float *r){
    Stream *s=&h->s;
    int last=base+TAPS/2, work=0;
    if(last>=s->write){*l=*r=0;return;}
    if(base<s->read-TAPS+TAPS/2){  /* time tweak backwards: rebuild from a restart */
        s->cache_pos=0;
        for(int i=0;i<TAPS;i++)s->cached[i][0]=s->cached[i][1]=0;
        if(base>=TAPS/2-1) seek(s,(base-(TAPS/2-1))/N*N);
        else s->read=0;
    }
    if(last-s->read>=2*N && last>=TAPS){
        s->cache_pos=0;
        seek(s,(last-(TAPS-1))/N*N);
    }
    while(s->read<=last){
        const int k=s->read;
        if(! (k%N))seek(s,k);
        uint8_t code=s->ring[(k/N)%B][6+k%N];
        s->cached[s->cache_pos][0]=(int16_t)decode(s->dec,code&15);
        s->cached[s->cache_pos][1]=(int16_t)decode(s->dec+1,code>>4);
        s->cache_pos=(s->cache_pos+1)%TAPS;
        s->read++;work++;
    }
    if(work>s->worst)s->worst=work;
    float a=0,b=0;
    const int start=base-TAPS/2+1;
    for(int j=0;j<TAPS;j++)if(start+j>=0){
        const int k=(s->cache_pos+j+TAPS-s->read+(last+1))%TAPS;
#ifdef STREAM_NATIVE_LAYOUT_ONLY
        const float w=kernel_native[frac][j];
#else
        const float w=h->kernel[frac][j];
#endif
        a+=w*s->cached[k][0];b+=w*s->cached[k][1];
    }
    *l=a*(1.f/32768.f);*r=b*(1.f/32768.f);
}

#ifndef STREAM_NATIVE_LAYOUT_ONLY
static double sinc(double x){return fabs(x)<1e-10?1.:sin(PI*x)/(PI*x);}
void *stream_new(void) {
    Host *h=calloc(1,sizeof(*h));
    if(!h)return NULL;
    for(int i=0;i<STRIDE;i++){
        double f=(double)i/STRIDE,sum=0;
        for(int x=0;x<TAPS;x++){
            double d=x-(TAPS/2-1)-f;
            double v=sinc(d)*sinc(d/(TAPS/2));
            sum+=v;h->kernel[i][x]=(float)v;
        }
        for(int x=0;x<TAPS;x++)h->kernel[i][x]/=(float)sum;
    }
    return h;
}
void stream_free(void *p){free(p);}
#endif

int stream_params(Host *h, int delay, float fb, float dry){
    if(delay < PRE+STRIDE*TAPS || delay > (FRAME-N-2)*STRIDE || !(fb>=0&&fb<=1) ||!(dry>=0&&dry<=1))return 0;
    h->s.del_pos=delay;h->s.fb=fb;h->s.dry=dry;return 1;
}
void stream_process(Host *h,size_t n,const float *l,const float *r,float *ol,float *orr,
                    const int16_t coeff[PRE]){
    Stream *s=&h->s;
    for(size_t i=0;i<n;i++){
        const int tm=s->frame-s->del_pos+PRE/2;
        int base=tm/STRIDE,frac=tm%STRIDE;
        if(frac<0){frac+=STRIDE;base--;}
        float dl=0,dr=0;
        if(tm>=-TAPS*STRIDE&&s->del_pos)wet_at(h,base,frac,&dl,&dr);
        const float dry=s->dry,wet=1.f-dry;
        ol[i]=dry*l[i]+wet*dl;orr[i]=dry*r[i]+wet*dr;
        const int16_t vl=pcm(l[i]+s->fb*dl),vr=pcm(r[i]+s->fb*dr);
        const int phase=s->frame%STRIDE;
        const int slot=(s->frame/STRIDE)%PENDING;
        int bin=(slot+(phase!=0))%PENDING;
        for(int k=(STRIDE-phase)%STRIDE;k<PRE;k+=STRIDE){
            s->next[bin][0]+=(int32_t)vl*coeff[k];
            s->next[bin][1]+=(int32_t)vr*coeff[k];
            if(++bin==PENDING)bin=0;
        }
        if(!phase){
            wr(s,(int16_t)clip(s->next[slot][0]>>15,-32768,32767),
                 (int16_t)clip(s->next[slot][1]>>15,-32768,32767));
            s->next[slot][0]=s->next[slot][1]=0;
        }
        /* Rebase before signed counters can overflow. Preserves the exact
         * phase of the IMA ring and of the queued FIR output bins. */
        enum { REBASE=FRAME*PENDING*STRIDE };
        if(++s->frame==2*REBASE){
            s->frame-=REBASE;
            s->write-=REBASE/STRIDE;s->read-=REBASE/STRIDE;
        }
    }
}
size_t stream_state(void){return sizeof(Stream);}
size_t stream_fir_taps(void){return PRE;}
size_t stream_ring(void){return B*BLOCK_BYTES;}
#ifndef STREAM_NATIVE_LAYOUT_ONLY
size_t stream_init_coeff_bytes(void){return sizeof(((Host*)0)->kernel);}
#endif
unsigned stream_worst(Host *h){return h->s.worst;}
