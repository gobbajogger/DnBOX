/* Independently authored diagnostic. Three preserved signal histories:
 * left 2T, right 2T, and right direct+cascade T. No music/preset knowledge. */
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
enum {FS=44100, N=128, HEAD=1960, H=192, K=8};
typedef struct {uint16_t anchor,step; unsigned char code[];} Block;
_Static_assert(sizeof(Block)==4,"anchor sizing");
typedef struct {
    unsigned cap,w,cache,have,at;
    float p,delta,cstep,previous,sum,pre_input,slope,decoded_slope,trend,dtrend;
    union { float decode[N]; float unused[N]; } v;
#ifdef S1_ABLATION
    float pre[H];
#endif
    union {uint16_t *f; unsigned char *a; float *diagnostic;} mem;
} Line;
static Line ln[3];
static float target,limit;
#ifdef S1_ABLATION
static float coeff[H];
#else
_Static_assert(sizeof(ln)+128u<=HEAD,"ring scratch outside reservation");
#endif
static unsigned stride,pos,mode,shape,analysis_taps,writeclamp,bits=4u,record=68u;
static uint64_t clips, accepted, rejects;
static uint32_t grid,grid_phase;
static float desired;
void s1_delay_cold(float t){desired=t;}
static uint16_t f16(float x)
{
    union{uint32_t u;float f;} v={.f=x};
    int e=(int)((v.u>>23)&255)-112;
    if(e<=0){
        if(!(shape&256u))return 0;
        return (uint16_t)(((v.u>>16)&32768u)|(unsigned)lrintf(fabsf(x)*16777216.f));
    }
    if(e>=31)e=30,v.u|=0x7fffff;
    return (uint16_t)(((v.u>>16)&32768u)|((unsigned)e<<10)|((v.u>>13)&1023u));
}
static float unhalf(uint16_t x)
{
    if(!(x&32767u))return 0.f;
    if(!(x&0x7c00u))return (float)(x&1023u)*((x&32768u)?-0x1p-24f:0x1p-24f);
    union{uint32_t u;float f;}v={((uint32_t)(x&32768u)<<16)|(((uint32_t)(x&32767u)+0x1c000u)<<13)};
    return v.f;
}
static float floor_step(void) {return (shape&256u)?0x1p-22f:.0000611f;}
static float adapt(float step,unsigned c)
{
    static const float ratio[]={.78f,.86f,.92f,.97f,1.04f,1.10f,1.21f,1.45f};
    unsigned mag=c&((1u<<(bits-1u))-1u);
    return fminf(.25f,fmaxf(floor_step(),step*ratio[mag>>(bits-4u)]));
}
static float mag_level(unsigned k,unsigned max)
{
    if(!(shape&16384u))return (float)k;
    /* Four-bit residuals use a full-range μ-law codebook. The decoder's
     * step adaptation is unchanged and callback-independent. */
    if(max==7u){static const float levels[]={0,.25f,.55f,1.f,1.7f,2.8f,4.5f,7.f};return levels[k];}
    /* Finer depths retain their natural linear quantizers. */
    return (float)k;
}
static unsigned quantized(float x,unsigned max)
{
    if(!(shape&16384u)||max!=7u)return (unsigned)fminf((float)max,floorf(x+.5f));
    unsigned k=0;
    for(;k<max;k++)if(x<.5f*(mag_level(k,max)+mag_level(k+1,max)))break;
    return k;
}
static float delta(float a,float s,unsigned c){unsigned sign=1u<<(bits-1u);float v=mag_level(c&(sign-1u),sign-1u)*s;return c&sign?a-v:a+v;}
static unsigned slot_read(const Block*p,unsigned j)
{
    unsigned pos=j*bits, b=pos/8u,shift=pos%8u;
    unsigned v=p->code[b];
    if(shift+bits>8)v|=(unsigned)p->code[b+1u]<<8;
    return (v>>shift)&((1u<<bits)-1u);
}
static void slot_write(Block*p,unsigned j,unsigned c)
{
    unsigned pos=j*bits,b=pos/8u,shift=pos%8u, mask=((1u<<bits)-1u)<<shift;
    p->code[b]=(uint8_t)((p->code[b]&~mask)|(c<<shift));
    if(shift+bits>8)p->code[b+1u]=(uint8_t)((p->code[b+1u]&~(mask>>8))|((c<<shift)>>8));
}
static float learned(float p,float slope,float trend){
    return p+((shape&512u)?trend*slope:(shape&128u)?slope:0.f);
}
static void trend_step(float *slope,float *trend,float actual)
{
    if(shape&512u){
        /* Normalized LMS over decoder-visible past signal; no original or
         * preset state in the algorithm. Symmetric slope safety limit. */
        float update=(actual-(*trend)*(*slope))*(*slope)/(.0000001f+(*slope)*(*slope));
        *trend=fminf(.95f,fmaxf(-.95f,*trend+.0625f*update));
        *slope=actual;
    }else if(shape&128u)*slope=.5f*(*slope+actual);
}
static void put(Line*r,float x)
{
    unsigned i=r->w;
    if(shape&64u)r->mem.diagnostic[i]=x;
    else if(mode==1)r->mem.f[i]=f16(x);
    else {
        unsigned b=i/N,j=i%N;
        Block*p=(Block*)(r->mem.a+b*record);
        if(r->cache==b&&(j==0||j<=r->have))r->cache=~0u;
        if(!j){
            p->anchor=f16(x);p->step=f16(fmaxf(floor_step(),r->delta));
            r->p=unhalf(p->anchor);r->delta=unhalf(p->step);r->slope=r->trend=0;
        }else{
            float pred=learned(r->p,r->slope,r->trend);
            float e=x-pred;
            unsigned sign=1u<<(bits-1u);
            unsigned m=quantized(fabsf(e)/r->delta,sign-1u);
            unsigned c=m|(e<0.f?sign:0u);
            slot_write(p,j,c);
            float recovered=delta(pred,r->delta,c);
            trend_step(&r->slope,&r->trend,recovered-r->p);
            r->p=recovered;r->delta=adapt(r->delta,c);
        }
    }
    if(++r->w==r->cap)r->w=0;
}
static float get(Line*r,int off)
{
    if(off>0)off=0;
    int i=(int)r->w-1+off;
    if(i<0)i+=(int)r->cap;
    if(i<0){clips++;return 0;}
    if(shape&64u)return r->mem.diagnostic[i];
    if(mode==1)return unhalf(r->mem.f[i]);
    unsigned b=(unsigned)i/N,j=(unsigned)i%N;
    const Block*p=(const Block*)(r->mem.a+b*record);
    if(!p->step)return 0.f;
    if(r->cache!=b){
        r->cache=b;r->have=0;r->v.decode[0]=unhalf(p->anchor);
        r->cstep=unhalf(p->step);r->decoded_slope=r->dtrend=0;
    }
    if(j<=r->have)return r->v.decode[j];
    float a=r->v.decode[r->have],s=r->cstep,slope=r->decoded_slope,trend=r->dtrend;
    for(unsigned k=r->have+1;k<=j;k++){
        unsigned c=slot_read(p,k);
        float h=delta(learned(a,slope,trend),s,c);
        trend_step(&slope,&trend,h-a);
        a=h;s=adapt(s,c);r->v.decode[k]=a;
    }
    r->have=j;r->cstep=s;r->decoded_slope=slope;r->dtrend=trend;return a;
}
static double sinc(double x){return fabs(x)>1e-12?sin(3.141592653589793*x)/(3.141592653589793*x):1.0;}
static void make_filter(void)
{
#ifdef S1_ABLATION
    if(!(shape&1u)||stride<2)return;
    analysis_taps=2u*(stride*5u)+1u;
    if(analysis_taps>H)analysis_taps=H-1;
    int m=(int)analysis_taps/2;
    double sum=0;
    for(int i=-m;i<=m;i++){
        /* Principled unit DC Blackman prefilter cutoff near new Nyquist. */
        double x=(double)i/m;
        double w=.42+.5*cos(3.141592653589793*x)+.08*cos(6.283185307179586*x);
        coeff[m+i]=(float)(w*sinc((double)i/stride));sum+=coeff[m+i];
    }
    for(unsigned k=0;k<analysis_taps;k++)coeff[k]/=(float)sum;
#endif
}
static float tick(Line*r,float x,float age,float fb)
{
    float in=x+r->previous*fb;
    float delayed;
    if(shape&16u){
        const uint32_t next=grid_phase+65536u;
        if(next>=grid){
            const float a=(float)(grid-grid_phase)*(1.f/65536.f);
            const float sample=r->pre_input+a*(in-r->pre_input);
            if(shape&32u){
                r->sum+=.5f*a*(sample+r->pre_input);
                put(r,r->sum*(65536.f/(float)grid));
                r->sum=.5f*(1.f-a)*(sample+in);
            }else put(r,sample);
        }else if(shape&32u)r->sum+=.5f*(in+r->pre_input);
        r->pre_input=in;
        delayed=((float)(next>=grid?next-grid:next)/65536.f)-age;
        if(shape&32u)delayed+=(float)grid*(.5f/65536.f);
    }else{
    r->sum+=in;
#ifdef S1_ABLATION
    r->pre[r->at]=in;
#endif
    if(pos==0){
        float w=r->sum/(float)stride;
#ifdef S1_ABLATION
        if(analysis_taps){
            w=0;int j=(int)r->at;
            for(unsigned k=0;k<analysis_taps;k++){w+=coeff[k]*r->pre[j];if(--j<0)j=H-1;}
        }
#endif
        put(r,w);r->sum=0;
    }
    if(++r->at==H)r->at=0;
    delayed=(float)pos-age + (analysis_taps?(float)(analysis_taps/2):.5f*(float)(stride-1));
    }
    float t=delayed*((shape&16u)?65536.f/(float)grid:1.f/(float)stride);
    int max_age=(int)r->cap-(mode==1?0:N)-K-4;
    if(t<-(float)max_age){writeclamp++;t=-(float)max_age;}
    if(t>0)t=0;
    int k=(int)floorf(t);float u=t-k,out;
    if((shape&2u)&&stride>1){
        double acc=0,sum=0;
        for(int j=1-K;j<=K;j++){
            double x=u-j, w=sinc(x)*sinc(x/K);
            acc+=w*get(r,k+j);sum+=w;
        }
        out=(float)(acc/sum);
    }else{
        float a=get(r,k-1),b=get(r,k),c=get(r,k+1),d=get(r,k+2);
        float z=(u*u-1.f)*(1.f/6.f),y=(u+1.f)*.5f,w=y-1.f,v=3.f*z;
        y-=v;w-=z;v-=u;out=(w*a+v*b+y*c+z*d)*u+b;
    }
    return r->previous=out;
}
static void report(void)
{
    unsigned cells=ln[0].cap+ln[1].cap+ln[2].cap;
    fprintf(stderr,"scope portable original-source delay isolated policy mode=%s bits=%u stride=%u cadence=%.6f shape=%u cold_target=%.8g max_owned_t=%.8g bytes=%u taps_out=%u invalid=%llu rejected=%llu accepted=%llu\n",
        mode==1?"F16":"ADPCM",(shape&64u)?32u:mode==1?16u:bits,stride,(shape&16u)?FS*65536.0/grid:(double)FS/stride,shape,desired,limit,
        (shape&64u)?4u*cells:mode==1?2u*cells:cells/N*record,writeclamp,(unsigned long long)clips,
        (unsigned long long)rejects,(unsigned long long)accepted);
}
static void setup(float t)
{
    unsigned bytes=8768u-HEAD;
    const char*rs=getenv("S1_RESERVE"),*a=getenv("S1_DELAY_SHAPE");
    if(rs)t=fmaxf(t,(float)atof(rs));
    shape=a?(unsigned)atoi(a):0;
    unsigned count=bytes/2u;
    unsigned ns=count/5u,nl=(count-ns)/2u;
    float reach=fminf((float)(ns-K-4),.5f*(float)(nl-K-4))/FS;
    if(t<=reach){mode=1;stride=1;limit=reach;
       uint16_t*m=calloc(2u*nl+ns,sizeof(*m));assert(m);
       ln[0].mem.f=m;ln[1].mem.f=m+nl;ln[2].mem.f=m+2u*nl;
    }else{
       mode=2;count=bytes/68;unsigned bs=count/5+1,bl=(count-bs)/2;
       ns=bs*N;nl=bl*N;
       reach=fminf((float)(ns-N-K-4),.5f*(float)(nl-N-K-4))/FS;
       stride=(unsigned)ceilf(t/reach);if(stride<1)stride=1;limit=reach*stride;
       if((shape&(1024u|2048u))&&!(shape&16u)){
           for(unsigned candidate=5u;candidate<=6u;candidate++){
               unsigned bc=4u+candidate*N/8u, nc=bytes/bc;
               unsigned assume_s=nc/5u+1u, assume_l=(nc-assume_s)/2u;
               float trial=fminf((float)(assume_s*N-N-K-4),
                                 .5f*(float)(assume_l*N-N-K-4))/FS;
               if(t>trial*stride){
                   /* Alternative generalized precision/cadence policy:
                    * keep a 20 kHz representable band on full-rate cold
                    * cases; take extra-bit fractional mode only if that
                    * full-band floor still fits. No preset inspection. */
                   if(!(shape&2048u)||stride!=1||trial*FS/t<40000.f)break;
                   shape|=16u;
               }
               bits=candidate;record=bc;bl=assume_l;bs=assume_s;
               ns=bs*N;nl=bl*N;reach=trial;limit=reach*stride;
           }
       }
       unsigned char*m=calloc(2u*bl+bs,record);assert(m);
       ln[0].mem.a=m;ln[1].mem.a=m+bl*record;ln[2].mem.a=m+2u*bl*record;
    }
    if(shape&16u){
        grid=(uint32_t)ceil((double)t/reach*65536.0)+1u;
        if(grid<=65536u)shape&=~48u;
        else limit=reach*((float)grid/65536.f);
    }
    if(shape&(64u|4096u|8192u)){
        if(shape&4096u){
            shape|=64u;shape&=~48u;stride=1;
            ns=(unsigned)ceilf(FS*t)+N+32u;
            nl=(unsigned)ceilf(2.f*FS*t)+N+32u;
            limit=fminf((float)(ns-N-K-4),.5f*(float)(nl-N-K-4))/FS;
        }
        if(shape&8192u){
            /* Diagnostic cadence bound with excess memory, not allocation. */
            const char*rate=getenv("S1_DIAGNOSTIC_HZ");
            double hz=rate?atof(rate):22050.0;
            assert(hz>=250.f&&hz<FS);
            shape|=64u|16u|32u;
            grid=(uint32_t)ceil(FS*65536.0/hz);
            stride=(unsigned)ceil((double)grid/65536.0);
            ns=(unsigned)ceil(t*hz)+N+32u;nl=(unsigned)ceil(2.0*t*hz)+N+32u;
            reach=fminf((float)(ns-N-K-4),.5f*(float)(nl-N-K-4))/FS;
            limit=reach*((float)grid/65536.f);
        }
        free(ln[0].mem.a);
        float*m=calloc(2u*nl+ns,sizeof(*m));assert(m);
        ln[0].mem.diagnostic=m;ln[1].mem.diagnostic=m+nl;ln[2].mem.diagnostic=m+2u*nl;
    }

    ln[0].cap=ln[1].cap=nl;ln[2].cap=ns;
    for(unsigned i=0;i<3;i++)ln[i].cache=~0u;
    make_filter();pos=1u%stride;atexit(report);
}
void s1_delay_reset(void)
{
    /* Source reset zeros storage/index but leaves previous output alive. */
    if(!mode)return;
    for(unsigned i=0;i<3;i++){
        Line*l=ln+i;
        if(shape&64u)memset(l->mem.diagnostic,0,l->cap*sizeof(float));
        else if(mode==1)memset(l->mem.f,0,l->cap*sizeof(uint16_t));
        else memset(l->mem.a,0,(l->cap/N)*record);
        l->w=l->at=0;l->cache=~0u;l->sum=l->p=l->delta=l->pre_input=0;
#ifdef S1_ABLATION
        memset(l->pre,0,sizeof(l->pre));
#endif
    }
    pos=1u%stride;
    grid_phase=0;
}
void s1_delay_tap(float t,float fb,float l,float r,float*L,float*R)
{
    if(!mode){
       if(fabsf(l)<=1e-18f&&fabsf(r)<=1e-18f){*L=*R=0;return;}
       setup(fmaxf(t,desired));target=t;
    }
    if(!isfinite(t)||t<0.f||t>limit){t=target;rejects++;}
    else{target=t;accepted++;}
    float T=t*FS;
    *L=tick(ln,l,2.f*T,fb);
    float first=tick(ln+1,r,2.f*T,fb);
    *R=tick(ln+2,r+first,T,fb);
    if(++pos==stride)pos=0;
    if(shape&16u){grid_phase+=65536u;if(grid_phase>=grid)grid_phase-=grid;}
}
