#include <stdio.h>
#include "drum909.c"
int main(void) {
    drum909_t zero;
    drum909_init(&zero);
    unsigned long total=0;
    for(int ix=0;ix<89;++ix) for(int lo=0;lo<16;++lo) {
        uint8_t byte=(uint8_t)(lo|(lo<<4));
        for(int pr=-32768;pr<=32767;++pr) {
            for(int i=0;i<2;++i) {
                int32_t a=pr,b=pr,j=ix,k=ix;
                if(d9_ima_decode(&byte,(uint32_t)i,&a,&j)!=ima_decode(&byte,(uint32_t)i,&b,&k) || a!=b || j!=k)
                    return 1;
                ++total;
            }
        }
    }
    printf("PASS %lu (valid predictor,index,code,nibble parity) transitions\n",total);
    return 0;
}
