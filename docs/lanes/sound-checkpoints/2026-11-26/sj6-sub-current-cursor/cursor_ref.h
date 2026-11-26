#pragma once
#include <stddef.h>
#include <math.h>
/* Reference-only provider. No embedded samples or estimated phase. */
typedef struct {
    const float *planar;
    unsigned length, channels;
    float end,loop_start,loop_stop;
    unsigned loop;
} sub_zone;
int cursor_ref(const sub_zone *z,double *cursor,double rate,double note_ratio,
               float gain,unsigned gate,float out[2]){
    if(!z || !z->planar || !cursor || !out || !z->length ||
       (z->channels!=1 && z->channels!=2) || !isfinite(*cursor) ||
       *cursor<0 || *cursor>z->end || z->end>z->length ||
       !isfinite(rate)||rate<=0||!isfinite(note_ratio)||note_ratio<=0 ||
       !isfinite(gain) || (z->loop && (!isfinite(z->loop_start)||!isfinite(z->loop_stop)||
        z->loop_start<0||z->loop_stop<z->loop_start||z->loop_stop>z->end))) return 0;
    const unsigned i=(unsigned)*cursor;
    const double t=*cursor-i;
    for(unsigned c=0;c<2;c++){
        const float *s=z->planar+(z->channels==2?c*z->length:0);
        const double a=i<z->length?s[i]:0,b=i+1<z->length?s[i+1]:0;
        out[c]=(float)(fma(1-t,a,t*b)*(double)gain);
    }
    double p=fma(rate,note_ratio,*cursor);
    if(z->loop && gate==1 && p>z->loop_stop)p=(p-z->loop_stop)+z->loop_start;
    *cursor=p;
    return 1;
}
