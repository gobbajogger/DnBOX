/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { FLASH=0x100000, LO=0xE5000, HI=0xF4000, LEN=HI-LO, OFF=4096, WRITES=15*17 };
static uint8_t model[FLASH], snapshot[FLASH], image[OFF+LEN], old_body[LEN], new_body[LEN];
static uint8_t hdr[0x400], sec[4096], cur[4096];
static unsigned attempt, fail_on, mutations;
static uint32_t ota_rd16(const uint8_t *p) { return p[0] | p[1]<<8; }
static uint32_t ota_rd32(const uint8_t *p) {
    return (uint32_t)p[0] | (uint32_t)p[1]<<8 | (uint32_t)p[2]<<16 | (uint32_t)p[3]<<24;
}
static void put_le(uint8_t *q, uint32_t n, unsigned z) {
    for (unsigned k=0;k<z;k++,n>>=8) q[k]=(uint8_t)n;
}
static unsigned ota_crc16(const uint8_t *p, unsigned z, unsigned crc) {
    for (unsigned i=0;i<z;i++) {
        crc^=(unsigned)p[i]<<8;
        for(unsigned k=0;k<8;k++) crc=((crc<<1)^(crc&0x8000u?0x1021u:0u))&0xffffu;
    }
    return crc;
}
static int ota_memeq(const uint8_t *a,const uint8_t *b,unsigned z){return !memcmp(a,b,z);}
static int ota_read(unsigned src,uint8_t *dst,unsigned z) {
    assert(src>=OFF && src+z<=sizeof image);
    memcpy(dst,image+src,z);
    return 0;
}
static int ldr_fread(unsigned src,uint8_t *dst,unsigned z) {
    assert(src>=LO && src+z<=HI && z==4096);
    memcpy(dst,model+src,z);
    return 0;
}
static int ldr_erase(unsigned pos) {
    assert(pos>=LO && pos+4096<=HI && !(pos%4096));
    int stop=++attempt==fail_on;
    memset(model+pos,255,stop?2048u:4096u);
    ++mutations;
    return stop;
}
static int ldr_prog(unsigned pos,const uint8_t *src,unsigned n) {
    assert(pos>=LO && pos+n<=HI && n==256 && !(pos%256));
    int stop=++attempt==fail_on;
    for(unsigned i=0;i<(stop?n/2:n);i++){
        assert((model[pos+i]&src[i])==src[i]);
        model[pos+i]=src[i];
    }
    ++mutations;
    return stop;
}
#include "selected_loader.inc"
static int run(unsigned write) {
    return ldr_region(hdr,sec,cur,write,103,LO,HI,-16);
}
static void stage(const uint8_t *body) {
    memset(hdr,0,sizeof hdr);
    put_le(hdr+8,1,2);put_le(hdr+0x40,103,2);
    put_le(hdr+0x44,ota_crc16(body,LEN,0),2);
    put_le(hdr+0x48,OFF,4);put_le(hdr+0x4c,LEN,4);
    memcpy(image+OFF,body,LEN);
}
static void protected_same(void) {
    assert(!memcmp(model,snapshot,LO));
    assert(!memcmp(model+HI,snapshot+HI,FLASH-HI));
}
int main(void) {
    /* Synthetic old/new exact full-region sequences: never actual user contents. */
    for(unsigned i=0;i<FLASH;i++) snapshot[i]=(uint8_t)(13u+i*23u);
    for(unsigned i=0;i<LEN;i++) {
        old_body[i]=(uint8_t)(97u+i*13u);
        new_body[i]=(uint8_t)(19u+i*67u);
    }
    memcpy(old_body,"S1PK",4);memcpy(new_body,"D9RD",4);
    unsigned outcomes=0;
    for(unsigned cut=1;cut<=WRITES;cut++){
        for(unsigned direction=0;direction<2;direction++){
            const uint8_t *before=direction?new_body:old_body, *after=direction?old_body:new_body;
            memcpy(model,snapshot,FLASH);memcpy(model+LO,before,LEN);
            stage(after);attempt=mutations=0;fail_on=cut;
            assert(!run(0) && mutations==0);
            assert(run(1)<0 && mutations==cut);
            protected_same();

            /* Repeating the originally selected, still readable staging input
               reconstructs the entire region over a partial erase/program. */
            unsigned n=mutations; fail_on=0;
            assert(!run(0) && !run(1) && !memcmp(model+LO,after,LEN));
            assert(mutations>n);
            protected_same();
            n=mutations;
            assert(!run(1) && mutations==n); /* idempotent verified repeat */
            outcomes++;

            /* A separately retained full prior input can be restored without
               assuming the first-word header describes old flash contents. */
            stage(before);
            assert(!run(0) && !run(1) && !memcmp(model+LO,before,LEN));
            protected_same();
        }
    }
    stage(new_body);attempt=mutations=fail_on=0;
    image[OFF+LEN-1]^=1;
    assert(run(0)==-17 && !mutations); /* complete precheck catches last block */
    put_le(hdr+0x4c,LEN-4096,4);
    assert(run(0)==-16 && !mutations);
    put_le(hdr+8,0,2);
    assert(!run(0) && !run(1) && !mutations);
    protected_same();
    printf("PASS existing FDATA writer: %u interrupted sector-operation cases, retry and rollback exact; protected regions unchanged\n",outcomes);
    return 0;
}
