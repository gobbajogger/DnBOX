/* Known independent signed marker, no sample assets. */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "suffix.h"
static float input[2][44100],a[2][44100],b[2][44100];
int main(void) {
 const double hz[]={32.70319566257483,97.99885899543733};
 const double tau=0x1.921fb54442d18p+2;
 for(unsigned ch=0;ch<2;ch++)for(unsigned i=0;i<44100;i++){
  const double phase=hz[ch]*i*tau/44100;
  input[ch][i]=(float)(.4*cos(phase)-.12*cos(3*phase)+.035*cos(5*phase));
 }
 memcpy(a,input,sizeof a);memcpy(b,input,sizeof b);
 sj6_scoped_dry_suffix first={0},second={0};
 sj6_pre_dry(&first,a[0],a[1],44100);
 sj6_post_dry(&first,a[0],a[1],44100);
 for(unsigned i=0;i<44100;){
  unsigned n=i%127+1;
  if(n>44100-i)n=44100-i;
  sj6_pre_dry(&second,b[0]+i,b[1]+i,n);
  sj6_post_dry(&second,b[0]+i,b[1]+i,n);
  i+=n;
 }
 assert(!memcmp(a,b,sizeof a));
 fprintf(stderr,"irregular callback partition invariant\n");
 assert(fwrite(input,sizeof input,1,stdout)==1 && fwrite(a,sizeof a,1,stdout)==1);
}
