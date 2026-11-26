/* Independent host-only checks of standard HP and DC-blocker transfer laws. */
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    double hp_x1, hp_x2, hp_y1, hp_y2, dc_x1, dc_y1;
} leaf64_t;
typedef struct {
    float hp_x1, hp_x2, hp_y1, hp_y2, dc_x1, dc_y1;
} leaf32_t;

void sj6_zero_leaf_reference(unsigned stage, const float *x, float *y, size_t n) {
    /* Apple actual fixture pin is 44.1 kHz, 10 Hz and 0 dB handle resonance. */
    const double omega = 6.283185307179586476925286766559 * (10. / 44100.);
    const double c = cos(omega), alpha = sin(omega) * 0.5, inv = 1. / (1. + alpha);
    const double b0 = (1.+c)*0.5*inv, b1 = -2.*b0, a1 = -2.*c*inv, a2 = (1.-alpha)*inv;
    leaf64_t s = {0};
    for(size_t i=0;i<n;++i) {
        const double a=x[i];
        if(stage==0) {
            const double v=b0*a+b1*s.hp_x1+b0*s.hp_x2-a1*s.hp_y1-a2*s.hp_y2;
            s.hp_x2=s.hp_x1; s.hp_x1=a; s.hp_y2=s.hp_y1; s.hp_y1=v;
            y[i]=(float)v;
        } else {
            const double v=(a-s.dc_x1)+.995*s.dc_y1;
            s.dc_x1=a; s.dc_y1=v;
            y[i]=(float)v;
        }
    }
}
void sj6_zero_leaf_float(unsigned stage, const float *x, float *y, size_t n) {
    const double omega = 6.283185307179586476925286766559 * (10. / 44100.);
    const double c = cos(omega), alpha = sin(omega) * 0.5, inv = 1. / (1. + alpha);
    const float b0=(float)((1.+c)*0.5*inv), b1=-2.f*b0, a1=(float)(-2.*c*inv), a2=(float)((1.-alpha)*inv);
    leaf32_t s={0};
    for(size_t i=0;i<n;++i) {
        const float a=x[i];
        if(stage==0) {
            const float v=b0*a+b1*s.hp_x1+b0*s.hp_x2-a1*s.hp_y1-a2*s.hp_y2;
            s.hp_x2=s.hp_x1; s.hp_x1=a; s.hp_y2=s.hp_y1; s.hp_y1=v;
            y[i]=v;
        } else {
            const float v=(a-s.dc_x1)+.995f*s.dc_y1;
            s.dc_x1=a; s.dc_y1=v;
            y[i]=v;
        }
    }
}

/* Same HP poles; evolve increments rather than summing near-cancelled
 * polynomial terms near DC. These constants describe the pinned profile. */
void sj6_zero_leaf_stable(unsigned stage, const float *x, float *y, size_t n) {
    if(stage!=0) {sj6_zero_leaf_float(stage,x,y,n);return;}
    const double w=6.283185307179586476925286766559*(10./44100.);
    const double c=cos(w), alpha=0.5*sin(w), inv=1./(1.+alpha);
    const float gain=(float)((1.+c)*.5*inv), decay=(float)((1.-alpha)*inv);
    const float stiffness=(float)(2.*(c-1.)*inv);
    float x1=0, dx1=0, y1=0, dy1=0;
    for(size_t i=0;i<n;++i) {
        const float dx=x[i]-x1;
        const float dy=gain*(dx-dx1)+stiffness*y1+decay*dy1;
        y1+=dy;
        y[i]=y1;
        x1=x[i]; dx1=dx; dy1=dy;
    }
}

#include "compact.h"
void sj6_zero_stream(unsigned stage, const float *x, float *y, size_t n, unsigned split) {
    sj6_zero_fx_channel s={0};
    const unsigned cuts[]={1,3,128,511,17,256,65};
    for(size_t i=0,j=0;i<n;++j) {
        size_t end=split?i+cuts[j%7]:n;
        if(end>n)end=n;
        for(;i<end;++i) {
            float v=x[i];
            if(stage==0 || stage==2)v=sj6_hp10_44100(&s,v);
            if(stage==1 || stage==2)v=sj6_zero_crusher_44100(&s,v);
            y[i]=v;
        }
    }
}
