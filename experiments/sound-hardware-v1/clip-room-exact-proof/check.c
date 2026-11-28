/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "s1u.c"
int test_rb_put_exhaustive(void)
{
    static const uint16_t off[5]={0,16,32,48,64};
    s1u_rvb r={0}; _Alignas(2) unsigned char arena[196];
    r.off=off;
    for (int p=0;p<=32768;p++) {
        s1u_rvb_attach(&r,arena);
        for (int j=0;j<16;j++) {
            const int v=j==0?(p==32768?-32768:p):j==1?-p:(j*p/16)*(j&1?1:-1);
            rb_put(&r,0,j,(float)v*(1.0f/2048.0f));
        }
        int e=p?(int)__builtin_ceilf(log2f((float)p*(1.0f/2048.0f))):-128;
        assert(r.e[0]==e && r.bn[0]==0);
        if(p) {
            const float sc=exp2f((float)e)/127.0f;
            for(int j=0;j<16;j++)
                assert(r.m[j]==(int8_t)__builtin_roundf((float)r.st[j]*(1.0f/2048.0f)/sc));
        }
    }
    for (unsigned sh=0;sh<=15;sh++) {
        const int d=1u<<sh;
        const int peak=d==32768?-32768:d;
        for(int v=-d;v<=d && v<=32767;v++){
            s1u_rvb_attach(&r,arena);
            for(int j=0;j<16;j++){
                const int staged=j==0?peak:v;
                rb_put(&r,0,j,(float)staged*(1.0f/2048.0f));
            }
            const float sc=exp2f((float)((int)sh-11))/127.0f;
            assert(r.e[0]==(int)sh-11);
            assert(r.m[1]==(int8_t)__builtin_roundf((float)v*(1.0f/2048.0f)/sc));
        }
    }
    for (int e=-11;e<=4;e++) {
        r.e[0]=(int8_t)e;
        for (int q=-127;q<=127;q++) {
            r.m[0]=(int8_t)q;
            float expected=(float)q*(exp2f((float)e)/127.0f);
            float actual=rb_get(&r,0,0,16);
            assert(memcmp(&expected,&actual,sizeof actual)==0);
        }
    }
    puts("PASS peak exponent 32769, signed sample pairs 131085, read scale pairs 4080");
    return 0;
}
void base_init(s1u_rvb *,const uint16_t *,const uint16_t *,float);
void base_attach(s1u_rvb *,void *);
void base_run(s1u_rvb *,float,float,float,float *,float *,float);
int main(void)
{
    test_rb_put_exhaustive();
    const uint16_t lens[4]={350,414,476,548}, off[5]={0,192,416,672,960};
    _Alignas(2) unsigned char a[1148],b[1148];
    s1u_rvb ca={0}, cb={0};
    s1u_rvb_init(&ca,lens,off,22050);
    base_init(&cb,lens,off,22050);
    s1u_rvb_attach(&ca,a);
    base_attach(&cb,b);
    uint32_t s=1;
    for(unsigned i=0;i<1000000;i++){
        s=1664525u*s+1013904223u;
        float l=(int16_t)(s>>16)*(i%8192<5000?1.0f/32768.0f:0.0f);
        s=1664525u*s+1013904223u;
        float r=(int16_t)(s>>16)*(i%8192<5000?1.0f/32768.0f:0.0f);
        float f=(i/2235%10)*0.1f;
        float oa[2],ob[2];
        s1u_rvb_run(&ca,f,l,r,&oa[0],&oa[1],22050);
        base_run(&cb,f,l,r,&ob[0],&ob[1],22050);
        assert(memcmp(oa,ob,sizeof oa)==0 && memcmp(a,b,sizeof a)==0);
        const s1u_rvb tmp=ca;
        ca.m=cb.m;ca.e=cb.e;ca.st=cb.st;
        assert(memcmp(&ca,&cb,sizeof ca)==0);
        ca=tmp;
    }
    puts("PASS 1,000,000 stereo Room samples, complete arena and engine state");
}
