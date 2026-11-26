#include "drum909.h"
d9_future_t receipt_private_owner;
char receipt_voice_size[sizeof(d9_smp_t)];
char receipt_frames_cap[D9_FUTURE_MAX];
char receipt_pcm_offset[__builtin_offsetof(d9_future_t,pcm)];
