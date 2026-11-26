#include "exact-profile.h"
enum { PROFILE_HEADER=40, PROFILE_SUPPORTED=1 };
struct collect {
    uint32_t cutoff;
    uint8_t seen, weak, duplicate;
};
static void field(void *context,unsigned id,uint32_t word)
{
    struct collect *c=context;
    unsigned bit=0,expected=0;
    switch(id) {
    case 28:bit=1;break;                   /* authored HP zero */
    case 107:bit=2;break;                  /* panel zero: connected, no 1.3 booster (verified separately) */
    case 108:bit=4;break;                  /* amount zero */
    case 109:bit=8;break;                  /* explicit Lo-Fi zero */
    case 180:bit=16;c->cutoff=word;break;  /* exact raw source Float32 */
    default:return;
    }
    if(c->seen&bit)c->duplicate=1;
    c->seen|=bit;
    if(bit!=16 && word!=expected)c->weak=1;
}
int sj6_exact_stage(const dnbank *b,unsigned preset,const uint8_t *capsule,size_t n,
                    sj6_profile_trust trust,void *context,sj6_exact_plan *out)
{
    sj6_exact_plan temp={0,0,0};
    struct collect c={0,0,0,0};
    char name[48];
    if(!b || !out || b->engine!=3 || preset>=b->programs ||
       dnbank_read(b,preset,name,field,&c)<=0 || c.duplicate)return 0;
    if(c.seen&16u) {
        if(c.cutoff>0x3f800000u)return 0; /* rejects noncanonical -0 and invalid range */
        temp.cutoff_base_180=c.cutoff;temp.has_cutoff_180=1;
    }
    /* Capsule SJP1; v1; exact 44.1k physical HP10 + active-zero profile code.
     * offsets 6..7 count LE, 8..39 decoded-bank SHA256, 40.. bitmap by ordinal.
     * Only the external authority checks source measurements/signature and
     * that SHA against all exact underlying DNL2 bank bytes. */
    if(capsule && trust && !c.weak && (c.seen&31u)==31u &&
       n==PROFILE_HEADER+(b->programs+7u)/8u &&
       capsule[0]=='S' && capsule[1]=='J' && capsule[2]=='P' && capsule[3]=='1' &&
       capsule[4]==1 && capsule[5]==PROFILE_SUPPORTED &&
       (capsule[6]|(unsigned)capsule[7]<<8)==b->programs &&
       (!(b->programs&7u) || !(capsule[n-1]>>(b->programs&7u))) &&
       ((capsule[PROFILE_HEADER+(preset>>3)]>>(preset&7u))&1u) &&
       trust(context,b,capsule,n))
        temp.exact_hp_zero_crusher=1;
    *out=temp;
    return 1;
}
