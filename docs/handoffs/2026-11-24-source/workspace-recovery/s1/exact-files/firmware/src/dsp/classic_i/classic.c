/* Host-only translation of active classic musical Mode I inner render.
 * Caller supplies real AU-retained scalars at the existing block boundary. */
#include "classic.h"
#include <math.h>

static float wrap(float p,float n) {
    while(p<0.f)p+=n;
    while(p>=n)p-=n;
    return p;
}
static float sine(unsigned i) {
    return (float)sin(((double)i/256.+(double)i/256.)*3.141592653589793);
}
static float lfo(float phase) {
    float pos=wrap(phase,1.f)*256.f;
    unsigned i=(unsigned)pos;
    float frac=pos-(float)i;
    float end=sine((i+1u)&255u)*frac;
    return (float)fma(1.-(double)frac,sine(i),(double)end);
}
static float stereo_delay(classic_i *s,unsigned channel,float x,float ms,float fb)
{
    const float n=CLASSIC_I_DELAY_FRAMES;
    float delay=(float)(44100.*(double)ms/1000.);
    float off=wrap((float)s->write[channel]-delay,n);
    unsigned j=(unsigned)off, k=(j+1u)%CLASSIC_I_DELAY_FRAMES;
    float x0=s->ring[channel][j], x1=s->ring[channel][k], f=off-(float)j;
    float y=fmaf(1.f-f,x0,f*x1);
    s->ring[channel][s->write[channel]]=fmaf(fb,y,x);
    s->write[channel]=(s->write[channel]+1u)%CLASSIC_I_DELAY_FRAMES;
    return y;
}
void classic_i_frame(classic_i *s,float l,float r,float depth,
    float phase_increment,const float feedback[2],float mix,float out[2])
{
    const float amp=(24.f-3.518f)*.5f, center=(24.f+3.518f)*.5f;
    float m0=lfo(s->phase), m1=lfo(s->phase+.25f);
    float mst[2]={fmaf(amp*depth,m0,center),
                  fmaf(amp*depth,m1,center)};
    float phase=s->phase+phase_increment;
    s->phase=phase<1.f ? phase : phase-1.f;
    float yl=stereo_delay(s,0,l,mst[0],feedback[0]),yr=stereo_delay(s,1,r,mst[1],feedback[1]);
    out[0]=fmaf(1.f-mix,l,mix*yl);
    out[1]=fmaf(1.f-mix,r,mix*yr);
}
