#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
static uint8_t ed_out[391];
static uint32_t ed_n;
static void ed_b(uint32_t v)
{
    if (ed_n < sizeof ed_out - 1u)
        ed_out[ed_n++] = (uint8_t)(v & 0x7Fu);
}
/* Send the same low-first fixed-width digit stream from all reply types.
 * Keeping this outlined avoids repeating the overflow-clamping put calls. */
static __attribute__((noinline)) void ed_word(uint32_t v, uint32_t digits)
{
    while (digits--) {
        ed_b(v);
        v >>= 7;
    }
}
#define ed_w21(v) ed_word((v), 3u)
int main(void) {
 uint32_t r=0x9e3779b9u;
 static const unsigned widths[] = {2,3,5};
 for(unsigned x = 0; x<120000; ++x) {
    r=r*1664525u+1013904223u;
    unsigned n=widths[x%3], start=x%(sizeof ed_out + 1u);
    uint8_t expected[sizeof ed_out];
    memset(ed_out,0xad,sizeof ed_out); memset(expected,0xad,sizeof expected); ed_n=start;
    unsigned ref=start;
    for (unsigned j=0;j<n;j++) if(ref<sizeof expected-1u) expected[ref++]=(uint8_t)((r>>(7u*j))&127u);
    ed_word(r,n);
    assert(ed_n==ref);
    assert(!memcmp(ed_out,expected,sizeof expected));
 }
 puts("editor: 120,000 exact fixed-width replies including output clipping");
}