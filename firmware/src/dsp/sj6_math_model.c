/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_math_model.h"
#include <string.h>
/* Existing generic mathematical sine constants, unchanged. No private data. */
#include "sj6_sine_table.inc"

static uint16_t u16(const uint8_t *p){return(uint16_t)((unsigned)p[0]|((unsigned)p[1]<<8));}
static uint32_t u32(const uint8_t *p){return(uint32_t)p[0]|((uint32_t)p[1]<<8)|((uint32_t)p[2]<<16)|((uint32_t)p[3]<<24);}
static uint64_t u64(const uint8_t *p){return(uint64_t)u32(p)|((uint64_t)u32(p+4)<<32);}
static float f32(const uint8_t *p){uint32_t u=u32(p);float f;__builtin_memcpy(&f,&u,4);return f;}
static int finite32(float x){uint32_t u;__builtin_memcpy(&u,&x,4);return((u>>23)&255u)!=255u;}
static float absf(float x){return x<0?-x:x;}
static uint32_t crc(const uint8_t *p,uint32_t n){uint32_t c=~0u,i;unsigned k;for(i=0;i<n;i++){if(i>=76&&i<80)continue;c^=p[i];for(k=0;k<8;k++)c=(c>>1)^(0xedb88320u&(0u-(c&1u)));}return~c;}
static int valid_coefficient(float x){return finite32(x)&&absf(x)<=100000000.f;}
static int valid_tau(float x){return finite32(x)&&x>0.f&&x<=1.f;}
int sj6_math_open(Sj6MathModel *out,const void *bytes,uint32_t n){
 Sj6MathModel m;const uint8_t *p=(const uint8_t*)bytes;unsigned i,j;uint32_t end=0;
 if(!out||!p||n<80||n>1048576u)return 1;
 if(memcmp(p,"SJ6M",4)||u16(p+4)!=1||u16(p+6)!=80||u32(p+8)!=n||u32(p+12)!=44100||p[31])return 1;
 if(crc(p,n)!=u32(p+76))return 1;
 memset(&m,0,sizeof m);m.bytes=p;m.size=n;m.frames=u32(p+16);m.loop_start=u32(p+20);m.loop_end=u32(p+24);
 m.root=p[28];m.low=p[29];m.high=p[30];m.phase_inc=u64(p+32);m.phase_zero=u64(p+40);m.dc=f32(p+48);
 m.fade=u16(p+52);m.cycle_n=u16(p+54);m.harmonics=u16(p+56);m.segments=u16(p+58);
 m.off_harm=u32(p+64);m.off_drift=u32(p+68);m.off_transient=u32(p+72);
 if(!m.frames||m.frames>0x3fffffffu||m.loop_start>=m.loop_end||m.loop_end>m.frames||m.root>127||m.low>m.high||m.high>127||!valid_coefficient(m.dc))return 1;
 if(!m.cycle_n||m.cycle_n<16||m.cycle_n>8192||(m.cycle_n&(m.cycle_n-1))||!m.harmonics||m.harmonics>1024||!m.segments||m.segments>64||m.fade>63||u16(p+60)!=8||u16(p+62))return 1;
 if(m.off_harm!=80||m.off_drift!=80u+6u*m.harmonics||m.off_transient!=m.off_drift+24u*m.segments||n!=m.off_transient+420u)return 1;
 for(i=0;i<m.harmonics;i++){const uint8_t *h=p+m.off_harm+6u*i;unsigned k=u16(h);if(!k||k>=m.cycle_n/2||(i&&k<=u16(h-6)))return 1;}
 for(i=0;i<m.segments;i++){const uint8_t *q=p+m.off_drift+24u*i;uint32_t a=u32(q),b=u32(q+4);float bound=0;if(a>=b||b>=m.frames||(i&&a!=end))return 1;end=b;for(j=0;j<4;j++){float x=f32(q+8+4*j);if(!finite32(x))return 1;bound+=absf(x);}if(bound>=.25f)return 1;}
 p+=m.off_transient;
 for(i=0;i<8;i++)if(!valid_coefficient(f32(p+4*i))||!valid_tau(f32(p+32+4*i)))return 1;
 for(i=0;i<6;i++)if(!valid_tau(f32(p+64+4*i)))return 1;
 for(i=0;i<3;i++)if(!valid_coefficient(f32(p+88+4*i)))return 1;
 for(i=0;i<8;i++){const uint8_t *r=p+100+40*i;unsigned h=u16(r),found=0;if(u16(r+2))return 1;for(j=0;j<m.harmonics;j++)found|=h==u16(m.bytes+m.off_harm+6*j);if(!found)return 1;for(j=0;j<6;j++)if(!valid_coefficient(f32(r+16+4*j)))return 1;}
 *out=m;return 0;
}
static float sine(uint32_t p){unsigned i=p>>21;float x=(float)(p&2097151u)*(1.f/2097152.f);return g_sine[i]+x*(g_sine[(i+1)&2047u]-g_sine[i]);}
static int32_t nearest(float x){int32_t n=(int32_t)x;float r=x-(float)n;if(r>.5f||(r==.5f&&(n&1)))n++;else if(r<-.5f||(r==-.5f&&(n&1)))n--;return n;}
int sj6_math_build(const Sj6MathModel *m,int16_t *cycle,uint32_t capacity){
 unsigned i,j;uint32_t step;if(!m||!cycle||capacity<m->cycle_n)return 1;step=0x80000000u/(m->cycle_n/2u);
 for(i=0;i<m->cycle_n;i++){float y=m->dc;for(j=0;j<m->harmonics;j++){const uint8_t *h=m->bytes+m->off_harm+6*j;uint32_t phase=(uint32_t)(i*step)*u16(h)+((uint32_t)u16(h+4)<<16);y+=(float)u16(h+2)*sine(phase+0x40000000u);}if(!finite32(y)||y<-32768.5f||y>=32767.5f)return 1;{int32_t n=nearest(y);if(n<-32768||n>32767)return 1;cycle[i]=(int16_t)n;}}
 return 0;
}
void sj6_math_reset(Sj6MathState *s){if(s)s->segment=0;}
static uint64_t mul_u64_u32(uint64_t a,uint32_t b){return(uint64_t)(uint32_t)a*b+((uint64_t)((uint32_t)(a>>32)*b)<<32);}
static uint64_t fraction_phase(uint64_t a,uint32_t b){return(uint64_t)(uint32_t)(a>>32)*b+(((uint64_t)(uint32_t)a*b)>>32);}
/* Approximate exp(-x) for recorded-zone transient math only. No libm, binary64
 * or omitted source graph. Degree-eight exp2 Taylor on [0,1), explicit scale. */
