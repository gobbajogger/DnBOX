/* Diagnostic: same original authored room send and smoothed feedback at two rates. */
#include <stdio.h>
#include <stdlib.h>
#include "soundpipe.h"
int main(int n,char**v){
 if(n!=4&&n!=5)return 2;
 int d=atoi(v[3]);if(d!=1&&d!=7)return 2;
 sp_data sp={0};sp.sr=44100/d;sp_revsc *q=NULL;sp_revsc_create(&q);
 if(sp_revsc_init(&sp,q)!=SP_OK)return 4;
 q->lpfreq=.5f*sp.sr;
 FILE *in=fopen(v[1],"rb"),*out=fopen(v[2],"wb");if(!in||!out)return 2;
 float a[10],acc[2]={0},p[2]={0},y[2]={0},z[2][4]={{0}};
 FILE*trajectory=NULL;
 FILE *snap=fopen(v[4],"wb"); size_t frame=0; if(!snap||n!=5||d!=1)return 8;
 trajectory=NULL;
 int k=0;
 while(fread(a,sizeof(float),10,in)==10){
   acc[0]+=a[2];acc[1]+=a[3];
   if(++k>=d){
     k=0; q->feedback=a[8];
     p[0]=y[0];p[1]=y[1];float il=acc[0]/d,ir=acc[1]/d;
     if(trajectory){
       float delay[8];
       for(int j=0;j<8;j++){
         sp_revsc_dl* l=&q->delayLines[j];
         float x=(float)(l->writePos-l->readPos)-(float)l->readPosFrac*(1.f/268435456.f);
         if(x<0.f)x+=(float)l->bufferSize;
         delay[j]=x;
       }
       if(fwrite(delay,sizeof(float),8,trajectory)!=8)return 7;
     }
     sp_revsc_compute(&sp,q,&il,&ir,&y[0],&y[1]);
     acc[0]=acc[1]=0;
     if((++frame%4096)==0){
        for(unsigned j=0;j<8;j++){
          sp_revsc_dl *dl=&q->delayLines[j];
          if(fwrite(dl->buf,sizeof(float),dl->bufferSize,snap)!=dl->bufferSize)return 9;
        }
     }
   }
   float o[2]={y[0],y[1]};
   if(d>1)for(int c=0;c<2;++c){
      o[c]=p[c]+(y[c]-p[c])*((float)k/d);
      for(int j=0;j<4;++j){z[c][j]+=.38f*(o[c]-z[c][j]);o[c]=z[c][j];}
   }
   if(fwrite(o,sizeof(float),2,out)!=2)return 5;
 }
 fclose(snap); if(trajectory)fclose(trajectory);fclose(out);fclose(in);sp_revsc_destroy(&q);return 0;
}
