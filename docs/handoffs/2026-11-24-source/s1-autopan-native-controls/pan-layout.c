/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/x0x.h"
#include "app/original_runtime.h"
#define OFF(n,t,x) const unsigned int n=__builtin_offsetof(t,x)
const unsigned int partBytes=sizeof(original_runtime_part);
const unsigned int setBytes=sizeof(project_t);
OFF(kind,original_runtime_part,kind);
OFF(sound,project_t,sound);
OFF(rev,project_t,set.snd_rev);
OFF(own0,project_t,sound.v[T_303][8]);
OFF(own1,project_t,sound.v[T_303][10]);
OFF(rate,original_runtime_part,dsp.synthone.e.t[36]);
OFF(amount,original_runtime_part,dsp.synthone.e.t[35]);
OFF(phase,original_runtime_part,dsp.synthone.e.pan_ph);
OFF(authored,original_runtime_part,dsp.synthone.e.pan_authored);
