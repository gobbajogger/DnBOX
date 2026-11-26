#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "drum909.h"
static unsigned bits(float src){unsigned w;memcpy(&w,&src,4);return w;}
static void advance(drum909_t *d,float v[4][256],int from,int n){
 drum909_render_st_cleared(d,v[0]+from,v[1]+from,v[2]+from,v[3]+from,n,0x7ffu);
}
int main(void){
 unsigned reported=0;
 for(int voice=-1;voice<DR_NUM;voice++){
  drum909_t a={0},b={0};
  drum909_init(&a); drum909_init(&b);
  if(voice>=0){drum909_trigger(&a,voice,.75f);drum909_trigger(&b,voice,.75f);}
  float x[4][256]={{0}},y[4][256]={{0}};
  advance(&a,x,0,128); advance(&a,x,128,128);
  advance(&b,y,0,256);
  unsigned first=256, diff=0,prefix=0, leftbefore=0,leftafter=0;
  for(unsigned i=0;i<256;i++) for(unsigned j=0;j<4;j++){
   if(bits(x[j][i])!=bits(y[j][i])){
    if(first==256){first=i;leftbefore=bits(x[j][i]);leftafter=bits(y[j][i]);}
    if(i<128)prefix++;
    diff++;
   }
  }
  printf("%d,%u,%u,%u,%08x,%08x,%d\n",voice,first,diff,prefix,leftbefore,leftafter,memcmp(&a,&b,sizeof a)==0);
  if(voice>=0 && diff>0 && first>=128)reported++;
 }
 return reported ? 0 : 1;
}
