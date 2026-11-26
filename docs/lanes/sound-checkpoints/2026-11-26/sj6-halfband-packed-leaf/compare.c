#include "sj6_main_4x.h"
#include "halfband.h"
void probe(float *src,float *dst,float *witness,unsigned n){
    sj6_main_4x s={0}; compact_halfband c={0};
    for(unsigned i=0;i<n;i++,src+=4){
        float a=sj6_hb(src[0],src[1],s.section[0]), b=sj6_hb(src[2],src[3],s.section[0]);
        witness[i]=sj6_hb(a,b,s.section[1]);dst[i]=compact_decimate(&c,src);
    }
}
unsigned footprint(void){return sizeof(compact_halfband);}
