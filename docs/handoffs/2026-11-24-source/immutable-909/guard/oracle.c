#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "app/cold_companion_lab.h"
#include "app/fdata_readonly_lab.h"
static uint8_t needed[32];
static void in32(uint8_t *d,uint32_t v) { for (unsigned i=0;i<4;++i) d[i]=(uint8_t)(v>>(8*i)); }
int main(void)
{
    long pg=sysconf(_SC_PAGESIZE);
    uint8_t *seg=mmap(0,(size_t)(pg*2),PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    assert(seg!=MAP_FAILED);
    uint8_t *d=seg+pg-32;
    memset(d,255,64); memcpy(needed,"D9RD",4); in32(needed+4,1); in32(needed+8,1); in32(needed+12,32);
    for (unsigned i=32;i<64;++i) d[i]=(uint8_t)(i*7);
    in32(needed+16,x0x_crc32(d+32,32)); needed[20]=43;
    memcpy(d,needed,32);
    assert(fdata_lab_status(d,64,needed)==FDATA_READY);
    d[33]^=1;
    assert(fdata_lab_status(d,64,needed)==FDATA_CORRUPT);
    d[33]^=1;
    assert(fdata_lab_status(d,63,needed)==FDATA_CORRUPT);
    assert(mprotect(seg+pg,(size_t)pg,PROT_NONE)==0);
    /* Only matching header is allowed to touch body: prove using inaccessible mapping. */
    d[8]^=3; assert(fdata_lab_status(d,64,needed)==FDATA_MISMATCH);d[8]^=3;
    d[4]^=2; assert(fdata_lab_status(d,64,needed)==FDATA_MISMATCH);d[4]^=2;
    d[20]^=1; assert(fdata_lab_status(d,64,needed)==FDATA_MISMATCH);
    memcpy(d,"S1PK",4); assert(fdata_lab_status(d,64,needed)==FDATA_LEGACY);
    memset(d,255,4); assert(fdata_lab_status(d,64,needed)==FDATA_MISSING);
    assert(fdata_lab_status(d,31,needed)==FDATA_MISSING);
    assert(fdata_lab_status(NULL,64,needed)==FDATA_MISSING);
    assert(munmap(seg,(size_t)pg*2)==0);
    puts("10 checks: valid, CRC, size, wrong model/version/app, legacy, erased/short/null; unmatched body PROT_NONE");
}
