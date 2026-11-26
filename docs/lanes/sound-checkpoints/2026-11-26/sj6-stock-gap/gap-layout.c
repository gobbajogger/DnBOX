#include <stddef.h>
#include "app/original_runtime.h"
#define PROBE(n,v) const char n[(v)+1u]={0}
PROBE(juno_core_off,offsetof(juno6_t,core));
PROBE(juno_knobs_off,offsetof(juno6_t,macro));
PROBE(juno_pb_off,offsetof(juno6_t,pb_typed));
PROBE(unison_hist_off,offsetof(original_runtime_backend,juno.unison_history));
PROBE(s1_history_off,offsetof(original_runtime_backend,s1_wet_lease.delay));
PROBE(full_size,sizeof(original_runtime_backend));
PROBE(owner_size,sizeof(original_runtime_part));
#if DNBX_SJ6_RECLAIM_STOCK_LAYOUT
PROBE(gap_a, sizeof(((juno6_t*)0)->typed_filter_gap_a));
PROBE(gap_b, sizeof(((juno6_t*)0)->typed_filter_gap_b));
PROBE(gap_c, sizeof(((juno6_t*)0)->typed_filter_gap_c));
#endif
