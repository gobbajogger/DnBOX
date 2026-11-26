#include "drum909.h"
#define O(type,f) const unsigned char off_##f[__builtin_offsetof(type,f)+1]={0};
#define S(type) const unsigned char size_##type[sizeof(type)]={0};
S(d9_smp_t) S(drum909_t)
O(drum909_t,smp)
O(d9_smp_t,playing)
O(d9_smp_t,mute)
O(d9_smp_t,adpcm)
O(d9_smp_t,pos)
O(d9_smp_t,len)
O(d9_smp_t,frac)
O(d9_smp_t,inc)
O(d9_smp_t,incf)
O(d9_smp_t,huff_bit)
