#include "compact.h"
typedef struct {
    sj6_zero_fx_channel left,right;
} sj6_zero_fx_stereo;
__attribute__((noinline))
void sj6_hp_crusher_44100_span(sj6_zero_fx_stereo *s,float *l,float *r,unsigned n)
{
    for(unsigned i=0;i<n;i++){
        l[i]=sj6_zero_crusher_44100(&s->left,sj6_hp10_44100(&s->left,l[i]));
        r[i]=sj6_zero_crusher_44100(&s->right,sj6_hp10_44100(&s->right,r[i]));
    }
}
const unsigned char abi_sj6_zero_fx_stereo[sizeof(sj6_zero_fx_stereo)]={0};
