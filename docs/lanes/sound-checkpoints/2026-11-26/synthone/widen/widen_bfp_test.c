#include "widen_bfp.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc,char **argv)
{
    s1_widen_bfp8 state;
    s1_widen_bfp8_clear(&state);
    for(unsigned i=0;i<S1_W_DELAY;i++)assert(s1_widen_bfp8_next(&state,i?0.f:0.25f,1.f)==0.f);
    assert(s1_widen_bfp8_next(&state,0.f,1.f)==0.25f);
    for(unsigned i=0;i<S1_W_DELAY*4;i++)
        assert(s1_widen_bfp8_next(&state,(float)(i%5)/5.f,0.f)==(float)(i%5)/5.f);
    if(argc==3){
        FILE *in=fopen(argv[1],"rb"),*out=fopen(argv[2],"wb");
        assert(in&&out);
        s1_widen_bfp8_clear(&state);
        float v[3];
        while(fread(v,sizeof v,1,in)==1){
            assert(isfinite(v[1])&&isfinite(v[2]));
            const float r=s1_widen_bfp8_next(&state,v[1],v[2]);
            assert(fwrite(&r,sizeof r,1,out)==1);
        }
        assert(feof(in));
        fclose(in);fclose(out);
    }
    printf("PASS: %zu B per-owner incl state, delay=%u samples, exact width-off\n",sizeof state,S1_W_DELAY);
}
