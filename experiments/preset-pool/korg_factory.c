/* SPDX-License-Identifier: GPL-3.0-only */
#include "korg_factory.h"
static const dnbank *bound[2];
static char names[2][48];
static unsigned fields(unsigned e){return e?18u:22u;}
static unsigned presets(unsigned e){return e?30u:81u;}
typedef struct {float *out;unsigned next,limit;} stage;
static void collect(void *ctx,unsigned id,uint32_t word){
 stage *s=ctx;
 if(s->next>=s->limit||id!=s->next||(word>0x3f800000u&&word!=0x80000000u)){s->next=255;return;}
 union {uint32_t u;float f;} bits={word};s->out[s->next++]=bits.f;
}
static int values(const dnbank *b,unsigned engine,unsigned p,float *out,char *name){
 stage s={out,0,fields(engine)};
 return dnbank_read(b,p,name,collect,&s)==(int)s.limit&&s.next==s.limit;
}
int dn_korg_bind(unsigned e,const dnbank *b){
 if(e>=2)return 0;
 if(!b){bound[e]=0;return 1;}
 if(!b->pack||b->engine!=e||b->programs!=presets(e))return 0;
 float v[22];char name[48];uint8_t id[8];
 for(unsigned i=0;i<b->programs;i++){
  if(!dnbank_preset_id(b,i,id)||id[0]!=i+1)return 0;
  for(unsigned j=1;j<8;j++)if(id[j])return 0;
  if(!values(b,e,i,v,name))return 0;
 }
 bound[e]=b;return 1;
}
int dn_korg_values(unsigned e,unsigned p,float *out){
 if(e>=2||!bound[e]||p>=presets(e)||!out)return 0;
 char name[48];return values(bound[e],e,p,out,name);
}
const char *dn_korg_name(unsigned e,unsigned p){
 if(e>=2||!bound[e]||p>=presets(e))return 0;
 return dnbank_read(bound[e],p,names[e],0,0)==(int)fields(e)?names[e]:0;
}
