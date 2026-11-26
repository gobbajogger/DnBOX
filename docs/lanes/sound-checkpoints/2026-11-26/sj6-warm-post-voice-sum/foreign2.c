#include "original_runtime.h"
#include "s1c/s1c_scales.h"
#include <stdlib.h>
/* Not part of Raw (no S1 owner and Scale=0): any unintended ingress fails closed. */
void s1_part_key_on(s1_dry_part *p,unsigned key,unsigned v){abort();}
float s1c_shared_scale_hz(unsigned index,int note,float a4,unsigned npo,
                          const uint8_t *head,unsigned head_bytes,const uint8_t *tail){abort();}
