/* Standalone COPY restoration cost only. Input is untrusted until a validated
 * DNP parser passed a private staged row. Never publish a failing row. */
#include <stdint.h>
#define COPY UINT32_C(0x7fc00001)
#define PREV UINT32_C(0x7fc00002)

int restore_copy(const uint16_t *layout, uint32_t *staged, unsigned count,
                 const uint16_t (*sorted_refs)[2], unsigned nrefs)
{
    if (!layout || !staged || !sorted_refs || !count || count>167 || nrefs>174) return 0;
    for (unsigned i=0;i<count;++i) {
        unsigned dst=layout[i], a=0, b=nrefs;
        if (dst>=174u || staged[i]==PREV) return 0;
        if (staged[i]!=COPY) continue;
        while (a<b) {
            unsigned c=a+(b-a)/2;
            if (sorted_refs[c][0]<dst) a=c+1; else b=c;
        }
        if (a==nrefs || sorted_refs[a][0]!=dst || sorted_refs[a][1]>=174u) return 0;
        unsigned j=i,p=sorted_refs[a][1];
        while (j && layout[j-1]!=p) --j;
        if (!j) return 0;
        staged[i]=staged[j-1];
    }
    return 1;
}
