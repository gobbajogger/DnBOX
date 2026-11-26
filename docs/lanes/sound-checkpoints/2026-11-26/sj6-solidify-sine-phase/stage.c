#include <math.h>
#include <stdio.h>
#include "sj6_main_4x.h"
static float analytic(float p,float step,void *unused){
    (void)step;(void)unused;
    return .5f*cosf(p*6.2831853071795864769f);
}
int main(void) {
    const double pi=3.141592653589793238462643;
    for(unsigned i=0;i<6;i++){
        float f=110.f*(1u<<i),step=f/44100.f,t=0;
        sj6_main_4x state={0};
        double rc[3]={0},rs[3]={0};
        for(unsigned j=0;j<22050;j++){
            const float w[3]={.5f*sinf(t*(float)(2*pi)),analytic(t,0,0),sj6_main_4x_sample(&state,step,analytic,0)};
            t=sj6_phase_add(t,step);
            if(j<4410)continue;
            double a=2*pi*f*j/44100.;
            for(unsigned k=0;k<3;k++){rc[k]+=w[k]*cos(a);rs[k]-=w[k]*sin(a);}
        }
        printf("%g", f);
        for(unsigned k=0;k<3;k++)printf(" %.9f",atan2(rs[k],rc[k])*180/pi);
        puts("");
    }
}
