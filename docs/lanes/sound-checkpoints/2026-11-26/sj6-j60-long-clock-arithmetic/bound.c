#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>
/* Bare source recurrence; not a native render or composition witness. */
static void verify(unsigned fs, unsigned up, unsigned down) {
    const float ratio=96000.f/(float)fs;
    float bridge=0.f;
    uint64_t count=0;
    int64_t low=0, high=0;
    uint64_t first=0, jumps=0;
    for(uint64_t n=1;n<=60u*60u*fs;n++){
        float next=1.f-bridge;
        unsigned add=0;
        while(next<=ratio){
            ++count; ++add;
            next+=1.f;
        }
        assert(add==up/down || add==up/down+1);
        bridge=ratio-next+1.f;
        uint64_t exact=n*up/down;
        int64_t q=(int64_t)count-(int64_t)exact;
        if(q && !first) first=n;
        if(q<low)low=q;
        if(q>high)high=q;
        jumps += (add != exact-(n-1)*up/down);
        if(n==148684u || n==300u*fs || n==3600u*fs)
            printf("%u Hz host=%" PRIu64 " compact=%" PRIu64 " exact=%" PRIu64 " delta=%" PRId64 "\n",fs,n,count,exact,q);
    }
    printf("first=%" PRIu64 " min=%" PRId64 " max=%" PRId64 " host advances differing=%" PRIu64 "\n",first,low,high,jumps);
}
int main(void) { verify(44100,320,147); verify(48000,2,1); }
