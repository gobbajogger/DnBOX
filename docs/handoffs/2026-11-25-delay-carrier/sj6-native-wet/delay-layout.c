#include "app/original_runtime.h"
#define OFF(n) __builtin_offsetof(original_runtime_part,dsp.juno.synth.delay24.n)
const unsigned delay_layout[]={
 sizeof(original_runtime_part), OFF(tap),OFF(current),OFF(fb),OFF(wet),OFF(inv_stride),OFF(stride),OFF(filled),OFF(mix),OFF(clock_refused),OFF(sync),OFF(enabled)
};
