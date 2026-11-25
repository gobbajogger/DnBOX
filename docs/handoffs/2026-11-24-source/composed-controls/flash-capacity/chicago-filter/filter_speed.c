/* Offline comparison of the exact original and fixed-point table readers. */
#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
typedef struct {const uint8_t *data;const uint16_t *offset;uint32_t bytes,count,blocks;} original_lossless_table16;
#include "original_compact.h"
#include "p_a.h"
#include "p_e.h"

static uint16_t u16(const uint8_t *p){return (uint16_t)p[0]|(uint16_t)((uint16_t)p[1]<<8);}
static uint32_t u32(const uint8_t *p){return (uint32_t)u16(p)|((uint32_t)u16(p+2)<<16);}
static __attribute__((noinline)) uint32_t baseline_lookup(const original_lossless_table16 *t,unsigned i,uint32_t prefix){
    const uint8_t *p=t->data+t->offset[i>>5];
    unsigned entry=i&31u,width=p[3]&31u,available=0;
    uint32_t word=((uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|((uint32_t)p[3]>>5)<<24)|prefix;
    uint32_t delta=(uint32_t)p[4]|(uint32_t)p[5]<<8|(uint32_t)p[6]<<16;
    delta=(delta^0x800000u)-0x800000u;
    if(entry)word+=delta;
    p+=7;
    uint32_t bits=0,mask=(1u<<width)-1u;
    for(unsigned j=1;j<entry;j++){
        while(available<width){bits|=(uint32_t)*p++<<available;available+=8;}
        uint32_t z=bits&mask;
        bits>>=width;available-=width;
        delta+=(z>>1)^(0u-(z&1u));word+=delta;
    }
    return word;
}
static __attribute__((noinline)) uint32_t compact_lookup(const unsigned char *t,unsigned idx){
    unsigned i=idx&127,ns=u16(t+2);
    const uint8_t *p=t+4+2*ns+u16(t+4+(idx>>7)*2);
    unsigned degree=p[0]>>5,width=p[0]&31,q=p[9];
    uint32_t base=u32(p+1),lo=u32(p+5);
    p+=10;
    int32_t coefficients[4];
    for(unsigned j=0;j<=degree;j++){
        uint32_t z=0;
        unsigned b,count=0;
        do{b=*p++;z|=(b&127u)<<count;count+=7;}while(b&128u);
        coefficients[j]=(int32_t)((z>>1)^(0u-(z&1u)));
    }
    int32_t v=coefficients[degree];
    for(unsigned j=degree;j>0;j--)v=coefficients[j-1]+((v*(int32_t)i)>>7);
    if(q)v=(v+(int32_t)(1u<<(q-1)))>>q;
    unsigned bit=i*width,shift=bit&7,rem=width;
    uint32_t z=0;
    p+=bit>>3;
    if(rem){
        unsigned take=8-shift;
        if(take>rem)take=rem;
        z=(unsigned)(*p++>>shift)&((1u<<take)-1u);
        rem-=take;
        while(rem){unsigned n=rem<8?rem:8;z|=((uint32_t)*p++&((1u<<n)-1u))<<take;take+=n;rem-=n;}
    }
    return base+lo+(uint32_t)v+z;
}
static unsigned micros(void){
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,&ts);
    return (unsigned)(ts.tv_sec*1000000UL+ts.tv_nsec/1000);
}
int main(void){
    for(unsigned i=0;i<=16384;i++){
        if(compact_lookup(p_a,i)!=baseline_lookup(&original_chicago_packed_a,i,original_chicago_packed_a_prefix)||
           compact_lookup(p_e,i)!=baseline_lookup(&original_chicago_packed_e,i,original_chicago_packed_e_prefix)){
            fprintf(stderr,"exact check failed at index %u\n",i);return 1;
        }
    }
    puts("32,770 words match in C.");
    for(unsigned trial=0;trial<3;trial++){
        for(unsigned mode=0;mode<2;mode++){
            uint32_t s=1,acc=0;
            unsigned start=micros(),runs=2000000;
            for(unsigned k=0;k<runs;k++){
                s=s*1664525u+1013904223u;
                unsigned i=(s>>16)%16385;
                acc+=mode?compact_lookup(p_a,i)+compact_lookup(p_e,i):
                    baseline_lookup(&original_chicago_packed_a,i,original_chicago_packed_a_prefix)+
                    baseline_lookup(&original_chicago_packed_e,i,original_chicago_packed_e_prefix);
            }
            printf("%s %u us, checksum %08x\n",mode?"fixed-random-access":"baseline",micros()-start,acc);
        }
    }
    return 0;
}
