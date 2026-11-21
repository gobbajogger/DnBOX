/* Exact immutable direct-word experiment. No codec or target placement. */
#ifndef S1_DIRECT_READER_H
#define S1_DIRECT_READER_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define S1_MORPH_LOGICAL_ROWS 52u
#define S1_MORPH_UNIQUE_ROWS 50u
#define S1_MORPH_WORDS_PER_ROW 4097u
#define S1_MORPH_DIRECT_BYTES ((size_t)819400u)
typedef struct s1_morph_binding {
    const unsigned char *bytes;
    size_t byte_count;
    uint32_t format_tag;
} s1_morph_binding;
/* Cold admission scans every word/guard. Failure leaves out byte-identical.
 * An admitted binding copies metadata and borrows immutable bytes for its life.
 * No arbitrary callback, external map, cache, allocation or global state. */
int s1_morph_binding_valid(const s1_morph_binding *);
int s1_morph_bind_direct(s1_morph_binding *out,const void *bytes,size_t byte_count);
/* Output is unchanged on failure. Logical rows48/49 map exactly to44/45.
 * All reads are total on admitted immutable bytes and in-domain indices. */
int s1_morph_read_word(const s1_morph_binding *,unsigned logical_row,unsigned sample,uint32_t *out);
#ifdef __cplusplus
}
#endif
#endif
