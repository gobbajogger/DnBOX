#include "chicago_packed.h"
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int baseline_lookup(const void *,uint32_t,float *,float *);
static float expected_a[65536], expected_e[65536];
static uint32_t count;
typedef struct { original_chicago_packed_cache cache; uint32_t next; } owner;
static void *check(void *arg) {
    owner *s=arg;
    for(unsigned i=0;i<100000;i++){
        s->next=s->next*1664525u+1013904223u;
        uint32_t index=s->next%count;
        float a=0.f,e=0.f;
        for(unsigned repeat=0;repeat<3;repeat++) {
            assert(original_chicago_packed_cached(&s->cache,index,&a,&e));
            assert(!memcmp(&a,&expected_a[index],4) && !memcmp(&e,&expected_e[index],4));
        }
    }
    return 0;
}
int main(void) {
    float a,e;
    for(;count<65536;count++){
        if(!baseline_lookup(0,count,&expected_a[count],&expected_e[count]))break;
        assert(original_chicago_packed_lookup(0,count,&a,&e));
        assert(!memcmp(&a,&expected_a[count],4) && !memcmp(&e,&expected_e[count],4));
    }
    assert(count && count<65536 && !original_chicago_packed_lookup(0,count,&a,&e));
    assert(!original_chicago_packed_lookup(&count,0,&a,&e));
    assert(!original_chicago_packed_lookup(0,0,&a,&a));
    assert(!original_chicago_packed_lookup(0,0,0,&e));
    owner states[2]={{{UINT32_MAX,0.f,0.f},13},{{UINT32_MAX,0.f,0.f},13111}};
    assert(!original_chicago_packed_cached(0,0,&a,&e));
    assert(!original_chicago_packed_cached(&states[0].cache,UINT32_MAX,&a,&e));
    assert(!original_chicago_packed_cached(&states[0].cache,0,&a,&a));
    assert(!original_chicago_packed_cached(&states[0].cache,0,&a,0));
    assert(original_chicago_packed_cached(&states[0].cache,1,&a,&e));
    original_chicago_packed_cache before=states[0].cache;
    assert(!original_chicago_packed_cached(&states[0].cache,count,&a,&e));
    assert(!memcmp(&states[0].cache,&before,sizeof before));
    pthread_t threads[2];
    for(unsigned j=0;j<2;j++)assert(!pthread_create(&threads[j],0,check,&states[j]));
    for(unsigned j=0;j<2;j++)assert(!pthread_join(threads[j],0));
    printf("paired coefficients=%u concurrent exact lookups=600000 owner-bytes=%zu\n",count,sizeof(original_chicago_packed_cache));
}
