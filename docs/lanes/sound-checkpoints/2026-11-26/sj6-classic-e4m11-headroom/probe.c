#include "history.h"
#define FM_TWO_PI 6.2831853071795864769f
static float fm_minf(float a,float b){return a<b?a:b;}
#define fm_sinf sinf
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static float peak(float a,float b){return a>b?a:b;}
static float old(float x)
{
    float v=x*32767.f;
    return (int16_t)(v>32767.f?32767.f:v< -32768.f?-32768.f:v)/32768.f;
}
static void one(unsigned kind,const char *raw)
{
    const int n=65536; float f[256]={0},o[256]={0}; uint16_t c[256]={0};
    double p=0,e0=0,e1=0, tail=0,t0=0,t1=0; float mx=0;unsigned null=0;
    FILE *in=raw?fopen(raw,"rb"):NULL; if(raw&&!in)abort();
    uint32_t phase=0;
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
        float wet=DELAY(f[a0],f[a1]), wo=DELAY(o[a0],o[a1]), wc=DELAY(sj6_hist_decode(c[a0]),sj6_hist_decode(c[a1]));
        p+=(double)wet*wet;e0+=(double)(wet-wo)*(wet-wo);e1+=(double)(wet-wc)*(wet-wc);null+=wc!=wo;
        if(i>=(in?10500:20000)){tail+=(double)wet*wet;t0+=(double)(wet-wo)*(wet-wo);t1+=(double)(wet-wc)*(wet-wc);}
        f[i&255]=x; o[i&255]=old(x); c[i&255]=sj6_hist_encode(x);
    }
    if(in)fclose(in);
    printf("%s%u inputPeak=%.7f wet_vs_float_stock=%.3f compand=%.3f changed_vs_stock=%u release_or_late stock=%.3f compand=%.3f dBr\n",in?"actual ":"fixture ",kind,mx,10*log10(e0/p),10*log10(e1/p),null,10*log10(t0/tail),10*log10(t1/tail));
}
int main(int argc,char **argv)
{
    assert(sizeof(uint16_t[256])==512);
    for(unsigned v=0;v<32768u;++v){
       float p=sj6_hist_decode((uint16_t)v);
       assert(sj6_hist_encode(p)==v);
       assert(sj6_hist_encode(-p)==(v|32768u));
       if(v) {
          float prev=sj6_hist_decode((uint16_t)(v-1));
          assert(p>prev);
          float mid=(p+prev)*0.5f;
          assert(sj6_hist_encode(mid)==((v&1u) ? v-1u:v));
       }
    }
    for(unsigned k=0;k<4;k++)one(k,NULL);
    for(int k=1;k<argc;k++)one(k-1,argv[k]);
}
