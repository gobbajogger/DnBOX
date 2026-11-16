#ifndef S1_READER_MORPH_COMPUTE_H
#define S1_READER_MORPH_COMPUTE_H
#include "direct_reader.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "soundpipe.h"
#include "soundpipeextension.h"
/* Private runtime path over admitted original oscillator metadata and a direct
 * immutable binding. No arbitrary callback or silent sample substitution. */
int s1_runtime_morph_compute(sp_data *,sp_oscmorph2d *,SPFLOAT *,SPFLOAT *,const s1_morph_binding *);
#ifdef __cplusplus
}
#endif
#endif
