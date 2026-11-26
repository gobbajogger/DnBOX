/* Reuse an existing authorized, mapped DNL2 read and selection proof.
 * No header reparse, owner state, bank IO, allocations or sample callbacks. */
#include <stdint.h>
typedef struct {
    uint32_t base180;
    uint8_t seen;
} sj6_exact_words;
/* Caller zero-initializes this cold scratch and plumbs canonical ID/words
 * from its existing checked dnbank_read callback. This never grants trust. */
void sj6_exact_emit(void *,unsigned,uint32_t);
enum {
    SJ6_EXACT_BAD=-1, SJ6_EXACT_NONE=0, SJ6_EXACT_BASE_READY=1,
    SJ6_EXACT_CONNECTED_ZERO=2
};
/* The ordinal bit MUST have been obtained from the already-authorized,
 * immutable bank and externally verified companion transaction. It must
 * be internal verifier state, never an imported/unvalidated bool. */
int sj6_exact_finish(const sj6_exact_words *,unsigned trusted_ordinal);
