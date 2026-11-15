// Source-derived original-OTA reference experiment, not an admitted product API.
#pragma once
#include <cmath>
namespace DunneCore {
struct OriginalOTAFilter {
 template<class T> static T minimum(T a,T b){return b<a?b:a;}
 template<class T> static T maximum(T a,T b){return a<b?b:a;}
 float up[8]={},down[8]={};
 float rate=44100.f,z[4]={},hp=0,hpTan=0,hpG=0;
 float cutoffLog=0,g=0,gTarget=0,gStep=0,k=0,mode=0,modeTarget=0,modeStep=0,outGain=0;
 int modeLeft=0,gLeft=0;bool first=false,seeded=false,seedPermission=true;
 void clear(){for(float &x:up)x=0;for(float &x:down)x=0;for(float &x:z)x=0;hp=0;cutoffLog=g=gTarget=gStep=k=mode=modeTarget=modeStep=outGain=0;modeLeft=gLeft=0;first=seeded=false;seedPermission=true;}
 void updateSampleRate(float sr){rate=sr;hpTan=std::tan(31.41592788696289f/(sr+sr));hpG=hpTan/(1.f+hpTan);}
 void init(float sr){updateSampleRate(sr);clear();}
 static float curve(float q){
  if(q<=.5f){q/=.9f;return(q*q)*std::fma(q,-6.920000076293945f,10.920000076293945f);}
  if(q<.75f){q=(q+(-.5f))*4.f;return std::fma(q,std::fma(q,std::fma(q,.7831621170043945f,-1.407510757446289f),1.5905351638793945f),2.1838135719299316f);}
  if(q<.9f){q=(q+(-.75f))/.15000000596046448f;return std::fma(q,std::fma(q,std::fma(q,-.8449998497962952f,1.0199997425079346f),.675000011920929f),3.1500000953674316f);}
  return std::fma(q+(-.9f),1.2000000476837158f,4.f);
 }
 void setParameters(double cutoff,double normalizedRes,unsigned frames){
  outGain=1.f;
  if(first){if(modeTarget!=1.f){modeLeft=maximum(1,int(std::ceil(rate*.005f)));modeTarget=1.f;modeStep=(modeTarget-mode)/float(modeLeft);}}
  else{mode=modeTarget=1.f;modeLeft=0;}
  const float maxCut=minimum(rate*.9f,30000.f);
  const float c=maximum(minimum(float(cutoff),maxCut),3.f);
  const float lc=std::log2(c);
  float q=curve(maximum(minimum(float(normalizedRes),1.f),0.f));
  const float t=minimum(maximum((c/rate+(-.45f))/.05f,0.f),1.f);
  const float w=maximum(q+(-3.9f),0.f)*t;
  const float s=std::fma(t,-2.f,3.f);
  q=std::fma(-(t*w),s,q); k=q;
  if(!(q>4.f))seeded=false;
  if(first){if(lc==cutoffLog)return;cutoffLog=lc;gTarget=std::tan((std::exp2(lc)*3.1415927410125732f)/(rate+rate));gLeft=maximum(1u,frames);gStep=(gTarget-g)/float(gLeft);}
  else{cutoffLog=lc;g=gTarget=std::tan((std::exp2(lc)*3.1415927410125732f)/(rate+rate));first=true;}
 }
 float tick(float input,float G,float G4,float inv){
  float d=(input-hp)*hpG;const float low=hp+d;hp=d+low;
  const float a=1.f-G;
  float acc=0,power=1.f;
  for(int i=3;i>=0;--i){const float t=a*power;acc=std::fma(t,z[i],acc);power=G*power;}
  const float x=(input-low)*.025f;
  const float c=std::fma(k,.3f,1.f);
  float t=G4*c;
  t=std::fma(t,x,acc);
  t=-(inv*t);
  t=k*t;
  t=std::fma(c,x,t);
  float provisional[4];
  for(int i=0;i<4;++i){const float old=a*z[i];t=std::fma(G,t,old);provisional[i]=t;}
  float v=std::fma(x,.3f,-provisional[3]);
  v=std::tanh(v*8.f);
  const float factor=k*.125f;
  float in=std::fma(factor,v,x),coeff[4],offset[4];
  float product=1.f,affine=0.f;
  for(int i=0;i<4;++i){
   const float prev=provisional[i];const float tan=std::tanh(in-prev);
   float n=std::fma(-tan,tan,1.f);n=g*n;
   const float recip=1.f/(n+1.f);coeff[i]=n*recip;
   const float delta=prev-z[i];
   const float b=std::fma(-g,tan,delta);
   offset[i]=-(b*recip);
   product=product*coeff[i];affine=std::fma(coeff[i],affine,offset[i]);
   in=prev;
  }
  float feedback=std::fma(-v,v,1.f);feedback=-(k*feedback);
  float correction=(feedback*affine)/std::fma(-feedback,product,1.f);
  for(int i=0;i<4;++i){correction=std::fma(coeff[i],correction,offset[i]);provisional[i]+=correction;z[i]=std::fma(provisional[i],2.f,-z[i]);}
  const float wet=provisional[3]*outGain;
  if(mode==1.f)return wet;
  const float dry=provisional[1];if(mode==0.f)return dry;
  return std::fma(mode,wet-dry,dry);
 }
 static void half(float *s,float input,float &a,float &b) {
  a=std::fma(input-s[2],.079866424202919f,s[0]);
  b=std::fma(a-s[3],.5453536510467529f,s[1]);
  float c=std::fma(input-s[6],.28382933139801025f,s[4]);
  float d=std::fma(c-s[7],.8344119191169739f,s[5]);
  s[0]=input;s[1]=a;s[2]=a;s[3]=b;s[4]=input;s[5]=c;s[6]=c;s[7]=d;
  a=b;b=d;
 }
 float process(float input){
  if(modeLeft>0){mode+=modeStep;if(--modeLeft==0)mode=modeTarget;}
  if(gLeft>0){g+=gStep;if(--gLeft==0)g=gTarget;}
  if(k>4.f&&!seeded){
   if(seedPermission){const float a=std::sqrt(k+(-4.f))*.125f;const float b=a+a;const float t=std::tanh(b)*g;z[0]=std::fma(a,2.f,t);z[1]=b;const float tt=std::tanh(a);z[2]=std::fma(-g,tt,a);z[3]=-(g*tt);}
   else z[0]+=.00009999999747378752f;
   seeded=true;
  }
  float a,b;half(up,input,a,b);
  const float G=g/(g+1.f);float G4=G*G;G4=G*G4;G4=G*G4;
  const float inv=1.f/std::fma(k,G4,1.f);seedPermission=false;
  const float x=tick(a,G,G4,inv);const float y=tick(b,G,G4,inv);
  // Original downsampler first branch consumes the second oversampled tick.
  float aa=std::fma(y-down[2],.079866424202919f,down[0]);
  float ab=std::fma(aa-down[3],.5453536510467529f,down[1]);
  float ba=std::fma(x-down[6],.28382933139801025f,down[4]);
  float bb=std::fma(ba-down[7],.8344119191169739f,down[5]);
  down[0]=y;down[1]=aa;down[2]=aa;down[3]=ab;down[4]=x;down[5]=ba;down[6]=ba;down[7]=bb;
  return ((ab+bb)*.5f)*40.f;
 }
};
}
