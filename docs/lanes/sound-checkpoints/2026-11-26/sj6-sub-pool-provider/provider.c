#include "provider.h"
#include <math.h>
#include <stddef.h>
static int valid(const sub_media *m){
  return m && m->pair && m->ctx && (m->channels==1 || m->channels==2) &&
    m->frames && isfinite(m->end) && isfinite(m->start) &&
    m->start>=0 && m->end>=m->start && m->end<=m->frames &&
    (!m->loop || (isfinite(m->first) && isfinite(m->last) && m->first>=m->start &&
     m->last>=m->first && m->last<=m->end));
}
int sub_commit(sub_owner *o,const sub_media *m){
  if(!o || !valid(m))return 0;
  *o=(sub_owner){m,m->start,1,1};return 1;
}
int sub_gate(sub_owner *o,unsigned gate){
  if(!o||!o->media||gate>1)return 0;
  o->gate=gate;return 1;
}
int sub_frame(sub_owner *o,double rate,double ratio,float gain,float *audio){
  if(!o||!o->media||!audio||!isfinite(gain)||rate<=0||ratio<=0||
     !isfinite(rate)||!isfinite(ratio)||!isfinite(o->cursor)||o->cursor<0)return -1;
  if(!o->active || o->cursor>o->media->end){o->active=0;return 0;}
  const sub_media *m=o->media;
  unsigned i=(unsigned)o->cursor;
  const double t=o->cursor-i;
  float result[2];
  for(unsigned c=0;c<m->channels;c++){
    float a=0,b=0;
    if(i<m->frames && (!m->pair(m->ctx,c,i,&a,&b)||!isfinite(a)||!isfinite(b)))return -1;
    if(i+1>=m->frames)b=0;
    result[c]=(float)(fma(1.-t,a,t*b)*(double)gain);
    if(!isfinite(result[c]))return -1;
  }
  double p=fma(rate,ratio,o->cursor);
  if(!isfinite(p))return -1;
  if(m->loop && o->gate==1 && p>m->last)p=(p-m->last)+m->first;
  o->cursor=p;audio[0]=result[0];audio[1]=result[m->channels-1];
  return 1;
}
int sub_pcm16_pair(void *ctx,unsigned c,unsigned i,float *a,float *b){
  const sub_pcm16 *p=ctx;
  if(!p||!p->data||c>=p->channels||i>=p->frames)return 0;
  const int16_t *row=p->data+(size_t)c*p->frames;
  *a=(float)row[i]*0x1p-15f;*b=i+1<p->frames?(float)row[i+1]*0x1p-15f:0;
  return 1;
}
