#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "soundpipe.h"
enum{FS=44100, TAIL=44100, FRAMES=264728};
int main(int argc,char**argv){
 assert(argc==2);
 FILE *in=fopen(argv[1],"rb");assert(in);
 sp_data *sp=0;sp_create(&sp);sp->sr=FS;sp->nchan=2;
 sp_vdelay *p[5]={0};for(int i=0;i<5;i++){sp_vdelay_create(&p[i]);sp_vdelay_init(sp,p[i],10.f);}
 float x[8];double err=0,et=0,ref=0,reft=0,abs=0;
 unsigned n=0,diff=0,mono=0,monozero=0,state=0,wstate=0,streak=0,mstreak=0;
 while(fread(x,4,8,in)==8){
   float y[5],a[5]={x[2],x[3],x[3],0.f,0.f};
   for(int i=0;i<5;i++){
    if(i==3)a[i]=y[1];if(i==4)a[i]=x[3]+y[1];
    p[i]->del=x[0]*(i<2?2.f:1.f);p[i]->feedback=x[1];
    sp_vdelay_compute(sp,p[i],&a[i],&y[i]);
   }
   float sum=y[2]+y[3],q=y[4]; assert(!memcmp(y,x+4,3*sizeof(float))&&!memcmp(&sum,x+7,sizeof(float)));
   if(memcmp(&sum,&q,4))diff++;
   bool eq=!memcmp(x+2,x+3,4);mono+=eq;monozero+=(eq&&x[2]==0.f);
   if(eq){++streak;if(streak>mstreak)mstreak=streak;}else streak=0;
   state+=!memcmp(y,y+1,4);
   unsigned iw=(p[0]->left+(10*FS-1))%(10*FS);
   wstate+=!memcmp((float*)p[0]->buf.ptr+iw,(float*)p[1]->buf.ptr+iw,4);
   double d=(double)q-sum,e=d*d,ee=(double)sum*sum;
   if(fabs(d)>abs)abs=fabs(d);
   err+=e;ref+=ee;
   if(n>=FRAMES-TAIL){et+=e;reft+=ee;}
   ++n;
 }
 assert(n==FRAMES);
 printf("frames=%u stable_inputs=%u same_zero_inputs=%u longest_equal=%u same_long_write=%u same_long_read=%u folded_changed_words=%u folded_whole_dbr=%.8f folded_tail_dbr=%.8f max_signed_error=%g\n",
 n,mono,monozero,mstreak,wstate,state,diff,10*log10(err/ref),10*log10(et/reft),abs);
}
