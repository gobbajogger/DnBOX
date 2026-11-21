// Real compact input and scalar reader comparison, no DSP arithmetic changes.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "compact.c"
#define CHECK(x) do { if(!(x)){fprintf(stderr,"FAIL line %d: %s\n",__LINE__,#x);exit(1);} } while(0)
extern int s1c_original_read(const s1c_binding*,unsigned,unsigned,uint32_t*);
static unsigned rows_seen,words_seen;static int counting;
__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void*f,void*c){(void)c;if(counting&&f==(void*)&getrow)++rows_seen;if(counting&&f==(void*)&physical_word_row)++words_seen;}
__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void*f,void*c){(void)f;(void)c;}
static void rejected(const s1c_binding*b,unsigned r0,unsigned r1,unsigned p0,unsigned p1){uint32_t o[4]={1,2,3,4},old[4];memcpy(old,o,sizeof o);CHECK(!s1c_read_quad(b,r0,r1,p0,p1,o));CHECK(!memcmp(o,old,sizeof o));}
int main(int argc,char**argv){
 CHECK(argc==3);FILE*f=fopen(argv[1],"rb");CHECK(f);unsigned char *bytes=malloc(165292);CHECK(bytes);CHECK(fread(bytes,1,165292,f)==165292);CHECK(!fclose(f));s1c_binding b;CHECK(s1c_bind(&b,bytes,165292));
 uint32_t out[4],want[4];unsigned long long comparisons=0;
 // Count actual decoding of distinct coordinates, with every public guard retained.
 const unsigned unique_cases[][5]={{0,1,63,64,4},{44,48,63,64,2},{0,1,63,63,2},{44,48,4096,4096,1}};
 for(unsigned c=0;c<4;++c){const unsigned *v=unique_cases[c];words_seen=0;counting=1;
  CHECK(s1c_read_quad(&b,v[0],v[1],v[2],v[3],out));counting=0;
  printf("physical_word_evaluations=%u expected=%u\n",words_seen,v[4]);CHECK(words_seen==v[4]);
 }
 // Every row pair includes identical/aliased rows; every position covers block,
 // wrap and +0-guard edges. Scalar API remains the independent word path.
 for(unsigned r0=0;r0<52;++r0)for(unsigned r1=0;r1<52;++r1)for(unsigned p=0;p<4097;++p){unsigned q=(p+1)%4097;
  CHECK(s1c_original_read(&b,r0,p,&want[0]));CHECK(s1c_original_read(&b,r1,p,&want[1]));CHECK(s1c_original_read(&b,r0,q,&want[2]));CHECK(s1c_original_read(&b,r1,q,&want[3]));
  CHECK(s1c_read_quad(&b,r0,r1,p,q,out));CHECK(!memcmp(out,want,sizeof out));comparisons+=4;
 }
 counting=1;rows_seen=0;CHECK(s1c_read_quad(&b,0,1,63,64,out));counting=0;printf("row_validations=%u expected=%u\n",rows_seen,(unsigned)strtoul(argv[2],0,10));CHECK(rows_seen==strtoul(argv[2],0,10));
 unsigned bad[]={52,UINT_MAX};for(unsigned i=0;i<2;++i){rejected(&b,bad[i],0,0,1);rejected(&b,0,bad[i],0,1);}unsigned positions[]={4097,UINT_MAX};for(unsigned i=0;i<2;++i){rejected(&b,0,1,positions[i],1);rejected(&b,0,1,0,positions[i]);}
 rejected(NULL,0,1,0,1);s1c_binding invalid=b;invalid.byte_count=SIZE_MAX;rejected(&invalid,0,1,0,1);invalid=b;invalid.format_tag=0;rejected(&invalid,0,1,0,1);
 CHECK(!s1c_read_quad(&b,0,1,0,1,NULL));CHECK(!s1c_read_quad(&b,0,1,0,1,(uint32_t*)(UINTPTR_MAX-7)));
 s1c_binding saved=b;CHECK(!s1c_read_quad(&b,0,1,0,1,(uint32_t*)&b));CHECK(!memcmp(&saved,&b,sizeof b));
 unsigned char first[16];memcpy(first,bytes,16);CHECK(!s1c_read_quad(&b,0,1,0,1,(uint32_t*)bytes));CHECK(!memcmp(first,bytes,16));
 unsigned char unaligned[32];memset(unaligned,0x5a,sizeof unaligned);CHECK(!s1c_read_quad(&b,0,1,0,1,(uint32_t*)(unaligned+1)));for(unsigned i=0;i<sizeof unaligned;++i)CHECK(unaligned[i]==0x5a);
 // Same-position and guard pairs remain valid, including logical aliases.
 for(unsigned r=0;r<52;++r)for(unsigned p=0;p<4097;++p){CHECK(s1c_read_quad(&b,r,r,p,p,out));CHECK(s1c_original_read(&b,r,p,want));for(unsigned i=0;i<4;++i)CHECK(out[i]==want[0]);}
 // Corrupt a late row's block only as a defensive rejection test. Mutating a
 // bound payload is outside the supported immutable contract, never a cache API.
 view v;rowinfo q;CHECK(header(bytes,165292,&v));CHECK(getrow(&v,0,&q));CHECK(q.mode!=3);
 const unsigned sample=q.flags&4095;const uint32_t block_start=u32(bytes+1368+4*q.first);unsigned char old_mode=bytes[block_start];bytes[block_start]=99;
 CHECK(s1c_read(&b,44,sample,want));CHECK(s1c_read(&b,0,4096,want));CHECK(!s1c_read(&b,0,sample,want));rejected(&b,44,0,4096,sample);rejected(&b,0,0,4096,sample);bytes[block_start]=old_mode;
 printf("PASS quad scalar word comparisons=%llu, transactional invalid/alias/bounds checks\n",comparisons);free(bytes);return 0;
}
