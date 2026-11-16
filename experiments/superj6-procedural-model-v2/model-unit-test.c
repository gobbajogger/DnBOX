/* Synthetic, payload-free protocol/overflow/position regressions. */
#include "sj6_math_model.h"
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
static unsigned failed,checks;
static void expect(const char*n,int ok){checks++;if(!ok){failed++;printf("FAIL %s\n",n);}}
static void w16(unsigned char*p,uint16_t x){p[0]=(unsigned char)x;p[1]=(unsigned char)(x>>8);}
static void w32(unsigned char*p,uint32_t x){unsigned i;for(i=0;i<4;i++)p[i]=(unsigned char)(x>>(8*i));}
static void wf(unsigned char*p,float x){uint32_t u;memcpy(&u,&x,4);w32(p,u);}
static uint32_t crc(const unsigned char*p,unsigned n){uint32_t c=~0u;unsigned i,j;for(i=0;i<n;i++){if(i>=76&&i<80)continue;c^=p[i];for(j=0;j<8;j++)c=(c>>1)^(0xedb88320u& (0u-(c&1u)));}return~c;}
static void seal(unsigned char*p,unsigned n){w32(p+76,crc(p,n));}
static void synthetic(unsigned char*p,unsigned n){unsigned i;memset(p,0,n);memcpy(p,"SJ6M",4);w16(p+4,1);w16(p+6,80);w32(p+8,n);w32(p+12,44100);w32(p+16,10);w32(p+20,1);w32(p+24,9);p[28]=60;p[29]=0;p[30]=127;w32(p+36,0x10000000u);w16(p+54,512);w16(p+56,1);w16(p+58,1);w16(p+60,8);w32(p+64,80);w32(p+68,86);w32(p+72,110);w16(p+80,1);w16(p+82,100);w32(p+86,1);w32(p+90,9);for(i=0;i<14;i++)wf(p+110+32+4*i,.001f);for(i=0;i<8;i++)w16(p+110+100+40*i,1);seal(p,n);}
static void reject_case(const unsigned char*p,unsigned n,unsigned at,uint32_t word){unsigned char q[530];Sj6MathModel m,old;memcpy(q,p,n);w32(q+at,word);seal(q,n);memset(&m,0xa5,sizeof m);old=m;expect("semantic malformed payload rejected",sj6_math_open(&m,q,n)!=0);expect("failed open preserves view",memcmp(&m,&old,sizeof m)==0);}
int main(void){unsigned char p[530],q[530];int16_t table[512],guard[512];Sj6MathModel m;Sj6MathState state;unsigned i,j;synthetic(p,sizeof p);expect("synthetic valid",sj6_math_open(&m,p,sizeof p)==0);expect("build capacity check",sj6_math_build(&m,table,511)!=0);memset(table,0xa5,sizeof table);memcpy(guard,table,sizeof table);expect("capacity does not write",sj6_math_build(&m,table,511)!=0&&memcmp(table,guard,sizeof table)==0);expect("synthetic build",sj6_math_build(&m,table,512)==0);sj6_math_reset(&state);
for(i=0;i<10;i++)for(j=0;j<5;j++){uint32_t frac=j==4?0xffffffffu:j*0x40000000u;float f=sj6_math_at(&m,&state,table,i,frac);double pos=i+(double)frac/4294967296.0;double wanted=100*cos(6.2831853071795864769*pos/16);expect("fractional Q64 phase",fabs((double)f-wanted)<.6);expect("stateless equality",f==sj6_math_at(&m,0,table,i,frac));}
for(i=0;i<530;i++){memcpy(q,p,sizeof q);q[i]^=1;expect("every byte CRC protected",sj6_math_open(&m,q,sizeof q)!=0);}
for(i=0;i<530;i++)expect("all truncated spans rejected",sj6_math_open(&m,p,i)!=0);
reject_case(p,sizeof p,16,0);reject_case(p,sizeof p,64,0xffffffffu);reject_case(p,sizeof p,68,0xfffffff0u);reject_case(p,sizeof p,72,0xffffffffu);reject_case(p,sizeof p,48,0x7fc00000u);reject_case(p,sizeof p,54,0);reject_case(p,sizeof p,54,513);reject_case(p,sizeof p,80,0);reject_case(p,sizeof p,80,256);reject_case(p,sizeof p,86,10);reject_case(p,sizeof p,94,0x7fc00000u);reject_case(p,sizeof p,94,0x3f800000u);reject_case(p,sizeof p,110+32,0);reject_case(p,sizeof p,110,0x7f7fffffu);
expect("view restore",sj6_math_open(&m,p,sizeof p)==0);expect("out of domain zero",sj6_math_at(&m,&state,table,m.frames,0)==0);for(i=0;i<10;i++){uint32_t pos=(i*7)%10;expect("nonmonotone source position",sj6_math_at(&m,&state,table,pos,0)==sj6_math_at(&m,0,table,pos,0));}
printf("%u synthetic protocol/overflow/fractional-position checks, %u failed; view %u state %u\n",checks,failed,(unsigned)sizeof m,(unsigned)sizeof state);return!!failed;}
