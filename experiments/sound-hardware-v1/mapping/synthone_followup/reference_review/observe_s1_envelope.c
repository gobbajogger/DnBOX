/* Reference-only observer. This calls the frozen reference through its public
 * component interface. It is not candidate DSP and must not be shipped. */
#include "soundpipe.h"
#include <stdio.h>
#include <math.h>

int main(void) {
    const float settings[3][4]={{.05f,.005f,.8f,.05f},
                              {.2f,.3f,.4f,.5f},
                              {.0005f,.005f,0.f,.004f}};
    puts("case,frame,gate,output");
    for (unsigned c=0;c<3;c++) {
        sp_data context={0}; context.sr=44100;
        sp_adsr envelope; sp_adsr_init(&context,&envelope);
        envelope.atk=settings[c][0]; envelope.dec=settings[c][1];
        envelope.sus=settings[c][2]; envelope.rel=settings[c][3];
        for(unsigned frame=0;frame<88200;frame++) {
            float gate=(frame<44100)?1.f:0.f,output;
            sp_adsr_compute(&context,&envelope,&gate,&output);
            if(!isfinite(output))return 1;
            printf("%u,%u,%.0f,%.9g\n",c,frame,gate,output);
        }
    }
    return 0;
}
