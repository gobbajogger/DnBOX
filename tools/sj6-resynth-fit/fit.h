/* Host-only fitter. The firmware renderer stays libm-free. */
#ifndef SJ6_FIT_H
#define SJ6_FIT_H
#include "sj6_resynth.h"
#ifdef __cplusplus
extern "C" {
#endif
/* loop_start < 0 means the caller did not supply loop points. */
int sj6_fit(const int32_t *x, int n, int rate, int loop_start, int loop_end,
            const char *name, Sj6Voice *out);
#ifdef __cplusplus
}
#endif
#endif
