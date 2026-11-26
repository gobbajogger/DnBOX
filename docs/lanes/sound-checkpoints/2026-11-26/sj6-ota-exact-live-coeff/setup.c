#include <math.h>
#include <stdint.h>
/* A caller bound to one immutable rate and actual supported Main OTA uses
 * this descriptor; these functions do not decide OTA or source ownership. */
typedef struct {float loghz,current,target,step; uint32_t remaining, initialized;} MainCoeff;
static float minimum(float a,float b){return a<b?a:b;}
static float maximum(float a,float b){return a>b?a:b;}
void main_request(MainCoeff *s,float hz,float fs,int32_t n){
    const float f=maximum(3.f,minimum(hz,minimum(30000.f,fs*.9f)));
    const float loghz=log2f(f);
    if(s->initialized && s->loghz==loghz)return;
    const float g=tanf((exp2f(loghz)*3.14159265358979323846f)/(fs+fs));
    s->loghz=loghz;
    if(!s->initialized){s->current=s->target=g;s->step=0.f;s->remaining=0;s->initialized=1;return;}
    s->target=g; s->remaining=n>1?(uint32_t)n:1u;
    s->step=(g-s->current)/(float)s->remaining;
}
float main_frame(MainCoeff *s){
    if(s->remaining){
        s->current+=s->step;
        if(--s->remaining==0)s->current=s->target;
    }
    return s->current;
}
