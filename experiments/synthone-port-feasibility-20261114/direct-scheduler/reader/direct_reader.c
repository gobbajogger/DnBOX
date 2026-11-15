#include "direct_reader.h"
#include <stdint.h>
#include <string.h>
#include <math.h>
#define S1_DIRECT_TAG UINT32_C(0x53315231)
static int extent(const void *p,size_t n){return p!=NULL&&n!=0&&(uintptr_t)p<=UINTPTR_MAX-n;}
static int overlap(const void *a,size_t an,const void *b,size_t bn){
 if(!extent(a,an)||!extent(b,bn))return 1;
 return (uintptr_t)a<(uintptr_t)b+bn&&(uintptr_t)b<(uintptr_t)a+an;
}
static uint32_t word_le(const unsigned char *p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static int encoding(void){
 uint32_t one=1,bits=0;float f=1.f;if(sizeof(float)!=4||sizeof(uint32_t)!=4)return 0;
 memcpy(&bits,&f,4);return ((const unsigned char*)&one)[0]==1&&bits==UINT32_C(0x3f800000);
}
int s1_morph_binding_valid(const s1_morph_binding *b){
 if(!b||!extent(b,sizeof *b)||((uintptr_t)b%__alignof__(s1_morph_binding))!=0)return 0;
 return b->format_tag==S1_DIRECT_TAG&&b->byte_count==S1_MORPH_DIRECT_BYTES&&
        extent(b->bytes,b->byte_count)&&((uintptr_t)b->bytes%__alignof__(float))==0;
}
int s1_morph_bind_direct(s1_morph_binding *out,const void *data,size_t bytes){
 if(!out||!extent(out,sizeof *out)||((uintptr_t)out%__alignof__(s1_morph_binding))!=0||
    bytes!=S1_MORPH_DIRECT_BYTES||!extent(data,bytes)||((uintptr_t)data%__alignof__(float))!=0||
    overlap(out,sizeof *out,data,bytes)||!encoding())return 0;
 const unsigned char *p=(const unsigned char*)data;
 for(size_t i=0;i<(size_t)S1_MORPH_UNIQUE_ROWS*S1_MORPH_WORDS_PER_ROW;++i){
  const uint32_t bits=word_le(p+4*i);float f;memcpy(&f,&bits,4);
  if(!isfinite(f)||f< -2.f||f>2.f||(i%S1_MORPH_WORDS_PER_ROW==4096&&bits!=0))return 0;
 }
 const s1_morph_binding candidate={p,bytes,S1_DIRECT_TAG};*out=candidate;return 1;
}
int s1_morph_read_word(const s1_morph_binding *b,unsigned row,unsigned sample,uint32_t *out){
 if(!s1_morph_binding_valid(b)||row>=S1_MORPH_LOGICAL_ROWS||sample>=S1_MORPH_WORDS_PER_ROW||
    !out||!extent(out,sizeof *out)||((uintptr_t)out%__alignof__(uint32_t))!=0||
    overlap(out,sizeof *out,b,sizeof *b)||overlap(out,sizeof *out,b->bytes,b->byte_count))return 0;
 const unsigned unique=row<48?row:row==48?44:row==49?45:row-2;
 const size_t offset=((size_t)unique*S1_MORPH_WORDS_PER_ROW+sample)*4;
 /* Constants/counts above prove offset+4 is in the admitted immutable span. */
 *out=word_le(b->bytes+offset);return 1;
}
