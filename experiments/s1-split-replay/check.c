#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "app/s1c_part.h"
extern void split_s1c_render(s1c *,float *,float *,unsigned);
static s1_dry_part owner,old,expected;
_Alignas(16) static uint8_t ram[S1C_ARENA_BYTES],old_ram[S1C_ARENA_BYTES],expected_ram[S1C_ARENA_BYTES];
static float a[2][256],b[2][256];
#define S(p,v) s1c_set(&owner.e,p,v)
int main(void){
  unsigned frames=0;
  for(unsigned k=0;k<32;k++){
    const unsigned keys[]={48,51,55,60,64,66};
    const unsigned sizes[]={1,3,4,7,8,9,15,16,17,31,32,64,127,128,129,255,256};
    memset(ram,0,sizeof ram);
    s1_part_init(&owner);s1_part_bind(&owner,ram);
    S(20,k&1);S(115,k%3);S(12,k&2?.43f:0.f);S(10,k&4?.31f:0.f);
    S(37,k&8?1:0);S(38,.71f);S(40,.36f);S(41,k&16?1:0);S(42,.52f);S(43,.0137f);S(44,.41f);
    S(35,.48f);S(36,1.9f);S(121,.63f);S(116,.59f);S(117,.81f);S(118,-.2f);S(119,.77f);
    S(58,k%4);S(59,k%4);S(51,(k+1)%4);S(14,.32f);S(15,2.1f);S(45,1);S(46,.39f);S(47,.67f);
    S(26,.004f);S(27,.12f);S(28,.37f);S(29,.018f);
    for(unsigned i=0;i<170;i++){
      unsigned n=sizes[(i+k*11u)%(sizeof sizes/sizeof sizes[0])];
      if(i==3||i==104) for(unsigned j=0;j<6;j++)s1_part_note(&owner,keys[j],70+5*j);
      if(i%11==0){S(3,(int)i%97-48);S(4,48-(int)(i*7)%97);S(16,220.f+77.f*i);S(34,2048.f+224.f*i);}
      if(i%19==4)s1_part_note(&owner,keys[i%6],0);
      if(i==37||i==117)S(20,i==37?!(k&1):(k&1));
      if(i==81)for(unsigned j=0;j<6;j++)s1_part_note(&owner,keys[j],0);
      if(i==101){S(43,.0299f);S(40,.53f);S(42,.71f);S(35,.19f);S(116,.21f);}
      if(i==103)s1_part_all_off(&owner);
      memcpy(&old,&owner,sizeof old);memcpy(old_ram,ram,sizeof ram);
      s1c_render(&owner.e,a[0],a[1],n);
      memcpy(&expected,&owner,sizeof expected);memcpy(expected_ram,ram,sizeof ram);
      memcpy(&owner,&old,sizeof owner);memcpy(ram,old_ram,sizeof ram);
      split_s1c_render(&owner.e,b[0],b[1],n);
      if(memcmp(a[0],b[0],n*sizeof(float))||memcmp(a[1],b[1],n*sizeof(float))||
          memcmp(&owner,&expected,sizeof owner)||memcmp(ram,expected_ram,sizeof ram)){
        fprintf(stderr,"Mismatch test %u span %u n %u: PCM %d %d, owner %d, arena %d\n",k,i,n,
         memcmp(a[0],b[0],n*4),memcmp(a[1],b[1],n*4),memcmp(&owner,&expected,sizeof owner),memcmp(ram,expected_ram,sizeof ram));
        return 1;
      }
      frames+=n;
    }
  }
  printf("Exact stereo, complete owner, and FX arena: %u frames, 5440 spans\n",frames);
}
