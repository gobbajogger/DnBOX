#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "compact.h"
extern int s1c_original_read(const s1c_binding*,unsigned,unsigned,uint32_t*);
static uint64_t now(clockid_t id){struct timespec t;if(clock_gettime(id,&t))exit(3);return (uint64_t)t.tv_sec*1000000000+t.tv_nsec;}
int main(int argc,char**argv){if(argc!=2)return 3;FILE*f=fopen(argv[1],"rb");if(!f)return 3;unsigned char*b=malloc(165292);if(!b||fread(b,1,165292,f)!=165292)return 3;fclose(f);s1c_binding binding;if(!s1c_bind(&binding,b,165292))return 3;
for(unsigned run=0;run<4;run++){int batch=run==1||run==2;uint64_t sum=0;uint64_t wall=now(CLOCK_MONOTONIC),cpu=now(CLOCK_THREAD_CPUTIME_ID);unsigned count=0;
 for(unsigned repeat=0;repeat<8;repeat++)for(unsigned row=0;row<52;row++)for(unsigned pos=0;pos<4097;pos++){uint32_t w[4];unsigned row1=(row+repeat)%52,pos1=(pos+1)%4097;int ok;
 if(batch)ok=s1c_read_quad(&binding,row,row1,pos,pos1,w);else ok=s1c_original_read(&binding,row,pos,w)&&s1c_original_read(&binding,row1,pos,w+1)&&s1c_original_read(&binding,row,pos1,w+2)&&s1c_original_read(&binding,row1,pos1,w+3);
 if(!ok)return 4;for(unsigned j=0;j<4;j++)sum+=w[j];count++;
 }
 cpu=now(CLOCK_THREAD_CPUTIME_ID)-cpu;wall=now(CLOCK_MONOTONIC)-wall;printf("{\"run\":%u,\"batch\":%d,\"quads\":%u,\"sum\":%llu,\"cpu_ns\":%llu,\"wall_ns\":%llu}\n",run,batch,count,(unsigned long long)sum,(unsigned long long)cpu,(unsigned long long)wall);fflush(stdout);
}free(b);return 0;}
