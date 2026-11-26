from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/firmware/src/dsp/drum909.c';t=p.read_text()
a='''static inline int32_t d9_smp_decode(d9_smp_t *s, const uint8_t *stream)
{
    const uint32_t cursor = s->huff_bit;
    const uint8_t *p = stream + (cursor >> 3);
    const unsigned bits = p[0] | ((unsigned)p[1] << 8);'''
b='''typedef struct {uint32_t page, word;} d9_huff_window_t;
static inline int32_t d9_smp_decode(d9_smp_t *s, const uint8_t *stream, d9_huff_window_t *w)
{
    const uint32_t cursor=s->huff_bit, page=cursor>>3;
    unsigned bits=w->word;
    if (page!=w->page) {
        /* Refill only for a requested nibble, never speculatively at EOF.
         * Codes are at most six bits; no valid nibble skips a whole byte. */
        const uint8_t *p=stream+page;
        bits=(w->page+1==page ? bits>>8 : p[0]) | ((unsigned)p[1]<<8);
        w->page=page; w->word=bits;
    }'''
# UINT32_MAX+1 wraps to zero; bootstrap a non-successor sentinel.
assert t.count(a)==1;t=t.replace(a,b)
t=t.replace('static inline int32_t d9_smp_decode(d9_smp_t *s, const uint8_t *stream)\n{\n    return ima_decode','typedef struct {uint32_t page, word;} d9_huff_window_t;\nstatic inline int32_t d9_smp_decode(d9_smp_t *s, const uint8_t *stream, d9_huff_window_t *w)\n{\n    (void)w;\n    return ima_decode')
a='    const uint32_t len = s->len;'
assert t.count(a)==1;t=t.replace(a,a+'\n    d9_huff_window_t huff={0xfffffffeu,0};')
t=t.replace('d9_smp_decode(s, adpcm)','d9_smp_decode(s, adpcm,&huff)')
p.write_text(t)
