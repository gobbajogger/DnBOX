// Source-derived private reference implementation; not a production admission API.
#pragma once
#include <cmath>
namespace DunneCore {
struct OriginalSyrupFilter {
 template<class T> static T minimum(T a,T b){return b<a?b:a;}
 template<class T> static T maximum(T a,T b){return a<b?b:a;}
 double rate=44100.,lastCutoff=12.,lastRes=.1;
 float g=0,k=0,inGain=1,outGain=1,a=-1,b=1,invA=1,invB=1;
 float p1=0,h1=0,p2=0,h2=0,p3=0,h3=0,p4=0,h4=0;
 bool fourPole=false;
 void clear(){p1=h1=p2=h2=p3=h3=p4=h4=0;}
 void init(double sr){rate=sr;clear();}
 void updateSampleRate(double sr){rate=sr;}
 void setParameters(double cut,double res) {
  if(!std::isfinite(cut))cut=lastCutoff;
  if(!std::isfinite(res))res=lastRes;
  cut=minimum(maximum(cut,12.),rate*.45351473923);
  res=minimum(maximum(res,.1),10.);
  lastCutoff=cut;lastRes=res;
  const double z=(cut/rate)*3.141592653589793;
  g=float(std::tan(z/(1.+z)));
  double q=(-20.*std::log10(res))/19.; q=minimum(maximum(q,0.),1.);
  const double kr=std::pow(q,.45)*2.05;
  const double drive=std::fma(q,2.,1.);
  inGain=float(1./drive);
  outGain=float((std::pow(drive,.75)*1.35)/std::sqrt(drive*double(inGain)));
  const float twiceG=g+g;
  const float limit=maximum(1.f,std::fma(twiceG,.09f,.97f));
  k=float(kr/double(limit));
  const float gg=g*g;
  a=(twiceG-gg)+(-1.f); b=1.f-g;
  const float c=(gg-g)+1.f;
  invA=std::fabs(c)>1e-6f?1.f/c:(c<0?-1e6f:1e6f);
  const float d=std::fma(k,gg-g,1.f);
  invB=std::fabs(d)>1e-6f?1.f/d:(d<0?-1e6f:1e6f);
 }
 float process(float x) {
  if(fourPole) {
   float v=invA*(x-std::fma(a,h1,b*h2));
   float d=g*(v-h1); p1=h1+d; h1=d+p1;
   d=g*(p1-h2);p2=h2+d;h2=d+p2;x=p2;
  }
  const float feedback=std::fma(a,h3,b*h4);
  const float v=invB*std::fma(inGain,x,-(feedback*k));
  const float av=std::fabs(v), vv=v*v;
  const float n=v*std::fma(vv,std::fma(av,.8212266564369202f,.8932298421859741f),std::fma(av,2.455507516860962f,2.455507516860962f));
  const float d=std::fma(vv+2.445066452026367f,std::fabs(std::fma(v*.8146427273750305f,av,v)),2.445066452026367f);
  const float sat=n/d;
  float dx=g*(sat-h3);p3=h3+dx;h3=dx+p3;
  dx=g*(p3-h4);p4=h4+dx;h4=dx+p4;
  const float y=outGain*p4;
  if(!std::isfinite(y)){clear();return 0.f;} return y;
 }
};
}
