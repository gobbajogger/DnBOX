// SPDX-License-Identifier: GPL-3.0-only
#include "compact.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static unsigned checks,malformed,invalid_reads;
#define CHECK(x) do{checks++;if(!(x)){fprintf(stderr,"FAIL:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static unsigned char* load(const char*p,size_t*n){FILE*f=fopen(p,"rb");CHECK(f);CHECK(!fseek(f,0,SEEK_END));*n=(size_t)ftell(f);CHECK(!fseek(f,0,SEEK_SET));unsigned char*b=(unsigned char*)malloc(*n);CHECK(b);CHECK(fread(b,1,*n,f)==*n);fclose(f);return b;}
static uint32_t le(const unsigned char*p){return (uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void put(unsigned char*p,uint32_t v){for(unsigned i=0;i<4;i++)p[i]=(unsigned char)(v>>(8*i));}
static void put16(unsigned char*p,unsigned v){p[0]=(unsigned char)v;p[1]=(unsigned char)(v>>8);}
static void bad(s1c_binding*live,const unsigned char*p,size_t n){s1c_binding before;memcpy(&before,live,sizeof before);CHECK(!s1c_bind(live,p,n));CHECK(!memcmp(live,&before,sizeof before));malformed++;}
static void bad_read(s1c_binding*b,unsigned r,unsigned s){uint32_t out=UINT32_C(0xdeadbeef);CHECK(!s1c_read(b,r,s,&out));CHECK(out==UINT32_C(0xdeadbeef));invalid_reads++;}
int main(int argc,char**argv){
 if(argc!=3)return 2;size_t n=0,rn=0;unsigned char*p=load(argv[1],&n),*raw=load(argv[2],&rn),*tmp=(unsigned char*)malloc(n);CHECK(tmp&&n==165292&&rn==852176);
 s1c_binding b;memset(&b,0xa5,sizeof b);CHECK(s1c_bind(&b,p,n));CHECK(s1c_binding_valid(&b));unsigned words=0,nz=0;
 for(unsigned r=0;r<52;r++)for(unsigned s=0;s<4097;s++){uint32_t out=0;CHECK(s1c_read(&b,r,s,&out));CHECK(out==le(raw+4*((size_t)r*4097+s)));words++;if(out==UINT32_C(0x80000000))nz++;}
 CHECK(nz==14);s1c_binding second;CHECK(s1c_bind(&second,p,n));uint32_t out=0;CHECK(s1c_read(&second,49,4095,&out));CHECK(out==le(raw+4*((size_t)49*4097+4095)));
 const size_t shortlens[]={0,1,63,64,100,1367,6171,n-1};
 for(unsigned i=0;i<sizeof shortlens/sizeof shortlens[0];i++){size_t k=shortlens[i];unsigned char*shortbuf=(unsigned char*)malloc(k?k:1);CHECK(shortbuf);memcpy(shortbuf,p,k);bad(&b,shortbuf,k);free(shortbuf);}
 for(unsigned i=0;i<16;i++){memcpy(tmp,p,n);put(tmp+4*i,le(p+4*i)^UINT32_C(0x80000000));bad(&b,tmp,n);}
 for(unsigned i=0;i<52;i++){memcpy(tmp,p,n);tmp[1264+i]=255;bad(&b,tmp,n);}
 for(unsigned i=0;i<13;i++){memcpy(tmp,p,n);put(tmp+1316+4*i,UINT32_C(0x7fc00001));bad(&b,tmp,n);}
 for(unsigned r=0;r<50;r++){
 const unsigned q=64+24*r,mode=le(p+q);
 for(unsigned f=0;f<6;f++){memcpy(tmp,p,n);put(tmp+q+4*f,f==0?4:f==1?UINT32_C(0x40000000):le(p+q+4*f)+1);bad(&b,tmp,n);}
 if(mode==3){const unsigned off=le(p+q+12);memcpy(tmp,p,n);put16(tmp+off,0);bad(&b,tmp,n);memcpy(tmp,p,n);put(tmp+off+2,UINT32_C(0x7fc00000));bad(&b,tmp,n);}
 else{const unsigned off=le(p+q+16),count=le(p+q+20);for(unsigned j=0;j<count;j++){memcpy(tmp,p,n);put16(tmp+off+6*j,4096);bad(&b,tmp,n);memcpy(tmp,p,n);put(tmp+off+6*j+2,UINT32_C(0x7fc00000));bad(&b,tmp,n);}}
 }
 const unsigned idx=le(p+36),blocks=le(p+40);
 for(unsigned j=0;j<blocks;j++){const unsigned off=le(p+idx+4*j);memcpy(tmp,p,n);tmp[off]=255;bad(&b,tmp,n);memcpy(tmp,p,n);tmp[off+1]=33;bad(&b,tmp,n);memcpy(tmp,p,n);put(tmp+idx+4*j,off+1);bad(&b,tmp,n);}
 memcpy(tmp,p,n);tmp[n-1]=1;bad(&b,tmp,n);CHECK(malformed==4040);
 for(unsigned r=0;r<52;r++){bad_read(&b,r,4097);bad_read(&b,r,UINT32_MAX);}
 bad_read(&b,52,0);bad_read(&b,UINT32_MAX,0);CHECK(!s1c_read(&b,0,0,NULL));
 s1c_binding before;memcpy(&before,&b,sizeof b);CHECK(!s1c_read(&b,0,0,&b.format_tag));CHECK(!memcmp(&b,&before,sizeof b));
 memcpy(tmp,p,n);CHECK(!s1c_read(&b,0,0,(uint32_t*)(void*)(p+4)));CHECK(!memcmp(p,tmp,n));
 CHECK(!s1c_read(&b,0,0,(uint32_t*)(void*)(tmp+1)));
 CHECK(!s1c_read(&b,0,0,(uint32_t*)(uintptr_t)(UINTPTR_MAX-1)));
 CHECK(!s1c_bind((s1c_binding*)(void*)(tmp+64),tmp,n));
 CHECK(!s1c_bind((s1c_binding*)(void*)(tmp+1),p,n));
 bad(&b,(const void*)(uintptr_t)(UINTPTR_MAX-63),128);bad(&b,p,SIZE_MAX);
 unsigned char*offset=(unsigned char*)malloc(n+13);CHECK(offset);memcpy(offset+13,p,n);s1c_binding shifted;CHECK(s1c_bind(&shifted,offset+13,n));CHECK(s1c_read(&shifted,48,4095,&out));CHECK(out==le(raw+4*((size_t)48*4097+4095)));
 memcpy(tmp,p,n);const unsigned square=64+24,rle=le(p+square+12);CHECK(le(p+square)==3);put(tmp+rle+2,le(tmp+rle+2)^1);s1c_binding corrupt;CHECK(s1c_bind(&corrupt,tmp,n));CHECK(s1c_read(&corrupt,1,0,&out));CHECK(out!=le(raw+4*4097));
 s1c_binding forged=b;forged.format_tag^=1;out=123;CHECK(!s1c_read(&forged,0,0,&out)&&out==123);
 printf("{\"scope\":\"C compact words/errors/spans\",\"checks\":%u,\"words\":%u,\"negative_zeros\":%u,\"malformed\":%u,\"invalid_read_output_preserved\":%u,\"finite_wrong_data_detected\":true,\"failures\":0}\n",checks,words,nz,malformed,invalid_reads);
 free(offset);free(tmp);free(raw);free(p);return 0;
}
