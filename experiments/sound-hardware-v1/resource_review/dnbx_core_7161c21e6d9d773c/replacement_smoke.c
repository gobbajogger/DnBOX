/* Isolated adapter smoke check, not a full firmware or fidelity test. */
#include "juno6.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    juno6_t parts[2];
    juno6_chorus_t chorus[2];
    float left[2][256],right[2][256];
    double energy[2]={0,0};
    for(unsigned p=0;p<2;p++){
        juno6_init(&parts[p],44100);
        juno6_chorus_init(&chorus[p],44100);
        for(unsigned id=0;id<18;id++)for(unsigned v=0;v<3;v++)
            assert(juno6_set(&parts[p],id,.5f*v));
        for(unsigned id=0;id<16;id++)assert(juno6_set(&parts[p],id,.5f));
        assert(juno6_set(&parts[p],16,.5f));
        assert(juno6_set(&parts[p],17,0));
        assert(!juno6_set(&parts[p],18,.5f));
        assert(!juno6_set(&parts[p],0,NAN));
        for(unsigned voice=0;voice<6;voice++)assert(juno6_note_on(&parts[p],48+voice*3+p,100));
        assert(juno6_active(&parts[p]));
        for(unsigned block=0;block<2;block++){
            assert(juno6_render_stereo(&parts[p],left[p],right[p],256));
            juno6_chorus_process(&chorus[p],left[p],right[p],256);
            for(unsigned i=0;i<256;i++){
                assert(isfinite(left[p][i])&&isfinite(right[p][i]));
                energy[p]+=(double)left[p][i]*left[p][i]+(double)right[p][i]*right[p][i];
            }
        }
        assert(energy[p]>0);
        juno6_envelope_snapshot view;
        assert(juno6_read_envelope(&parts[p],&view));
        assert(view.active&&view.shared_amp_filter==0);
    }
    juno6_t untouched=parts[1];
    assert(juno6_set(&parts[0],17,1));
    juno6_note_off(&parts[0],48);
    assert(juno6_set(&parts[0],17,0));
    assert(memcmp(&untouched,&parts[1],sizeof untouched)==0);
    for(unsigned p=0;p<2;p++){
        juno6_all_off(&parts[p]);
        assert(!juno6_active(&parts[p]));
        assert(juno6_render_stereo(&parts[p],left[p],right[p],256));
        for(unsigned i=0;i<256;i++)assert(left[p][i]==0&&right[p][i]==0);
    }
    printf("PASS two instances, 6 voices and 2 layers each, 512 frames each; energy %.12g %.12g; facade %zu, chorus %zu bytes on host\n",energy[0],energy[1],sizeof(juno6_t),sizeof(juno6_chorus_t));
    return 0;
}
