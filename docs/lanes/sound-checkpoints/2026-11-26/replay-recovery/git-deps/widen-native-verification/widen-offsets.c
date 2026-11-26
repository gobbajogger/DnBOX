#include "app/original_runtime.h"
#define OFF(n) __builtin_offsetof(original_runtime_part, n)
const unsigned widen_layout[]={
 sizeof(original_runtime_part), OFF(dsp.synthone.e.t[S1C_P_WIDEN]), OFF(dsp.synthone.e.wpos), OFF(dsp.synthone.e.wid.buf), OFF(dsp.synthone.wbuf)
};
