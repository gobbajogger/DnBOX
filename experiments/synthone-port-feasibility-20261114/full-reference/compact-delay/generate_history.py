from pathlib import Path
import hashlib,json
r=Path(__file__).resolve().parent;src=(r.parent/'upstream/AudioKit/Core/Soundpipe/modules/vdelay.c').read_text();assert hashlib.sha1(('blob '+str(len(src.encode()))+'\0').encode()+src.encode()).hexdigest()=='45059d64201c418523a4af843f84d4d23a898276'
body=src[src.index('    int32_t maxd, indx;',src.index('int sp_vdelay_compute')):src.index('\nint sp_vdelay_reset')];body=body[:body.rfind('}')]
body=body.replace('buf[indx] = *in + p->prev*p->feedback;','buf[s->physical_left] = *in + p->prev*p->feedback;')
for n in range(4):body=body.replace('buf[v%d]'%n,'read_age(s,v%d)'%n)
code='''// Derived from authenticated Soundpipe vdelay.c; original numeric body/order.
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "history_delay.h"
#define MAGIC 0x53314844u
static int rate_ok(int r){return r==44100||r==48000||r==96000;}
static int overlap(const void*a,size_t an,const void*b,size_t bn){uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;if(an>UINTPTR_MAX-x||bn>UINTPTR_MAX-y)return 1;return x<y+bn&&y<x+an;}
size_t s1_history_words(int rate,float max_delay){if(!rate_ok(rate)||!isfinite(max_delay)||max_delay<=0.f||max_delay>8.f)return 0;return (size_t)ceil((double)max_delay*rate)+4;}
static int valid(const s1_history_delay*s){if(!s||s->magic!=MAGIC||!(s->v.sr==44100.f||s->v.sr==48000.f||s->v.sr==96000.f)||s->v.maxdel!=10.f||!s->v.buf.ptr)return 0;size_t n=s1_history_words((int)s->v.sr,s->admitted_max_delay);uint32_t old=(uint32_t)(s->v.maxdel*s->v.sr);return n&&s->physical_words==n&&s->v.buf.size==n*sizeof(float)&&s->physical_left<n&&s->v.left<old&&s->written<=old;}
int s1_history_init(s1_history_delay*s,int rate,float*storage,size_t words,float max_delay){size_t n=s1_history_words(rate,max_delay);if(!s||!storage||!n||words!=n||((uintptr_t)storage%_Alignof(float))||overlap(s,sizeof*s,storage,n*sizeof(float)))return 0;s1_history_delay next={0};next.v.sr=rate;next.v.del=5.f;next.v.maxdel=10.f;next.v.buf.ptr=storage;next.v.buf.size=n*sizeof(float);next.physical_words=n;next.admitted_max_delay=max_delay;next.magic=MAGIC;memset(storage,0,n*sizeof(float));*s=next;return 1;}
static uint32_t age(const s1_history_delay*s,int original){int d=(int)s->v.left-original;int old=(int)(s->v.maxdel*s->v.sr);return (uint32_t)(d<0?d+old:d);}
static float read_age(const s1_history_delay*s,int original){uint32_t d=age(s,original);if(d>=s->written)return 0.f;uint32_t q=(s->physical_left+s->physical_words-d)%s->physical_words;return ((const float*)s->v.buf.ptr)[q];}
static int coords_available(const sp_data*sp,const s1_history_delay*s){float fv=s->v.del*(-1.0*sp->sr);int v1=(int)fv;fv-=(float)v1;v1+=(int)s->v.left;int n=(int)(s->v.maxdel*s->v.sr);if(v1<0||fv<0){++fv;--v1;while(v1<0)v1+=n;}else while(v1>=n)v1-=n;int v2=v1==n-1?0:v1+1;int vi[4]={v1==0?n-1:v1-1,v1,v2,v2==n-1?0:v2+1};uint32_t known=s->written<(uint32_t)n?s->written+1:(uint32_t)n;for(int i=0;i<4;++i){uint32_t d=age(s,vi[i]);if(d<known&&d>=s->physical_words)return 0;}return 1;}
static int original_numeric_body(sp_data*sp,s1_history_delay*s,const float*in,float*out){sp_vdelay*p=&s->v;
'''+body+'''\n}
int s1_history_compute(sp_data*sp,s1_history_delay*s,const float*in,float*out){if(!sp||!s||!in||!out||overlap(sp,sizeof*sp,s,sizeof*s)||!valid(s))return 0;size_t bytes=s->physical_words*sizeof(float);if(overlap(sp,sizeof*sp,s->v.buf.ptr,bytes)||overlap(sp,sizeof*sp,in,sizeof*in)||overlap(sp,sizeof*sp,out,sizeof*out)||overlap(in,4,out,4)||overlap(in,4,s,sizeof*s)||overlap(out,4,s,sizeof*s)||overlap(in,4,s->v.buf.ptr,bytes)||overlap(out,4,s->v.buf.ptr,bytes))return 0;if(sp->sr!=(int)s->v.sr||!isfinite(s->v.del)||s->v.del<0||s->v.del>s->admitted_max_delay||!isfinite(s->v.prev)||!isfinite(s->v.feedback)||!isfinite(*in)||!coords_available(sp,s))return 0;uint32_t old=(uint32_t)(s->v.maxdel*s->v.sr);if(s->written<old)++s->written;int result=original_numeric_body(sp,s,in,out);if(++s->physical_left==s->physical_words)s->physical_left=0;return result;}
int s1_history_reset(s1_history_delay*s){if(!valid(s))return 0;s->v.left=0;s->physical_left=0;s->written=0;memset(s->v.buf.ptr,0,s->physical_words*sizeof(float));return SP_OK;}
'''
(r/'history_delay.c').write_text(code);(r/'DERIVATION.json').write_text(json.dumps(dict(source_blob='45059d64201c418523a4af843f84d4d23a898276',arithmetic='original compute numeric body/order retained; physical write and4read lookups alone replaced; original virtual maxd/index retained',limits=['Only fullS1 APP control/lifecycle hypothesis, not general vdelay or hotrestore acceptance','No storage placement/CPU/native/fit qualification','Fails if required old history unavailable, no fakezero'],sha256=hashlib.sha256(code.encode()).hexdigest()),indent=2)+'\n');print('Generated guarded history adapter',len(code))
