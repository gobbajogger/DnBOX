// SPDX-License-Identifier: GPL-3.0-only
// New public read-boundary tests: missing binding/tag/header/span checks, overreads,
// writes to payload, invalid coordinates and failure-output mutation must fail.
#include "compact.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/mman.h>
#include <unistd.h>
#define CHECK(x)do{++checks;if(!(x)){fprintf(stderr,"FAIL:%d %s\n",__LINE__,#x);exit(1);}}while(0)
static unsigned checks,rejects;
static unsigned char*load(const char*p,size_t*n){FILE*f=fopen(p,"rb");CHECK(f);CHECK(!fseek(f,0,SEEK_END));*n=(size_t)ftell(f);CHECK(!fseek(f,0,SEEK_SET));unsigned char*b=malloc(*n);CHECK(b);CHECK(fread(b,1,*n,f)==*n);CHECK(!fclose(f));return b;}
static uint32_t le(const unsigned char*p){return(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static void bad(const s1c_binding*b,unsigned row,unsigned sample){uint32_t out=0xdeadbeefU;CHECK(!s1c_read(b,row,sample,&out));CHECK(out==0xdeadbeefU);++rejects;}
int main(int ac,char**av){CHECK(ac==3);size_t n,rn;unsigned char*p=load(av[1],&n),*raw=load(av[2],&rn);CHECK(n==165292&&rn==852176);size_t pg=(size_t)sysconf(_SC_PAGESIZE),pages=(n+pg-1)/pg,cap=pages*pg;unsigned char*map=mmap(0,cap+2*pg,PROT_NONE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);CHECK(map!=MAP_FAILED);CHECK(!mprotect(map+pg,cap,PROT_READ|PROT_WRITE));unsigned char*data=map+pg+cap-n;memcpy(data,p,n);CHECK(!mprotect(map+pg,cap,PROT_READ));s1c_binding b;CHECK(s1c_bind(&b,data,n));
 unsigned nz=0;for(unsigned row=0;row<52;row++)for(unsigned sample=0;sample<4097;sample++){uint32_t out;CHECK(s1c_read(&b,row,sample,&out));CHECK(out==le(raw+4*((size_t)row*4097+sample)));nz+=out==0x80000000U;}CHECK(nz==14);
 for(unsigned i=0;i<52;i++){bad(&b,i,4097);bad(&b,i,UINT_MAX);}bad(&b,52,0);bad(&b,UINT_MAX,0);bad(0,0,0);bad(0,UINT_MAX,UINT_MAX);
 unsigned char misbuf[sizeof(s1c_binding)+16];uintptr_t mis=(uintptr_t)misbuf;while(mis%__alignof__(s1c_binding)==0)mis++;bad((const s1c_binding*)mis,0,0);bad((const s1c_binding*)(UINTPTR_MAX-sizeof(s1c_binding)/2),0,0);
 s1c_binding wrong=b;wrong.format_tag^=1;wrong.bytes=map;bad(&wrong,0,0);wrong=b;wrong.bytes=0;bad(&wrong,0,0);wrong=b;wrong.byte_count=n-1;bad(&wrong,0,0);wrong=b;wrong.bytes=map;wrong.byte_count=63;bad(&wrong,0,0);
 s1c_binding before=b;CHECK(!s1c_read(&b,0,0,&b.format_tag));CHECK(!memcmp(&b,&before,sizeof b));CHECK(!s1c_read(&b,0,0,(uint32_t*)(void*)data));CHECK(!s1c_read(&b,0,0,(uint32_t*)(void*)(data+1)));CHECK(!s1c_read(&b,0,0,(uint32_t*)(UINTPTR_MAX-1)));CHECK(!s1c_read(&b,0,0,0));
 CHECK(!memcmp(data,p,n));CHECK(!munmap(map,cap+2*pg));free(p);free(raw);printf("{\"checks\":%u,\"invalid_reads_preserved\":%u,\"words\":213044,\"negative_zero\":%u,\"guard_page\":true,\"payload_readonly\":true}\n",checks,rejects,nz);}
