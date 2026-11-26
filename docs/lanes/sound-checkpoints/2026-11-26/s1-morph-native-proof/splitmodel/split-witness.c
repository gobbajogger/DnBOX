#include "drum909.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
int main(void){
    static drum909_t a,b;
    static float x[4][256],y[4][256];
    const unsigned lengths[]={64,97,128,159,192,220};
    unsigned divergent=0;
    for(unsigned q=0;q<sizeof lengths/sizeof *lengths;q++){
      memset(&a,0,sizeof a);drum909_init(&a);
      for(int v=0;v<DR_NUM;v++)drum909_trigger(&a,v,0.8f);
      b=a;memset(x,0,sizeof x);memset(y,0,sizeof y);
      int n=lengths[q];
      drum909_render_st_cleared(&a,x[0],x[1],x[2],x[3],256,0x7ff);
      drum909_render_st_cleared(&b,y[0],y[1],y[2],y[3],n,0x7ff);
      drum909_render_st_cleared(&b,y[0]+n,y[1]+n,y[2]+n,y[3]+n,256-n,0x7ff);
      unsigned non=0;int first=-1;
      for(int c=0;c<4;c++)for(int i=0;i<256;i++)if(memcmp(&x[c][i],&y[c][i],sizeof(float))){++non;if(first<0 || i<first)first=i;}
      divergent+=!!non;
      printf("256 vs %d+%d: %u PCM words differ, first frame %d, full state differs=%d\n",n,256-n,non,first,memcmp(&a,&b,sizeof a)!=0);
    }
    return divergent==0;
}
