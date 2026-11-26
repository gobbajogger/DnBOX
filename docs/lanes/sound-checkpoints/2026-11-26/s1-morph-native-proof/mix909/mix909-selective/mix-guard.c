#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#define fm_fabsf fabsf
#include "mix909.inc"
static uint32_t s=0x951e430u;
static uint32_t r32(void){s^=s<<13;s^=s>>17;s^=s<<5;return s;}
static float f(uint32_t v){union {uint32_t u;float f;}a={.u=v};return a.f;}
static float val(void){
 uint32_t r=r32();
 if((r&31u)==0) return f(r&0x80000000u);
 if((r&31u)==1) return f(r&0x80000001u);
 return (float)(int32_t)r*0x1p-26f;
}
static float ref(float a[4][256],unsigned n,const float g[6],float level,float pk)
{
 float m[256];
 for(unsigned i=0;i<n;i++)m[i]=(a[0][i]+a[1][i])*.5f;
 for(unsigned j=0;j<4;j++){
     float c=g[j<2?j:j+2];
     if(c!=1.f)for(unsigned i=0;i<n;i++)a[j][i]*=c;
 }
 for(unsigned j=2;j<4;j++)if(g[j]!=0.f)
     for(unsigned i=0;i<n;i++)a[j][i]+=m[i]*g[j];
 for(unsigned i=0;i<n;i++){float x=fabsf(m[i])*level;if(x>pk)pk=x;}
 return pk;
}
int main(void){
 for(unsigned j=0;j<20000;j++){
    const unsigned n=j%257u;
    float a[4][256],b[4][256],g[6];
    for(unsigned i=0;i<4;i++)for(unsigned k=0;k<256;k++)a[i][k]=val();
    for(unsigned i=0;i<6;i++)switch(r32()&3u){case 0:g[i]=0.f;break;case 1:g[i]=-0.f;break;case 2:g[i]=1.f;break;default:g[i]=val()*.01f;break;}
    memcpy(b,a,sizeof a);
    float p0=fabsf(val()),lev=fabsf(val());
    float x=ref(a,n,g,lev,p0),y;
    if(g[0]==1.f||g[1]==1.f||g[4]==1.f||g[5]==1.f)y=ref(b,n,g,lev,p0);
    else y=fm1_909_mix_settled(b[0],b[1],b[2],b[3],n,g,lev,p0);
    if(memcmp(a,b,sizeof a)||memcmp(&x,&y,sizeof x))return 1;
 }
 puts("PASS 20,000 settled glide vectors: four full PCM buses and meter bit-identical");
 return 0;
}
