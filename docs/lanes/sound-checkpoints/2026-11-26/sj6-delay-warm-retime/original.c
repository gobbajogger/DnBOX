/* Host-only behavioral port of the Build9 StereoDelay core. */
#include <math.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

struct sj6_delay {
    float *ring;
    unsigned capacity, write;
    float read, feedback, dry, right_last, right_send;
    int ping;
    double rate;
};

void *sj6_new(double rate) {
    if (!(rate >= 8000 && rate <= 384000)) return NULL;
    struct sj6_delay *x = calloc(1, sizeof(*x));
    if (!x) return NULL;
    x->capacity=(unsigned)(rate*3.0);
    x->ring=calloc(2*x->capacity,sizeof(float));
    if (!x->ring) { free(x); return NULL; }
    x->rate=rate;
    return x;
}

void sj6_free(void *ptr) {
    struct sj6_delay *x=ptr;
    if(x){free(x->ring);free(x);}
}

void sj6_clear(void *ptr) {
    struct sj6_delay *x=ptr;
    memset(x->ring,0,2*x->capacity*sizeof(float));
    x->right_last=0;
}

void sj6_params(void *ptr, double sec, float feedback, float dry, int ping) {
    struct sj6_delay *x=ptr;
    /* The AU wrapper accepts seconds; its inner core accepts milliseconds. */
    const double ms=(double)(float)sec*1000.0;
    float d=(float)(x->rate*fmin(3000.,fmax(0.,ms))/1000.);
    float cap=(float)x->capacity;
    x->read=(float)x->write-d;
    if(x->read<0.f) x->read+=cap;
    if(x->read>=cap) x->read-=cap;
    x->feedback=fminf(1.f,fmaxf(0.f,feedback));
    x->dry=fminf(1.f,fmaxf(0.f,dry));
    x->ping=ping;
    x->right_send=ping?powf(x->feedback,.25f):0.f;
}

void sj6_process(void *ptr, size_t n, const float *l, const float *r,
                 float *ol, float *orr) {
    struct sj6_delay *x=ptr;
    float *hl=x->ring,*hr=hl+x->capacity;
    unsigned pos=x->write;
    float read=x->read,last=x->right_last;
    const float wet=1.f-x->dry;
    while(n--){
        unsigned a=(unsigned)read,b=a+1==x->capacity?0:a+1;
        float f=read-(float)a;
        float dl=fmaf(1.f-f,hl[a],f*hl[b]);
        float dr=fmaf(1.f-f,hr[a],f*hr[b]);
        float inl=*l++,inr=*r++;
        if(x->ping){
            hl[pos]=fmaf(x->feedback,last,(inl+inr)*.5f);
            hr[pos]=dl*x->right_send;
        }else{
            hl[pos]=fmaf(x->feedback,dl,inl);
            hr[pos]=fmaf(x->feedback,dr,inr);
        }
        last=dr;
        *ol++=fmaf(wet,dl,x->dry*inl);
        *orr++=fmaf(wet,dr,x->dry*inr);
        if(++pos==x->capacity) pos=0;
        read+=1.f;
        if(read>=(float)x->capacity) read-=(float)x->capacity;
    }
    x->write=pos;x->read=read;x->right_last=last;
}
