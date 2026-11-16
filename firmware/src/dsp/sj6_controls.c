/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_controls.h"
#include <string.h>
_Static_assert(sizeof(float)==4,"binary32 interface required");
#include "sj6_controls_data.inc"
#include "synthone_controls_data.inc"
static uint32_t rd(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void wr(unsigned char*p,uint32_t x){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(x>>(8*i));}
static uint32_t crc(const unsigned char*p,size_t n){uint32_t c=~0u;for(size_t k=0;k<n;k++){c^=p[k];for(unsigned j=0;j<8;j++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}return ~c;}
static int finite_word(uint32_t x){return (x&0x7f800000u)!=0x7f800000u;}
static int span(const void*p,size_t n){return p&&n<=UINTPTR_MAX-(uintptr_t)p;}
static int overlaps(const void*a,size_t n,const void*b,size_t m){return (uintptr_t)a<(uintptr_t)b+m&&(uintptr_t)b<(uintptr_t)a+n;}
static int valid(const void*v,size_t n){
 if(n!=DNBOX_SJ6_BLOB_BYTES||!span(v,n))return 0;const unsigned char*p=v;
 if(memcmp(p,"SJ61",4)||rd(p+4)!=(218u<<16|1u)||rd(p+8)!=872||rd(p+12)!=crc(p+16,872))return 0;
 for(unsigned i=0;i<218;i++)if(!finite_word(rd(p+16+i*4)))return 0;return 1;
}
const dnbox_sj6_control *dnbox_sj6_control_at(unsigned p,unsigned s){return p<8&&s<8?descriptors+p*8+s:0;}
const char *dnbox_sj6_page_name(unsigned p){return p<8?page_names[p]:0;}
const dnbox_s1_control *dnbox_s1_control_at(unsigned i){return i<150?s1_descriptors+i:0;}
dnbox_control_status dnbox_sj6_binding_status(unsigned id){return id<218?DNBOX_CTL_NOT_ADMITTED:DNBOX_CTL_BAD_ID;}
dnbox_control_status dnbox_sj6_blob_encode(void*v,size_t n,const float*x,size_t count){
 if(n!=DNBOX_SJ6_BLOB_BYTES||count!=218||!span(v,n)||!span(x,872)||overlaps(v,n,x,872))return DNBOX_CTL_BAD_ARGUMENT;
 for(unsigned i=0;i<218;i++){uint32_t b;memcpy(&b,x+i,4);if(!finite_word(b))return DNBOX_CTL_BAD_ARGUMENT;}
 unsigned char*p=v;memcpy(p,"SJ61",4);wr(p+4,218u<<16|1u);wr(p+8,872);
 for(unsigned i=0;i<218;i++){uint32_t b;memcpy(&b,x+i,4);wr(p+16+i*4,b);}wr(p+12,crc(p+16,872));return DNBOX_CTL_OK;
}
dnbox_control_status dnbox_sj6_blob_get(const void*v,size_t n,unsigned id,float*out){
 if(id>=218)return DNBOX_CTL_BAD_ID;if(!span(out,4))return DNBOX_CTL_BAD_ARGUMENT;
 if(!valid(v,n))return DNBOX_CTL_BAD_BLOB;if(overlaps(v,n,out,4))return DNBOX_CTL_BAD_ARGUMENT;
 uint32_t b=rd((const unsigned char*)v+16+id*4);memcpy(out,&b,4);return DNBOX_CTL_OK;
}
dnbox_control_status dnbox_sj6_blob_set(void*v,size_t n,unsigned id,float value){
 if(id>=218)return DNBOX_CTL_BAD_ID;uint32_t b;memcpy(&b,&value,4);if(!finite_word(b))return DNBOX_CTL_BAD_ARGUMENT;
 if(!valid(v,n))return DNBOX_CTL_BAD_BLOB;unsigned char*p=v;wr(p+16+id*4,b);wr(p+12,crc(p+16,872));return DNBOX_CTL_OK;
}
