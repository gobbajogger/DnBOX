/* Current authorized signed reference points used only in isolated lab RAM.
 * Main1' unique x after compact oscillator gain and before Sub sum/OTA.
 * All controller and untouched output/provider logic stay in dnbx. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
static const float *v;
static const uint8_t *allowed;
static unsigned len,begin,part,use;
static unsigned n[4],source[4];
static double ref2[4],err2[4];
__attribute__((visibility("default"))) void set_main(const float *x,const uint8_t *a,unsigned frames,unsigned op){
 v=x;allowed=a;len=frames;use=op;
 for(unsigned j=0;j<4;j++){n[j]=source[j]=0;ref2[j]=err2[j]=0.;}
}
__attribute__((visibility("default"))) void main_readback(unsigned *counts,double *s) {
 for(unsigned j=0;j<4;j++){counts[j]=n[j];counts[4+j]=source[j];s[j]=ref2[j];s[4+j]=err2[j];}
}
void evidence_block(unsigned i,unsigned frames){begin=i;part=frames;}
float evidence_main(unsigned k,unsigned key,float fader,float original){
 if(!v || fader==0.f)return original;
 assert(k<part && begin+k<len);
 unsigned j;
 switch(key){case 36:j=0;break;case 48:j=1;break;case 52:j=2;break;case 55:j=3;break;default:assert(0);}
 unsigned p=j*len+begin+k;
 float proposed=v[p];double d=(double)original-proposed;
 ++n[j];source[j]+=allowed[p]!=0;ref2[j]+=(double)proposed*proposed;err2[j]+=d*d;
 return use?proposed:original;
}
