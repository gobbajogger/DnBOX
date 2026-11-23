#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int s1_lossless_sine_read(unsigned,uint32_t*);
#ifdef __cplusplus
}
#endif
static inline float s1_canonical_sample(const float*data,unsigned i){if(data)return data[i];uint32_t u=0;s1_lossless_sine_read(i,&u);float x;__builtin_memcpy(&x,&u,4);return x;}
