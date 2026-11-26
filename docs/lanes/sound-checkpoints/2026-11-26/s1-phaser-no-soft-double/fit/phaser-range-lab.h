#include <stdint.h>
static inline uint32_t fc_bits(float v){uint32_t u;__builtin_memcpy(&u,&v,4);return u;}
static inline float fc_float(uint32_t u){float v;__builtin_memcpy(&v,&u,4);return v;}
static inline float fc_lowcos(float x){
 const float z=x*x;
 const float q=-0.5f+z*(0x1.555556p-5f+z*(-0x1.6c16c2p-10f+z*(0x1.a01a02p-16f+z*-0x1.27e4fcp-22f)));
 return 1.f+z*q;
}
static inline float fc_lowsin(float x){
 const float z=x*x;
 return x+x*z*(-0x1.555556p-3f+z*(0x1.111112p-7f+z*(-0x1.a01a02p-13f+z*(0x1.71de3ap-19f+z*-0x1.ae6456p-26f))));
}
static uint32_t fc_window(const uint32_t p[8],unsigned k){
 unsigned i=k>>5,b=k&31u;
 return b?(p[i]>>b)|(p[i+1]<<(32u-b)):p[i];
}
static __attribute__((noinline)) float fc_broad(float x,unsigned sine){
 const uint32_t w=fc_bits(x)&0x7fffffffu;
 if(w>=0x7f800000u)return fc_float(0x7fc00000u);
 if(w<=0x3f490fdau)return sine?fc_lowsin(x):fc_lowcos(x);
 /* 192 leading fraction bits of 2/pi, in little limb order. For binary32:
  * x = m * 2^(E-150); m*table / 2^(342-E) approximates |x|/(pi/2).
  * Largest normal exponent E=254 leaves >=88 bits below integer boundary.
  */
 static const uint32_t inv[6]={0x3c439041u,0xdb629599u,0xf534ddc0u,0xfc2757d1u,0x4e441529u,0xa2f9836eu};
 const uint32_t m=0x00800000u|(w&0x007fffffu);
 uint32_t p[8];uint64_t carry=0;
 for(unsigned j=0;j<6;j++){carry=(uint64_t)m*inv[j]+carry;p[j]=(uint32_t)carry;carry>>=32;}
 p[6]=(unsigned)carry;p[7]=0;
 const unsigned shift=342u-(w>>23);
 uint32_t hi=fc_window(p,shift-32u),lo=fc_window(p,shift-64u);
 const unsigned up=hi>>31,quad=(fc_window(p,shift)+up)&3u;
 if(up){lo=0u-lo;hi=~hi+(lo==0u);}
 const float f=(float)hi*0x1p-32f+(float)lo*0x1p-64f;
 const float r=(f*0x1.921fb6p0f)*(up?-1.f:1.f);
 if(sine){
  const float y=quad&1u?fc_lowcos(r):fc_lowsin(r);
  return ((((fc_bits(x)>>31)^(quad>>1))!=0)?-y:y);
 }
 if(!(quad&1u))return quad? -fc_lowcos(r):fc_lowcos(r);
 return quad==1u ? -fc_lowsin(r):fc_lowsin(r);
}
static inline float fc_full(float x){return fc_broad(x,0);}
static __attribute__((noinline)) float fc_product(float v,float c){
 const uint32_t vw=fc_bits(v),cw=fc_bits(c), a=vw&0x7fffffffu,b=cw&0x7fffffffu,sign=(vw^cw)&0x80000000u;
 if(a>=0x7f800000u||b>=0x7f800000u)return v*c;
 if(!a||!b)return fc_float(sign);
 unsigned ve=a>>23,ce=b>>23;
 if(ve && ce && ve+ce>=128u) return v*c;
 const unsigned vm=(a&0x7fffffu)|(ve?0x800000u:0),cm=(b&0x7fffffu)|(ce?0x800000u:0);
 const unsigned n=151u-(ve?ve:1u)-(ce?ce:1u);
 if(n>48u)return fc_float(sign);
 const uint64_t z=(uint64_t)vm*cm,half=UINT64_C(1)<<(n-1u);
 uint32_t q=(uint32_t)(z>>n);
 const uint64_t rest=z&((half<<1u)-1u);
 q+=(rest>half||(rest==half&&(q&1u)));
 return fc_float(sign|q);
}
