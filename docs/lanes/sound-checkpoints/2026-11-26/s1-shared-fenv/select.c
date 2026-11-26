#include "dnbank.h"
#include <stdint.h>
#include <string.h>
enum { REJECT=0, STAGED_NOT_ADOPTED=1 };
typedef struct { uint32_t words[220]; uint8_t ids[220]; char name[48]; unsigned n; } stage;
typedef struct { stage *out; const uint8_t *schema; unsigned n, max; int bad; } collect;
static void emit(void *p,unsigned id,uint32_t w){
 collect *c=p;
 unsigned i=c->out->n;
 if(c->bad || i>=c->n || i>=220 || id>=c->max || id!=c->schema[i] || (w&0x7f800000u)==0x7f800000u){c->bad=1;return;}
 c->out->ids[i]=(uint8_t)id; c->out->words[i]=w; c->out->n=i+1;
}
/* The host owns and authenticates the immutable image before opening.
 * This selector returns staging status only. There is no adoption API. */
int stage_program(const uint8_t *buf,size_t size,unsigned kind,
 const uint8_t bank_id[8],const uint8_t preset_id[8],const uint8_t *schema,unsigned n,stage *out){
 dnbank b; uint8_t id[8]; unsigned p;
 if(!out)return REJECT;
 memset(out,0,sizeof *out);
 if((kind!=2&&kind!=3)||!bank_id||!preset_id||!schema||!n||n>220||
    (kind==2&&n!=144)||!dnbank_open(&b,buf,size)||
    b.engine!=kind||memcmp(buf+16,bank_id,8)||!dnbank_validate(&b))return REJECT;
 for(p=0;p<b.programs;p++){
   if(!dnbank_preset_id(&b,p,id))return REJECT;
   if(!memcmp(id,preset_id,8))break;
 }
 if(p==b.programs)return REJECT;
 collect c={out,schema,n,kind==2?150u:220u,0};
 if(dnbank_read(&b,p,out->name,emit,&c)==(int)n && !c.bad && out->n==n)return STAGED_NOT_ADOPTED;
 memset(out,0,sizeof *out);
 return REJECT;
}
