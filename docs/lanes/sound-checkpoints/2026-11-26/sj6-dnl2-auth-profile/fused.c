#include "fused.h"
void sj6_exact_emit(void *v,unsigned id,uint32_t word)
{
    sj6_exact_words *x=v;
    unsigned bit;
    switch(id){
    case 28:bit=1;break;
    case 107:bit=2;break;
    case 108:bit=4;break;
    case 109:bit=8;break;
    case 180:bit=16; x->base180=word;break;
    default:return;
    }
    unsigned s=x->seen;
    if(s&bit)s|=64u;
    if(bit<16u && word)s|=32u;
    x->seen=(uint8_t)(s|bit);
}
int sj6_exact_finish(const sj6_exact_words *x,unsigned trusted_ordinal)
{
    const unsigned s=x->seen;
    if((s&64u) || ((s&16u) && x->base180>0x3f800000u))return SJ6_EXACT_BAD;
    if(!(s&16u))return SJ6_EXACT_NONE;
    return s==31u && trusted_ordinal==1u ? SJ6_EXACT_CONNECTED_ZERO : SJ6_EXACT_BASE_READY;
}
