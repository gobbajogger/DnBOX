/* Experimental cold plan, no shipping importer. Existing DNL2 bank unchanged. */
#include "dnbank.h"
typedef struct {
    uint32_t cutoff_base_180;
    uint8_t has_cutoff_180;
    uint8_t exact_hp_zero_crusher;
} sj6_exact_plan;
/* Authority verifies the signed/pinned companion and actual decoded bank
 * bytes (including header SHA claim). CRC alone MUST NOT authorize it. */
typedef int (*sj6_profile_trust)(void *,const dnbank *,const uint8_t *,size_t);
int sj6_exact_stage(const dnbank *,unsigned,const uint8_t *,size_t,
                    sj6_profile_trust,void *,sj6_exact_plan *);
