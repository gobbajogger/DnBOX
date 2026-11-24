/* Host prototype / standalone target size experiment, not firmware. */
#include <stdint.h>
#include <stddef.h>

static unsigned rd16(const uint8_t *p)
{ return p[0] | (unsigned)p[1] << 8; }

/* A bank's offset table and name blocks are bounded separately. Input metadata
 * is checked by the caller; no decoded name is accepted on a malformed row. */
int cat_name(const uint8_t *offsets, unsigned nblocks, const uint8_t *namebytes,
             unsigned length, unsigned shift, unsigned n, unsigned row,
             uint8_t *out, unsigned outcap)
{
    if (!offsets || !namebytes || !out || !outcap || shift<2 || shift>5 ||
        row>=n || nblocks!=((n-1)>>shift)+1) return 0;
    const unsigned block=row>>shift;
    unsigned start=rd16(offsets+(block<<1));
    unsigned end=block+1<nblocks?rd16(offsets+((block+1)<<1)):length;
    if (start>=end || end>length) return 0;
    unsigned prefix=0, size=0;
    for (unsigned r=(block<<shift); r<=row; ++r) {
        if (start>=end) return 0;
        unsigned packed=namebytes[start++];
        prefix=packed>>4;
        unsigned suffix=packed&15;
        if (prefix==15) {
            if(start>=end || namebytes[start]<15) return 0;
            prefix=namebytes[start++];
        }
        if (suffix==15) {
            if(start>=end || namebytes[start]<15) return 0;
            suffix=namebytes[start++];
        }
        if (prefix>size || suffix>end-start || prefix+suffix>=outcap) return 0;
        for (unsigned i=0;i<suffix;++i) out[prefix+i]=namebytes[start+i];
        start+=suffix;
        size=prefix+suffix;
    }
    out[size]=0;
    return 1;
}
