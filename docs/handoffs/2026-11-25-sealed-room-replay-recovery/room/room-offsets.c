#include "app/original_runtime.h"
#define OFF(n) __builtin_offsetof(original_runtime_part, n)
const unsigned room_witness_layout[] = {
    sizeof(original_runtime_part), OFF(dsp.synthone.e.rv.fb), OFF(dsp.synthone.e.rv.m),
    OFF(dsp.synthone.e.ddec), OFF(dsp.synthone.e.t[S1C_P_DLON]),
    OFF(dsp.synthone.e.t[S1C_P_RVON]), OFF(dsp.synthone.e.t[S1C_P_RVMIX]),
};
