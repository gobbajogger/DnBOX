/* Host-only proof: one initialization, serial access, bounded private copy.
 * Trust pin is external to input. Not an embedded ABI or storage proposal. */
#include <openssl/sha.h>
enum { FIX_ROWS=640, FIX_SIZE=496414, FIX_ROW_HEADER=40 };
static const uint8_t fixed_pin[32]={
  0x39,0x3d,0x86,0x56,0x56,0x75,0x93,0xec,0xf3,0x7d,0xeb,0xa4,0x5e,0x05,0xba,0x2d,
  0xe6,0xec,0xeb,0xe2,0x4e,0xb8,0x26,0x7d,0xf8,0x4e,0xdc,0xfc,0x85,0x1a,0x8b,0xa2
};
static uint8_t fixed_copy[FIX_SIZE];
static uint32_t fixed_index[FIX_ROWS];
static int fixed_ready;
static int fixed_open(const uint8_t *in, size_t size)
{
    const uint8_t header[]={ 'S','J','6','F',1,0,0x80,2 };
    uint8_t digest[32];
    fixed_ready=0;
    if (!in || size!=FIX_SIZE) return 0;
    memcpy(fixed_copy,in,size);
    if (!SHA256(fixed_copy,size,digest) || memcmp(digest,fixed_pin,32) ||
        memcmp(fixed_copy,header,sizeof header)) return 0;
    size_t i=sizeof header;
    unsigned banks=0, midi=0,last_pos=0;
    const uint8_t *last=NULL;
    for (unsigned ord=0;ord<FIX_ROWS;++ord) {
        if (size-i<FIX_ROW_HEADER) return 0;
        const uint8_t *r=fixed_copy+i;
        if (((unsigned)r[0]+256u*r[1])!=ord ||
            !r[5] || r[5]>92 || !r[6] || r[6]>92 || !r[7] || r[7]>240) return 0;
        if (!last || r[2]!=midi) {
            unsigned next=banks ? banks+1u : 0;
            if (r[2]!=next || r[3]!=0 ||
                (last && !memcmp(r+8,last+8,16))) return 0;
            ++banks;
        } else if (r[3]!=last_pos+1u || memcmp(r+8,last+8,16)) return 0;
        size_t len=FIX_ROW_HEADER+r[5]+r[6]+5u*r[7];
        if (len>size-i) return 0;
        fixed_index[ord]=(uint32_t)i;
        i+=len;
        last=r; midi=r[2]; last_pos=r[3];
    }
    if (i!=size || banks!=12) return 0;
    return fixed_ready=1;
}
static const uint8_t *fixed_record(unsigned ord)
{ return fixed_ready && ord<FIX_ROWS ? fixed_copy+fixed_index[ord] : NULL; }
static const uint8_t *fixed_fields(const uint8_t *r)
{ return r+FIX_ROW_HEADER+r[5]+r[6]; }
static uint32_t fixed_word(const uint8_t *f)
{ return f[1] | ((uint32_t)f[2]<<8) | ((uint32_t)f[3]<<16) | ((uint32_t)f[4]<<24); }
/* Explicitly named namespace. Never pass these ordinals to legacy preset APIs.
 * Caller supplies both immutable IDs; they must match the selected ordinal. */
static int fixed_audition(unsigned owner,unsigned ord,const uint8_t *bank,const uint8_t *program)
{
    const uint8_t *r=fixed_record(ord);
    if (owner>=2 || !r || !bank || !program ||
        memcmp(r+8,bank,16) || memcmp(r+24,program,16)) return 0;
    sj6_factory_row row={0};
    const uint8_t *f=fixed_fields(r);
    for (unsigned n=0;n<r[7];++n,f+=5) sj6_factory_add_word(&row,f[0],fixed_word(f));
    return engine_superj6_factory_load(owner,&row);
}
