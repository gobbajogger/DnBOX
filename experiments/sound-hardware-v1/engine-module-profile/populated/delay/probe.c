#include "soundpipe.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
enum { SR=44100, N=4*SR };
static uint8_t seq[N];
static uint32_t random_next(uint32_t *v) {return *v=*v*1664525u+1013904223u;}
int main(void) {
  sp_data sp={.sr=SR};
  sp_vdelay *line[4];
  for(unsigned k=0;k<4;k++){
    assert(sp_vdelay_create(line+k)==SP_OK);
    assert(sp_vdelay_init(&sp,line[k],10.f)==SP_OK);
  }
  unsigned d=0; uint32_t seed=97;
  for(unsigned j=0;j<2*N;j++) {
    if(j<N)seq[j]=(uint8_t)(random_next(&seed)>>31);
    /* Existing short time is 0.25 s; actual left branch reads at 2T.
       After the live smoothing/ramp completes, a held new legal T=2 s
       must retrieve the older data. This witness uses the final step at
       the delay control itself, not a claim about upstream gate ramp. */
    float t=j<N?.25f:2.f;
    float a=j<N?(seq[j]?0x1p-10f:-0x1p-10f):0.f, b=0.f;
    for(unsigned k=0;k<4;k++){line[k]->del=k<2?2.f*t:t;line[k]->feedback=0.f;}
    float l,rl,rs,rr;
    assert(sp_vdelay_compute(&sp,line[0],&a,&l)==SP_OK);
    assert(sp_vdelay_compute(&sp,line[1],&b,&rl)==SP_OK);
    assert(sp_vdelay_compute(&sp,line[2],&b,&rs)==SP_OK);
    assert(sp_vdelay_compute(&sp,line[3],&rl,&rr)==SP_OK); rr+=rs;
    if(j>=N) {
      const float want=seq[j-N]?0x1p-10f:-0x1p-10f;
      assert(memcmp(&l,&want,sizeof want)==0);d++;
    }
    assert(rr==0.f);
  }
  assert(d==N);
  for(unsigned k=0;k<4;k++)sp_vdelay_destroy(line+k);
  printf("original 2T stereo route, T=.25 to 2.0: %u old bits recovered exactly, no post-edit input, one physical lane\n", d);
  printf("arbitrary binary input lower bound: %u B, all grant + both arenas %u B, pair-per-owner %u B\n",d/8,9016u+2u*4252u,9016u/2u+4252u);
}
