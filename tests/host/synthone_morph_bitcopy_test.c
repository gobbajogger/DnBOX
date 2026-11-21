/* Exercise real morph/reader code: exact output/state, no tiny out-of-line copies. */
#include "compact_morph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
extern int s1c_baseline_morph_compute(sp_data*,sp_oscmorph2d*,SPFLOAT*,SPFLOAT*,const s1c_binding*);
static volatile int timed_region;
static uint64_t copy_calls;
void *__real_memcpy(void *, const void *, size_t);
void *__wrap_memcpy(void *dst, const void *src, size_t n) {
    if (timed_region) copy_calls++;
    return __real_memcpy(dst,src,n);
}
static uint32_t bits(float value) { uint32_t word; memcpy(&word,&value,4); return word; }
int main(int argc,char **argv) {
    if(argc!=2)return 2;
    FILE *f=fopen(argv[1],"rb");if(!f)return 2;
    if(fseek(f,0,SEEK_END))return 2;long n=ftell(f);if(n<=0||n>4194304)return 2;
    rewind(f);unsigned char *data=malloc((size_t)n);if(!data||fread(data,1,(size_t)n,f)!=(size_t)n)return 2;fclose(f);
    s1c_binding binding;if(!s1c_bind(&binding,data,(size_t)n))return 2;
    sp_data sp={0};sp.sr=48000;
    sp_ftbl table[52]={0};sp_ftbl *pointers[52];float bands[13];
    for(unsigned i=0;i<52;i++) { pointers[i]=table+i;table[i].size=4096;table[i].lobits=12;table[i].lomask=4095;table[i].lodiv=1.f/4096.f;table[i].sicvt=(float)(1.0*SP_FT_MAXLEN/sp.sr); }
    for(unsigned i=0;i<13;i++)memcpy(bands+i,data+1316+4*i,4);
    const float frequencies[]={0.f,0.1f,220.f,440.f,7999.f,15999.f};
    const float positions[]={0.f,0.25f,0.99f,1.f,1.75f,0.5f};
    uint64_t frames=0,differences=0;
    for(unsigned scene=0;scene<6;scene++) {
        sp_oscmorph2d old={0},now={0};old.tbl=pointers;old.fbl=bands;old.nft=4;old.nbl=13;
        old.freq=frequencies[scene];old.wtpos=positions[scene];old.amp=.7f;
        old.enableBandlimit=(int)(scene&1);old.bandlimitIndexOverride=-1;old.lphs=(int32_t)(scene*951);
        now=old;
        for(unsigned frame=0;frame<4096;frame++) {
            float want=0.f,got=0.f;
            int a=s1c_baseline_morph_compute(&sp,&old,NULL,&want,&binding);
            timed_region=1;int b=s1c_runtime_morph_compute(&sp,&now,NULL,&got,&binding);timed_region=0;
            if(a!=SP_OK||b!=SP_OK||bits(want)!=bits(got)||old.lphs!=now.lphs||old.inc!=now.inc||bits(old.wtpos)!=bits(now.wtpos))differences++;
            frames++;
        }
    }
    printf("frames=%" PRIu64 " differences=%" PRIu64 " hot_external_memcpy_calls=%" PRIu64 "\n",frames,differences,copy_calls);
    free(data);return differences||copy_calls?1:0;
}
