#include "history.h"
#include "fastmath.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static float peak(float a,float b){return a>b?a:b;}
static float old(float x)
{
    float v=x*32767.f;
    return (int16_t)(v>32767.f?32767.f:v< -32768.f?-32768.f:v)/32768.f;
}
static void one(unsigned kind,const char *raw)
{
    const int n=65536; float f[256]={0},o[256]={0}; int16_t c[256]={0}; blockmeta meta={0};
    double p=0,e0=0,e1=0, tail=0,t0=0,t1=0; float mx=0;unsigned null=0;
    FILE *in=raw?fopen(raw,"rb"):NULL; if(raw&&!in)abort();
    uint32_t phase=0; unsigned rewrites=0,lifts=0,max_group=0,group=0;
    for(int i=0;i<n;i++){
        const float a=2*3.141592653589793*(double)i/44100;
        float x=0;
        if(in){ if(fread(&x,sizeof x,1,in)!=1) break; }
        else if(kind==0) x=.018f*sinf(220*a);
        else if(kind==1) x=.40f*(sinf(220*a)+sinf(174.614*a)+sinf(195.998*a)+sinf(261.626*a)+sinf(329.628*a)+sinf(391.995*a));
        else if(kind==2) x=6*.404f*sinf(220*a);
        else x=4.5f*expf(-i*.00027f)*sinf(220*a);
        mx=peak(mx,fabsf(x));
        /* Same signed tapped transform and fractional delay as stock Classic I,
         * 1.7–3.4 ms at .8 Hz, 0.5 mix; ring advances after interpolation. */
        const float sr=44100.f, min_delay=sr*.0017f, max_delay=fm_minf(255,sr*.0054f);
        const float depth=3.4f*.001f-.0017f;
        const float angle=(float)phase/4294967296.f * FM_TWO_PI;
        const float sweep=.5f*fm_sinf(angle);
        float d=sr*(.0017f+depth*(.5f+sweep));
        d=d<min_delay?min_delay:d>max_delay?max_delay:d;
        phase+=(uint32_t)(.8f*4294967296.f/sr);
        unsigned a0=(i-(unsigned)d)&255,a1=(a0-1)&255;
        float u=d-(unsigned)d;
        #define DELAY(a,b) (.5f*((a)*(1.f-u)+(b)*u))
        float wet=DELAY(f[a0],f[a1]), wo=DELAY(o[a0],o[a1]), wc=DELAY(blockread(c,&meta,i&255,a0),blockread(c,&meta,i&255,a1));
        p+=(double)wet*wet;e0+=(double)(wet-wo)*(wet-wo);e1+=(double)(wet-wc)*(wet-wc);null+=wc!=wo;
        if(i>=(in?10500:20000)){tail+=(double)wet*wet;t0+=(double)(wet-wo)*(wet-wo);t1+=(double)(wet-wc)*(wet-wc);}
        f[i&255]=x; o[i&255]=old(x); if (!(i&31)) { if(group>max_group)max_group=group;group=0; }
        const unsigned cost=blockwrite(c,&meta,i&255,x,&lifts); rewrites+=cost;group+=cost;
    }
    if(in)fclose(in);
    if(group>max_group)max_group=group;
    printf("%s%u inputPeak=%.7f wet_vs_float_stock=%.3f block=%.3f changed_vs_stock=%u release_or_late stock=%.3f block=%.3f dBr rewrites=%u max_per_group=%u exponent_lifts=%u\n",in?"actual ":"fixture ",kind,mx,10*log10(e0/p),10*log10(e1/p),null,10*log10(t0/tail),10*log10(t1/tail),rewrites,max_group,lifts);
}
int main(int argc,char **argv)
{
    assert(sizeof(int16_t[256])==512);
    int16_t w[256]={0};blockmeta m={0};unsigned lifts=0;
    /* Exact low values over multiple wraps, including both signs and near zero. */
    for(int i=0;i<65536;i++){
        const float x=((i%8191)-4095+(i%2?.25f:-.25f))/32767.f;
        blockwrite(w,&m,i&255,x,&lifts);
        assert(w[i&255]==(int)(old(x)*32768.f));
    }
    assert(!lifts);
    /* For every age 0..255, verify exponent selection and absence of old-
       suffix value mutation, starting from diverse per-group exponents.
       In this test represent old/new words exactly, each chunk by powers of two. */
    unsigned e[256]={0};
    for(unsigned k=0;k<8192;k++){
        const unsigned pos=k&255;
        for(unsigned i=0;i<256;i++)
            assert(blockread(w,&m,pos,i)==((int32_t)w[i]*(1<<e[i]))/32768.f);
        unsigned pe=(k/32+k%32)%8u;
        const float x=(k&1)?(float)(1u<<pe):-(float)(1u<<pe);
        int16_t previous[256]; memcpy(previous,w,sizeof w);
        blockwrite(w,&m,pos,x,&lifts);
        for(unsigned i=0;i<256;i++){
            if(i/32==pos/32&&i<=pos)e[i]=m.exp[pos/32];
            else assert(previous[i]==w[i]);
        }
    }
    memset(w,0,sizeof w);memset(&m,0,sizeof m);
    unsigned total=0, high=0;
    for(unsigned p=0;p<32;p++){
        const unsigned inc=blockwrite(w,&m,p,p<25?.25f:1.25f*(1u<<(p-25)),&lifts);
        total+=inc; if(inc>high)high=inc;
    }
    assert(total==196 && high==31 && m.exp[0]==MAX_EXP);
    printf("adversarial bound reached: one 32-group %u previous-slot rewrites, %u in a single frame; zero-age and age255 wraps validated\n",total,high);
    for(unsigned k=0;k<4;k++)one(k,NULL);
    for(int k=1;k<argc;k++)one(k-1,argv[k]);
}
