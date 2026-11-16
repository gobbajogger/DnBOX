#include "reader/direct_reader.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
static unsigned long long checks=0,words=0;static unsigned negatives=0;
#define CHECK(x) do{++checks;if(!(x)){fprintf(stderr,"FAIL direct reader line%d: %s\n",__LINE__,#x);exit(1);}}while(0)
static void rejected(s1_morph_binding *live,const void*p,size_t n){unsigned char before[sizeof *live];memcpy(before,live,sizeof *live);CHECK(!s1_morph_bind_direct(live,p,n));CHECK(!memcmp(before,live,sizeof *live));++negatives;}
int main(int argc,char**argv){
 CHECK(argc==2);const size_t row=4097u*4u;unsigned char *raw=malloc(52u*row),*packed=malloc(S1_MORPH_DIRECT_BYTES),*bad=malloc(S1_MORPH_DIRECT_BYTES+8u);CHECK(raw&&packed&&bad);
 FILE*f=fopen(argv[1],"rb");CHECK(f);CHECK(fread(raw,1,52u*row,f)==52u*row);CHECK(fgetc(f)==EOF);CHECK(!fclose(f));
 memcpy(packed,raw,48u*row);memcpy(packed+48u*row,raw+50u*row,2u*row);memcpy(bad,packed,S1_MORPH_DIRECT_BYTES);
 s1_morph_binding live={0};CHECK(s1_morph_bind_direct(&live,packed,S1_MORPH_DIRECT_BYTES));CHECK(live.bytes==packed&&live.byte_count==S1_MORPH_DIRECT_BYTES);
 for(unsigned r=0;r<52;++r)for(unsigned i=0;i<4097;++i){uint32_t actual=123,expected=0;CHECK(s1_morph_read_word(&live,r,i,&actual));memcpy(&expected,raw+r*row+i*4u,4);CHECK(actual==expected);++words;}
 rejected(&live,NULL,S1_MORPH_DIRECT_BYTES);rejected(&live,bad,0);rejected(&live,bad,S1_MORPH_DIRECT_BYTES-1);rejected(&live,bad,S1_MORPH_DIRECT_BYTES+1);rejected(&live,bad,SIZE_MAX);rejected(&live,(const void*)(UINTPTR_MAX-1u),S1_MORPH_DIRECT_BYTES);rejected(&live,bad+1,S1_MORPH_DIRECT_BYTES);
 const uint32_t invalid[]={0x7fc00000u,0x7f800000u,0xff800000u,0x40400000u};for(unsigned i=0;i<4;++i){memcpy(bad,&invalid[i],4);rejected(&live,bad,S1_MORPH_DIRECT_BYTES);}memcpy(bad,packed,4);
 const uint32_t negative_zero=0x80000000u;memcpy(bad+row-4,&negative_zero,4);rejected(&live,bad,S1_MORPH_DIRECT_BYTES);memcpy(bad+row-4,packed+row-4,4);
 for(unsigned r=0;r<50;++r){const uint32_t nan=0x7fc00000u;memcpy(bad+r*row+row-4,&negative_zero,4);rejected(&live,bad,S1_MORPH_DIRECT_BYTES);memcpy(bad+r*row+row-4,packed+r*row+row-4,4);memcpy(bad+r*row,&nan,4);rejected(&live,bad,S1_MORPH_DIRECT_BYTES);memcpy(bad+r*row,packed+r*row,4);}
 unsigned char before[sizeof(s1_morph_binding)];memcpy(before,bad,sizeof before);CHECK(!s1_morph_bind_direct((s1_morph_binding*)bad,bad,S1_MORPH_DIRECT_BYTES));CHECK(!memcmp(before,bad,sizeof before));++negatives;
 const unsigned invalid_indices[][2]={{52,0},{UINT_MAX,0},{0,4097},{0,UINT_MAX}};for(unsigned i=0;i<4;++i){uint32_t out=0x12345678;CHECK(!s1_morph_read_word(&live,invalid_indices[i][0],invalid_indices[i][1],&out));CHECK(out==0x12345678);++negatives;}
 uint32_t out=0x12345678;s1_morph_binding empty={0};CHECK(!s1_morph_read_word((const s1_morph_binding*)(UINTPTR_MAX-1u),0,0,&out));CHECK(!s1_morph_read_word((const s1_morph_binding*)((const unsigned char*)&live+1),0,0,&out));CHECK(!s1_morph_read_word(&live,0,0,(uint32_t*)(UINTPTR_MAX-1u)));CHECK(!s1_morph_read_word(&live,0,0,(uint32_t*)((unsigned char*)&out+1)));CHECK(out==0x12345678);negatives+=4;CHECK(!s1_morph_read_word(NULL,0,0,&out));CHECK(!s1_morph_read_word(&empty,0,0,&out));CHECK(!s1_morph_read_word(&live,0,0,NULL));CHECK(out==0x12345678);negatives+=3;
 CHECK(!s1_morph_read_word(&live,0,0,(uint32_t*)packed));CHECK(!memcmp(packed,raw,4));++negatives;
 memcpy(before,&live,sizeof live);CHECK(!s1_morph_read_word(&live,0,0,(uint32_t*)&live));CHECK(!memcmp(before,&live,sizeof live));++negatives;
 printf("{\"scope\":\"direct-morph-reader-unit\",\"checks\":%llu,\"logical_words\":%llu,\"negative_cases\":%u,\"failures\":0}\n",checks,words,negatives);free(bad);free(packed);free(raw);return 0;
}