static float decay(float x){float q,f,p,scale;int n;uint32_t u;if(x>=87.f)return 0.f;if(x<=0.f)return 1.f;q=-x*1.4426950408889634f;n=(int)q;if((float)n>q)n--;f=q-(float)n;
 p=0.0000013215486790144307f;p=0.000015252733804059838f+f*p;p=0.0001540353039338161f+f*p;p=0.0013333558146428443f+f*p;p=0.009618129107628477f+f*p;p=0.05550410866482158f+f*p;p=0.2402265069591007f+f*p;p=0.6931471805599453f+f*p;p=1.f+f*p;
 u=(uint32_t)(n+127)<<23;__builtin_memcpy(&scale,&u,4);return p*scale;
}
float sj6_math_at(const Sj6MathModel *m,Sj6MathState *s,const int16_t *cycle,uint32_t frame,uint32_t fraction){
 const uint8_t *d,*t;unsigned seg=0,i,j,shift=0;uint32_t phase,index,mask;uint64_t linear;float u,drift,y,at[3]={0,0,0},on[3]={0,0,0},time;
 if(!m||!cycle||frame>=m->frames)return 0.f;
 if(s&&s->segment<m->segments)seg=s->segment;
 d=m->bytes+m->off_drift+24u*seg;if(frame<u32(d))seg=0;
 while(seg+1u<m->segments&&frame>=u32(m->bytes+m->off_drift+24u*seg+4))seg++;
 if(s)s->segment=(uint16_t)seg;
 d=m->bytes+m->off_drift+24u*seg;
 if(frame<u32(d))u=0.f;else if(frame>=u32(d+4))u=1.f;else u=((float)(frame-u32(d))+(float)fraction*(1.f/4294967296.f))/(float)(u32(d+4)-u32(d));
 drift=((f32(d+8)*u+f32(d+12))*u+f32(d+16))*u+f32(d+20);
 linear=m->phase_zero+mul_u64_u32(m->phase_inc,frame)+fraction_phase(m->phase_inc,fraction);
 phase=(uint32_t)(linear>>32)+(uint32_t)(int32_t)(drift*4294967296.f);
 for(i=m->cycle_n;i>1;i>>=1)shift++;
 shift=32-shift;index=phase>>shift;mask=(1u<<shift)-1u;
 u=(float)(phase&mask)/(float)(1u<<shift);y=(float)cycle[index]+u*((float)cycle[(index+1u)&(m->cycle_n-1u)]-(float)cycle[index]);
 time=(float)frame+(float)fraction*(1.f/4294967296.f);
 if(m->fade&&frame<m->fade)y*=time/(float)m->fade;
 t=m->bytes+m->off_transient;
 if(frame>=m->fade){if(frame<132300u)for(i=0;i<8;i++)y+=f32(t+4*i)*decay(time*f32(t+32+4*i));if(frame<22050u)for(i=0;i<3;i++)at[i]=decay(time*f32(t+64+4*i));}
 if(frame<441u)for(i=0;i<3;i++){on[i]=decay(time*f32(t+76+4*i));y+=f32(t+88+4*i)*on[i];}
 if(frame<22050u){for(i=0;i<8;i++){const uint8_t *r=t+100+40*i;uint32_t p=phase*u16(r);float cs=sine(p+0x40000000u),sn=sine(p);for(j=0;j<3;j++){if(at[j]!=0.f)y+=((float)(int16_t)u16(r+4+4*j)*cs+(float)(int16_t)u16(r+6+4*j)*sn)*at[j];if(on[j]!=0.f)y+=(f32(r+16+8*j)*cs+f32(r+20+8*j)*sn)*on[j];}}}
 return y;
}
