/* Use exact sealed native lab sections. No writers, device or firmware emit. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app/cold_companion_lab.h"
#include "app/fdata_readonly_lab.h"
enum {TOTAL=55307,FRES=61440};
static uint8_t elite[TOTAL],pro[TOTAL],fd[FRES];
static void read_payload(const char *path,uint8_t *out)
{
    FILE *f=fopen(path,"rb");assert(f);
    assert(fread(out,1,TOTAL,f)==TOTAL && fgetc(f)==EOF && !fclose(f));
}
static void verify(const uint8_t *good,const uint8_t *other)
{
    uint8_t expected[32];memcpy(expected,good,32);
    memcpy(fd,good,TOTAL);
    memset(fd+TOTAL,0xff,FRES-TOTAL);
    assert(fdata_lab_status(fd,TOTAL,expected)==FDATA_READY);
    assert(fdata_lab_status(fd,TOTAL-1,expected)==FDATA_CORRUPT);
    assert(fdata_lab_status(fd,TOTAL+1,expected)==FDATA_CORRUPT);
    assert(fdata_lab_status(other,TOTAL,expected)==FDATA_MISMATCH);
    const unsigned idx[]={32,(TOTAL+32)/2,TOTAL-1};
    for(unsigned i=0;i<3;i++){
        fd[idx[i]]^=0x80;
        assert(fdata_lab_status(fd,TOTAL,expected)==FDATA_CORRUPT);
        fd[idx[i]]^=0x80;
    }
    fd[20]^=1;assert(fdata_lab_status(fd,TOTAL,expected)==FDATA_MISMATCH);fd[20]^=1;
    assert(!memcmp(good,fd,TOTAL));
    for(unsigned i=TOTAL;i<FRES;i++)assert(fd[i]==0xff);
    memset(fd,0xff,sizeof fd);
    assert(fdata_lab_status(fd,TOTAL,expected)==FDATA_MISSING);
    memcpy(fd,"S1PK",4); /* legacy classification, no old user bytes */
    assert(fdata_lab_status(fd,TOTAL,expected)==FDATA_LEGACY);
    memcpy(fd,"UNKN",4);
    assert(fdata_lab_status(fd,TOTAL,expected)==FDATA_MISMATCH);
}
int main(int argc,char **argv)
{
    assert(argc==3);
    read_payload(argv[1],elite);read_payload(argv[2],pro);
    assert(memcmp(elite,pro,32) && !memcmp(elite+32,pro+32,TOTAL-32));
    verify(elite,pro);verify(pro,elite);
    puts("PASS native-linked guard both headers, same exact 55275 B bodies, cross-model rejection, first/middle/last corruption, truncated/oversized, erased, occupied legacy/unknown");
}
