#define X0X_HOST 1
#include D8_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>
static uint32_t rng=0xe48c2231;
static uint32_t next(void){return rng=rng*1664525u+1013904223u;}
int main(void){
    unsigned samples=0, calls=0;
    for(unsigned test=0;test<64;++test){
        d8_cy_t a={0},b={0};cy_init(&a);cy_ratio(&a,.25f+3.75f*(next()>>16)/65535.f);
        cy_trigger(&a,.1f+3.f*(next()>>16)/65535.f,.1f+8.f*(next()>>16)/65535.f);b=a;
        for(unsigned step=0;step<4000;++step){
            float x[32]={0},y[32]={0},in[32];
            unsigned m=1+(next()>>27);
            for(unsigned i=0;i<m;++i)in[i]=step<1000?(float)(int32_t)next()*0x1p-31f:0;
            int n1=cy_run(&a,x,in,(int)m),n2=cy_compare(&b,y,in,(int)m);
            if(n1!=n2||memcmp(x,y,(size_t)n1*4)||memcmp(&a,&b,sizeof a)){
                fprintf(stderr,"mismatch test %u step %u m %u n %d/%d\n",test,step,m,n1,n2);return 1;
            }
            samples+=(unsigned)n1;++calls;
        }
    }
    unsigned rewinds=0;
    for(unsigned m=1;m<=CHUNK;++m) for(unsigned stop=1;stop<=m;++stop) {
        d8_cy_t a={0},b={0};
        cy_init(&a);
        a.active=1; a.quiet=401-stop;
        /* Gates remain zero, so positive input later in the speculative
         * front still cannot extend the quiet life of this voice. */
        b=a;
        float x[CHUNK]={0},y[CHUNK]={0},in[CHUNK];
        for(unsigned i=0;i<m;++i)in[i]=(float)(int32_t)next()*0x1p-31f;
        int n1=cy_run(&a,x,in,(int)m),n2=cy_compare(&b,y,in,(int)m);
        assert(n1==(int)stop && n2==n1);
        assert(!memcmp(x,y,sizeof x) && !memcmp(&a,&b,sizeof a));
        rewinds += stop < m;
    }
    assert(rewinds == 496);
    printf("%u forced early retire cases exercise front rewind\n",rewinds);
    printf("exact cymbal: %u samples, %u mixed-size calls, output and full state\n",samples,calls);
}
