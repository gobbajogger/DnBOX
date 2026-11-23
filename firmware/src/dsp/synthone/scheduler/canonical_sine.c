#include <stddef.h>
#include <stdint.h>
#include "canonical_sine_data.h"
static uint32_t get_bits(const uint8_t*payload,unsigned at,unsigned width){uint32_t z=0;unsigned used=0;while(width){unsigned shift=at&7u,take=8u-shift;if(take>width)take=width;z|=((payload[at>>3]>>shift)&((1u<<take)-1u))<<used;at+=take;used+=take;width-=take;}return z;}
static uint32_t quarter(unsigned q){if(q==1024)return 0x3f800000u;unsigned lo=0,hi=SINE_SEGMENTS;while(lo+1<hi){unsigned m=(lo+hi)/2;if(sine_starts[m]<=q)lo=m;else hi=m;}
 unsigned k=sine_powers[lo],j=q-sine_starts[lo];uint32_t a=sine_anchors[lo],b=lo+1<SINE_SEGMENTS?sine_anchors[lo+1]:0x3f800000u,d=b-a;
 uint32_t base=a+(d>>k)*j+(((d&((1u<<k)-1u))*j)>>k);uint32_t z=get_bits(sine_payload,sine_offsets[lo]+j*sine_widths[lo],sine_widths[lo]);return base+((z&1u)?0u-((z>>1)+1u):(z>>1));}
int s1_lossless_sine_read(unsigned index,uint32_t*out){if(!out||index>4096)return 0;if(index==4096){*out=0;return 1;}
 unsigned q;uint32_t sign=0;if(index<1024)q=index;else if(index<2048)q=2048-index;else if(index<3072){q=index-2048;sign=0x80000000u;}else{q=4096-index;sign=0x80000000u;}uint32_t word=quarter(q)^sign;
 if(index>=1024){unsigned lo=0,hi=SINE_CORRECTIONS;while(lo+1<hi){unsigned m=(lo+hi)/2;if(correction_starts[m]<=index)lo=m;else hi=m;}
 uint32_t z=get_bits(correction_payload,correction_offsets[lo]+(index-correction_starts[lo])*correction_widths[lo],correction_widths[lo]);word+=correction_bases[lo]+((z&1u)?0u-((z>>1)+1u):(z>>1));}
 *out=word;return 1;}
