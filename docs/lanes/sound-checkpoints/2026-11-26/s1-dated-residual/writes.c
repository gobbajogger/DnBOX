#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "soundpipe.h"
int main(int argc,char**argv){
 assert(argc==3);FILE *in=fopen(argv[1],"rb"),*out=fopen(argv[2],"wb");assert(in&&out);
 sp_data *sp=0;sp_create(&sp);assert(sp);sp->sr=44100;sp->nchan=2;
 sp_vdelay *p[4]={0};for(unsigned j=0;j<4;j++){sp_vdelay_create(&p[j]);sp_vdelay_init(sp,p[j],10.f);}
 float x[8];unsigned frames=0,wrong=0;
 while(fread(x,4,8,in)==8){
  float y[4]={0},w[4];const float in[]={x[2],x[3],x[3],x[5]};
  for(unsigned j=0;j<4;j++){
   p[j]->del=x[0]*(j<2?2.f:1.f);p[j]->feedback=x[1];
   unsigned i=p[j]->left;
   sp_vdelay_compute(sp,p[j],(float*)in+j,y+j);
   w[j]=((float*)p[j]->buf.ptr)[i];
  }
  float rs=y[2]+y[3];
  unsigned er=(memcmp(y,x+4,3*4)!=0)||(memcmp(&rs,x+7,4)!=0);
  if(er && wrong<10)fprintf(stderr,"mismatch frame %u t=%g fx %a/%a %a/%a %a/%a %a/%a\n",frames,x[0],y[0],x[4],y[1],x[5],y[2],x[6],rs,x[7]);
  wrong+=er;frames++;
  float rec[]={w[0],w[1],w[2],w[3],y[0],y[1],y[2],y[3]};
  assert(fwrite(rec,4,8,out)==8);
 }
 assert(fclose(out)==0);fclose(in);printf("frames=%u different_frames=%u\n",frames,wrong);return !!wrong;
}
