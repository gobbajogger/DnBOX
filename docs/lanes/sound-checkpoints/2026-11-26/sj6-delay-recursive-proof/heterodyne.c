/* Host feasibility: two lowpass bands, one in complex baseband. */
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
enum {CL=950, CH=475};
typedef struct {float a1,a2,b0;} coef;
typedef struct {float a,b;} st;
typedef struct {
 uint8_t low[CL][2],high[CH][4]; coef q[2][2];st z[6][2];
 unsigned rate,pos[2],fill[2],sub[2],stride[2],frames,ping;
 float c,s,rotc,rots,dc,ds,time,group[2],fb,send,dry,prev;
} Band;
_Static_assert(sizeof(Band)<=4096,"host arena");
static const double pi=3.14159265358979323846;
size_t heterodyne_size(void){return sizeof(Band);}
void *heterodyne_new(unsigned rate,float fc){
 if((rate!=44100&&rate!=48000)||fc<200||fc>600)return NULL;
 Band*p=calloc(1,sizeof*p);if(!p)return NULL;
 p->rate=rate;p->stride[0]=rate==44100?24:26;p->stride[1]=2*p->stride[0];p->c=1;
 p->rotc=(float)cos(2*pi*3000/rate);p->rots=(float)sin(2*pi*3000/rate);
 for(unsigned band=0;band<2;band++){
 double w=2*pi*(band?fc:805.f)/rate,c=cos(w),sn=sin(w),g=0;const double Q[]={.541196100146197,1.306562964876377};
 for(unsigned i=0;i<2;i++){
   const double a=sn/(2*Q[i]),k=1/(1+a);coef*q=&p->q[band][i];
   *q=(coef){(float)(-2*c*k),(float)((1-a)*k),(float)(.5*(1-c)*k)};
   g+=(1.-q->a2)/(1.+q->a1+q->a2);
 }
 p->group[band]=(float)g;
 }
 return p;
}
void heterodyne_free(void*p){free(p);}
int heterodyne_params(void*v,float seconds,float fb,float dry,unsigned ping){
 Band*p=v;float t=seconds*p->rate;
 if(!(t>fmaxf(p->group[0]+p->stride[0],p->group[1]+p->stride[1]))||t>=(CH-2)*p->stride[1]||!(fb>=0&&fb<=1&&dry>=0&&dry<=1)||ping>1)return 0;
 p->time=t;p->fb=fb;p->dry=dry;p->ping=ping;p->send=ping?powf(fb,.25f):0;
 p->dc=(float)cos(t*(-2*pi*3000/p->rate));p->ds=(float)sin(t*(-2*pi*3000/p->rate));
 return 1;
}
static float filt(Band*p,unsigned b,float x){
 for(unsigned k=0;k<2;k++){
   const coef*q=&p->q[b%3!=0][k];st*z=&p->z[b][k];
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
  float c=p->c,s=p->s,rc=c*p->dc-s*p->ds,rs=c*p->ds+s*p->dc,z[2]={0};
  for(unsigned k=0;k<2;k++){
   unsigned cap=k?CH:CL;float age=(p->time-p->group[k]+p->stride[k]-p->sub[k]-1)/p->stride[k];
   unsigned q=(unsigned)age,a=(p->pos[k]+cap-q)%cap,b=a?a-1:cap-1;float f=age-q;
   if(!q||q>p->fill[k])continue;
   for(unsigned i=0;i<2;i++){
    if(!k){float v=dec(p->low[a][i]),w=q<p->fill[k]?dec(p->low[b][i]):0;z[i]+=v+(w-v)*f;}
    else {
      float x=dec(p->high[a][2*i]),y=dec(p->high[a][2*i+1]);
      float j=q<p->fill[k]?dec(p->high[b][2*i]):0,h=q<p->fill[k]?dec(p->high[b][2*i+1]):0;
      z[i]+=2*((x+(j-x)*f)*rc-(y+(h-y)*f)*rs);
    }
   }
  }
  float x[2];
  if(p->ping){x[0]=fmaf(p->fb,p->prev,(l[j]+r[j])*.5f);x[1]=z[0]*p->send;}
  else{x[0]=fmaf(p->fb,z[0],l[j]);x[1]=fmaf(p->fb,z[1],r[j]);}
  p->prev=z[1];
  for(unsigned i=0;i<2;i++){
   float lo=filt(p,3*i,x[i]),hi=filt(p,3*i+1,x[i]*c),hq=filt(p,3*i+2,-x[i]*s);
   if(p->sub[0]+1==p->stride[0])p->low[p->pos[0]][i]=enc(lo);
   if(p->sub[1]+1==p->stride[1]){p->high[p->pos[1]][2*i]=enc(hi);p->high[p->pos[1]][2*i+1]=enc(hq);}
  }
  for(unsigned k=0;k<2;k++)if(++p->sub[k]==p->stride[k]){
   unsigned cap=k?CH:CL;p->sub[k]=0;if(++p->pos[k]==cap)p->pos[k]=0;if(p->fill[k]<cap)p->fill[k]++;
  }
  if(p->dry==1.f){*ol++=l[j];*orr++=r[j];}
  else {*ol++=fmaf(1-p->dry,z[0],p->dry*l[j]);*orr++=fmaf(1-p->dry,z[1],p->dry*r[j]);}
  p->c=c*p->rotc-s*p->rots;p->s=s*p->rotc+c*p->rots;
  if(!(++p->frames&1023u)){float g=1/sqrtf(p->c*p->c+p->s*p->s);p->c*=g;p->s*=g;}
 }
}
