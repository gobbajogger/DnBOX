#include "remote_wire.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int ref_dnr2_init(dnr2_core*,const dnr2_host*,uint32_t,const uint32_t*);
void ref_dnr2_changed(dnr2_core*);
dnr2_reply ref_dnr2_handle(dnr2_core*,const dnr2_command*,uint32_t);
int ref_dnr2_decode(dnr2_command*,const uint8_t*,size_t);
int ref_dnr2_encode_command(uint8_t*,const dnr2_command*);
int ref_dnr2_encode_reply(uint8_t*,unsigned,const dnr2_reply*);
typedef struct {uint32_t value[3],sets,downs,ups,cookie;} state;
static int rd(void*p,uint16_t id,uint8_t*k,uint32_t*v){state*s=p;if(id<1||id>3)return 0;*k=id;*v=s->value[id-1];return 1;}
static int st(void*p,uint16_t id,uint8_t k,uint32_t v){state*s=p;if(id<1||id>2||k!=id||(id==1&&v>127))return 0;s->value[id-1]=v;s->sets++;return 2;}
static int down(void*p,uint16_t id,uint32_t v,uint32_t*c){state*s=p;if(id!=3||v>127||!v)return 0;*c=++s->cookie;s->downs++;return 1;}
static void up(void*p,uint32_t c){state*s=p;assert(c);s->ups++;}
static uint32_t seed=0x73f2a619;static uint32_t rnd(void){seed^=seed<<13;seed^=seed>>17;seed^=seed<<5;return seed;}
#define EQ(F) assert(a.F==b.F)
static void replyeq(dnr2_reply a,dnr2_reply b){assert(!memcmp(a.unit_high,b.unit_high,12));EQ(unit);EQ(client);EQ(epoch);EQ(request);EQ(revision);EQ(value);EQ(control);EQ(status);EQ(kind);}
static void cmdeq(dnr2_command a,dnr2_command b){assert(!memcmp(a.unit_high,b.unit_high,12));EQ(unit);EQ(client);EQ(epoch);EQ(request);EQ(revision);EQ(value);EQ(control);EQ(token);EQ(version);EQ(op);}
int main(void){
 state sa={0},sb={0};dnr2_core a,b;dnr2_host ha={&sa,rd,st,down,up,123,3},hb={&sb,rd,st,down,up,123,3};uint32_t high[]={2,3,4};
 assert(ref_dnr2_init(&a,&ha,1,high)==dnr2_init(&b,&hb,1,high));uint32_t now=0,request=1;dnr2_command saved={0};unsigned statuses[9]={0};
 for(unsigned i=0;i<100000;i++){
  if(i%97==0){ref_dnr2_changed(&a);dnr2_changed(&b);}
  now+=rnd()%11;dnr2_command c={0};c.unit=1;memcpy(c.unit_high,high,12);c.client=7;c.epoch=a.epoch;c.request=request++;c.version=2;c.op=(uint8_t)(rnd()%8);c.revision=a.revision;
  if(!a.active)c.op=DNR2_OPEN,c.epoch=0,c.revision=0;
  if(c.op==DNR2_HELLO)c.unit=0,memset(c.unit_high,0,12),c.epoch=0,c.revision=0;
  if(c.op==DNR2_OPEN)c.epoch=0,c.revision=0;
  if(c.op==DNR2_READ||c.op==DNR2_SET||c.op==DNR2_DOWN||c.op==DNR2_UP)c.control=1+rnd()%4;
  if(c.op==DNR2_SET)c.value=rnd()%150;
  if(c.op==DNR2_DOWN)c.control=3,c.token=1+rnd()%70,c.value=1+rnd()%140;
  if(c.op==DNR2_UP)c.control=3,c.token=1+rnd()%70;
  if(i%11==0)c=saved;else saved=c;
  if(i%31==0)c.revision^=1;
  dnr2_reply x=ref_dnr2_handle(&a,&c,now),y=dnr2_handle(&b,&c,now);replyeq(x,y);assert(x.status<9);statuses[x.status]++;assert(!memcmp(&sa,&sb,sizeof sa));
  EQ(active);EQ(nheld);EQ(revision);EQ(epoch);EQ(serial);EQ(client);EQ(last_request);EQ(deadline);EQ(cache_valid);
  assert(!memcmp(a.unit_high,b.unit_high,12));EQ(unit);
  for(unsigned j=0;j<a.nheld;j++){assert(a.held[j].cookie==b.held[j].cookie);assert(a.held[j].token==b.held[j].token);assert(a.held[j].control==b.held[j].control);}
  if(a.cache_valid){cmdeq(a.cached_command,b.cached_command);replyeq(a.cached_reply,b.cached_reply);}
  uint8_t p[51],q[51];memset(p,0xa5,51);memset(q,0xa5,51);assert(ref_dnr2_encode_command(p,&c)==dnr2_encode_command(q,&c));assert(!memcmp(p,q,51));
  if(i%3==0)p[rnd()%51]=(uint8_t)rnd();
  dnr2_command d={0},e={0};int v=ref_dnr2_decode(&d,p,51),w=dnr2_decode(&e,p,51);assert(v==w);if(v)cmdeq(d,e);
  memset(p,0xa5,51);memset(q,0xa5,51);assert(ref_dnr2_encode_reply(p,c.op,&x)==dnr2_encode_reply(q,c.op,&y));assert(!memcmp(p,q,51));
 }
 state random_final=sa;
 /* Directed capacity/exhaustion/expiry boundaries absent from random coverage. */
 sa=(state){0};sb=(state){0};assert(ref_dnr2_init(&a,&ha,1,high)&&dnr2_init(&b,&hb,1,high));
 dnr2_command c={0};memcpy(c.unit_high,high,12);c.unit=1;c.client=7;c.version=2;c.request=1;c.op=DNR2_OPEN;
 dnr2_reply x=ref_dnr2_handle(&a,&c,0),y=dnr2_handle(&b,&c,0);replyeq(x,y);assert(x.status==DNR2_OK);c.epoch=x.epoch;
 for(unsigned i=0;i<=DNR2_MAX_HELD;i++) {c.request++;c.op=DNR2_DOWN;c.control=3;c.token=i+1;c.value=100;c.revision=a.revision;x=ref_dnr2_handle(&a,&c,1);y=dnr2_handle(&b,&c,1);replyeq(x,y);assert(x.status==(i<DNR2_MAX_HELD?DNR2_OK:DNR2_FULL));}
 c.request++;c.op=DNR2_SET;c.control=1;c.token=0;c.value=4;a.revision=b.revision=DNR2_MAX_SERIAL;c.revision=DNR2_MAX_SERIAL;
 x=ref_dnr2_handle(&a,&c,2);y=dnr2_handle(&b,&c,2);replyeq(x,y);assert(x.status==DNR2_EXHAUSTED);
 c.request++;c.op=DNR2_KEEPALIVE;c.control=c.value=0;x=ref_dnr2_handle(&a,&c,5000);y=dnr2_handle(&b,&c,5000);replyeq(x,y);assert(x.status==DNR2_STALE && sa.ups==DNR2_MAX_HELD && sb.ups==DNR2_MAX_HELD);
 a.serial=b.serial=DNR2_MAX_SERIAL;c.request++;c.op=DNR2_OPEN;c.epoch=c.revision=0;x=ref_dnr2_handle(&a,&c,5001);y=dnr2_handle(&b,&c,5001);replyeq(x,y);assert(x.status==DNR2_EXHAUSTED);
 uint8_t good[51],bad[51];c.version=2;c.op=DNR2_READ;c.control=1;c.unit=DNR2_MAX_SERIAL;assert(ref_dnr2_encode_command(good,&c));
 for(unsigned at=0;at<51;at++)for(unsigned value=0;value<256;value++){memcpy(bad,good,51);bad[at]=(uint8_t)value;dnr2_command d={0},e={0};int v=ref_dnr2_decode(&d,bad,51),w=dnr2_decode(&e,bad,51);assert(v==w);if(v)cmdeq(d,e);}
 for(size_t len=0;len<=60;len++){dnr2_command d={0},e={0};int v=ref_dnr2_decode(&d,good,len),w=dnr2_decode(&e,good,len);assert(v==w);if(v)cmdeq(d,e);}
 assert(!ref_dnr2_decode(0,good,51)&&!dnr2_decode(0,good,51));
 assert(!ref_dnr2_decode(&c,0,51)&&!dnr2_decode(&c,0,51));
 puts("PASS active held/cached semantic fields, decode lengths0..60 and null guards");
 puts("PASS directed67-owner capacity/full, revision+lease exhaustion, expiry cleanup,13056 single-byte decode mutations");
 printf("PASS 100000 deterministic transactions + command/reply wire and mutated decode comparisons; callbacks SET=%u DOWN=%u UP=%u\n",random_final.sets,random_final.downs,random_final.ups);
 printf("statuses");for(unsigned i=0;i<9;i++)printf(" %u",statuses[i]);puts("");return 0;
}
