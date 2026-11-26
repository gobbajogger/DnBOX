#pragma once
#include <stddef.h>
void *micro_new(void);
void micro_drop(void *);
size_t micro_size(void);
void micro_config(float,float,float,float);
void micro_tick(void *,float,float,float,float *,float *);
void micro_run(void *,const float*,float*,const float*,unsigned);
