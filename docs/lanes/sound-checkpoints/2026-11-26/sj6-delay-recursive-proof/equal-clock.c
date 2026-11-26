/* Host feasibility: two lowpass bands, one in complex baseband. */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
enum {CAP=636};
typedef struct {float a1,a2,b0;} coef;
typedef struct {float a,b;} st;
typedef struct {
 uint8_t cell[CAP][6]; coef q[2];st z[6][2];
 unsigned rate,pos,fill,sub,stride,frames,ping;
 float c,s,rotc,rots,dc,ds,time,group,fb,send,dry,prev;
} Band;
_Static_assert(sizeof(Band)<=4096,"host arena");
static const double pi=3.14159265358979323846;
size_t heterodyne_size(void){return sizeof(Band);}
void *heterodyne_new(unsigned rate,float fc){
 if((rate!=44100&&rate!=48000)||fc<200||fc>600)return NULL;
 Band*p=calloc(1,sizeof*p);if(!p)return NULL;
 p->rate=rate;p->stride=rate==44100?35:38;p->c=1;
 p->rotc=(float)cos(2*pi*3000/rate);p->rots=(float)sin(2*pi*3000/rate);
 double w=2*pi*fc/rate,c=cos(w),sn=sin(w),g=0;const double Q[]={.541196100146197,1.306562964876377};
 for(unsigned i=0;i<2;i++){
   const double a=sn/(2*Q[i]),k=1/(1+a);coef*q=&p->q[i];
   *q=(coef){(float)(-2*c*k),(float)((1-a)*k),(float)(.5*(1-c)*k)};
   g+=(1.-q->a2)/(1.+q->a1+q->a2);
 }
 p->group=(float)g;return p;
}
void heterodyne_free(void*p){free(p);}
int heterodyne_params(void*v,float seconds,float fb,float dry,unsigned ping){
 Band*p=v;float t=seconds*p->rate;
 if(!(t>p->group+p->stride)||t>=(CAP-2)*p->stride||!(fb>=0&&fb<=1&&dry>=0&&dry<=1)||ping>1)return 0;
 p->time=t;p->fb=fb;p->dry=dry;p->ping=ping;p->send=ping?powf(fb,.25f):0;
 p->dc=(float)cos(t*(-2*pi*3000/p->rate));p->ds=(float)sin(t*(-2*pi*3000/p->rate));
 return 1;
}
static float filt(Band*p,unsigned b,float x){
 for(unsigned k=0;k<2;k++){
   const coef*q=p->q+k;st*z=&p->z[b][k];
   float y=q->b0*x+z->a;
   z->a=2*q->b0*x-q->a1*y+z->b;z->b=q->b0*x-q->a2*y;x=y;
 }
 return x;
}
static uint8_t enc(float x){
 unsigned sign=x<0?128u:0u,n=(unsigned)(fminf(1.f,fabsf(x))*32635.f+.5f)+132u,e=0;
 for(unsigned v=n>>8;v;v>>=1)++e;
 return sign|(e<<4)|((n>>(e+3))&15);
}
static float dec(unsigned x){
 unsigned e=(x>>4)&7;float y=(float)((((x&15)<<3)+132)<<e)-132.f;
 return (x&128?-y:y)*(1.f/32635.f);
}
void heterodyne_process(void*v,size_t n,const float*l,const float*r,float*ol,float*orr){
 Band*p=v;
 if(p->time==0){memcpy(ol,l,n*sizeof *l);memcpy(orr,r,n*sizeof*r);return;}
 for(size_t j=0;j<n;j++){
  float age=(p->time-p->group+p->stride-p->sub-1)/p->stride;
  unsigned q=(unsigned)age,a=(p->pos+CAP-q)%CAP,b=a?a-1:CAP-1;
  float f=age-q,c=p->c,s=p->s,rc=c*p->dc-s*p->ds,rs=c*p->ds+s*p->dc,z[2]={0};
  if(q&&q<=p->fill)for(unsigned i=0;i<2;i++){
   float band[3];for(unsigned k=0;k<3;k++){
    unsigned h=3*i+k;float x=dec(p->cell[a][h]),y=q<p->fill?dec(p->cell[b][h]):0.f;band[k]=x+(y-x)*f;
   }
   z[i]=band[0]+2*(band[1]*rc-band[2]*rs);
  }
  float x[2];
  if(p->ping){x[0]=fmaf(p->fb,p->prev,(l[j]+r[j])*.5f);x[1]=z[0]*p->send;}
  else{x[0]=fmaf(p->fb,z[0],l[j]);x[1]=fmaf(p->fb,z[1],r[j]);}
  p->prev=z[1];
  for(unsigned i=0;i<2;i++){
   float lo=filt(p,3*i,x[i]),hi=filt(p,3*i+1,x[i]*c),hq=filt(p,3*i+2,-x[i]*s);
   if(p->sub+1==p->stride){p->cell[p->pos][3*i]=enc(lo);p->cell[p->pos][3*i+1]=enc(hi);p->cell[p->pos][3*i+2]=enc(hq);}
  }
  if(++p->sub==p->stride){p->sub=0;if(++p->pos==CAP)p->pos=0;if(p->fill<CAP)p->fill++;}
  if(p->dry==1.f){*ol++=l[j];*orr++=r[j];}
  else {*ol++=fmaf(1-p->dry,z[0],p->dry*l[j]);*orr++=fmaf(1-p->dry,z[1],p->dry*r[j]);}
  p->c=c*p->rotc-s*p->rots;p->s=s*p->rotc+c*p->rots;
  if(!(++p->frames&1023u)){float g=1/sqrtf(p->c*p->c+p->s*p->s);p->c*=g;p->s*=g;}
 }
}
