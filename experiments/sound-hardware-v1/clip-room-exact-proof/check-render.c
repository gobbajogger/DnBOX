/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app/s1c_part.h"

void candidate_s1c_render(s1c *,float *,float *,unsigned);
static s1_dry_part p, prestate, poststate;
static _Alignas(16) unsigned char arena[S1C_ARENA_BYTES],pre[S1C_ARENA_BYTES],post[S1C_ARENA_BYTES];
static float out[4][256];

int main(int argc,char **argv) {
    assert(argc==2);
    FILE *f=fopen(argv[1],"rb"); assert(f);
    assert(!fseek(f,0,SEEK_END));
    long sz=ftell(f); assert(sz>0 && !fseek(f,0,SEEK_SET));
    unsigned char *data=malloc((size_t)sz); assert(data);
    assert(fread(data,1,(size_t)sz,f)==(size_t)sz && !fclose(f));
    unsigned count=s1_part_set_pack(data,(uint32_t)sz);
    assert(count==694);
    unsigned long long frames=0;
    const unsigned sizes[]={1,3,32,41,64,127,249,256,256,256};
    for(unsigned k=0;k<=count;k++) {
        memset(arena,0,sizeof arena);
        s1_part_init(&p);
        s1_part_bind(&p,arena);
        assert(s1_part_program(&p,k<count?k:S1_PART_INIT));
        for(unsigned b=0;b<96;b++) {
            if (b==0 || b==29) for(unsigned j=0;j<6;j++)
                s1_part_note(&p,44+2*j+(b==29),82+3*j);
            if(b==48) s1_part_all_off(&p);
            if(b==63 && (k%5==1)) {
                s1c_set(&p.e,37,1.0f);
                s1c_set(&p.e,40,.63f);
                s1c_set(&p.e,41,1.0f);
                s1c_set(&p.e,42,.51f);
            }
            unsigned n=sizes[(b+k)%10];
            memcpy(&prestate,&p,sizeof p);memcpy(pre,arena,sizeof arena);
            s1c_render(&p.e,out[0],out[1],n);
            memcpy(&poststate,&p,sizeof p);memcpy(post,arena,sizeof arena);
            memcpy(&p,&prestate,sizeof p);memcpy(arena,pre,sizeof arena);
            candidate_s1c_render(&p.e,out[2],out[3],n);
            if(memcmp(&p,&poststate,sizeof p)||memcmp(arena,post,sizeof arena)||
               memcmp(out[0],out[2],n*sizeof(float))||memcmp(out[1],out[3],n*sizeof(float))) {
                fprintf(stderr,"Mismatch: case %u, span %u\n",k,b);return 1;
            }
            frames+=n;
        }
    }
    free(data);
    printf("PASS %u factory programs plus INIT; %llu exact stereo samples and complete engine/arena state\n",count,frames);
}
