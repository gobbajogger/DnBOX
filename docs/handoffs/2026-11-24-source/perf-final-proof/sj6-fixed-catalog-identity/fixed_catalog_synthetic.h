/* SPDX-License-Identifier: GPL-3.0-only */
/* Fenced representation experiment. No firmware/provider dependencies or admission. */
enum { FIXED_N=640, FIXED_SOUND_FIRST=4, FIXED_SOUND_EXCLUSIVE=644 };
static unsigned pilot_sound_code(const uint8_t *c)
{
    return c[30] | ((unsigned)(c[18] >> 7) << 8) | ((unsigned)(c[19] >> 7) << 9);
}
static int pilot_sound_decode(const uint8_t *c, unsigned *n, int manifest_verified);
static int pilot_sound_store(uint8_t *c, unsigned n, int manifest_verified)
{
    if (!manifest_verified || c[29] != ORIGINAL_SOUND_JUNO || n>=FIXED_N ||
        pilot_sound_code(c)>=FIXED_SOUND_EXCLUSIVE) return 0;
    unsigned previous;
    if (!original_sound_valid(c) && !pilot_sound_decode(c,&previous,1)) return 0;
    unsigned x=FIXED_SOUND_FIRST+n;
    c[30]=(uint8_t)x;
    c[18]=(c[18]&127u)|((x>>1)&128u);
    c[19]=(c[19]&127u)|((x>>2)&128u);
    return 1;
}
static int pilot_sound_decode(const uint8_t *c, unsigned *n, int manifest_verified)
{
    if(!manifest_verified || c[29]!=ORIGINAL_SOUND_JUNO || !n) return 0;
    unsigned x=pilot_sound_code(c);
    if(x<FIXED_SOUND_FIRST || x>=FIXED_SOUND_EXCLUSIVE) return 0;
    uint8_t check[32];
    memcpy(check,c,sizeof check);
    check[18]&=127u;check[19]&=127u;check[30]=0;
    if(!original_sound_valid(check)) return 0;
    *n=x-FIXED_SOUND_FIRST;
    return 1;
}
/* Exactly 64 individually old-invalid marker bytes. Nothing can resolve to
 * a historical preset even if rsv[2] is lost before a decoder sees it. */
static int pilot_pattern_chunk(unsigned byte)
{
    if(byte>=82u && byte<=128u) return (int)(byte-82u);
    if(byte>=159u && byte<=175u) return (int)(byte-159u+47u);
    return -1;
}
static int pilot_pattern_valid(const uint8_t r[3], int manifest_verified)
{
    const int m=(pilot_pattern_chunk(r[0])>=0)||(pilot_pattern_chunk(r[1])>=0);
    for(unsigned v=0;v<2;++v) {
        const unsigned hi=(r[2]>>(4u*v))&15u;
        const int lo=pilot_pattern_chunk(r[v]);
        if(lo>=0) {
            if(!manifest_verified || (hi<<6)+(unsigned)lo>=FIXED_N) return 0;
        } else {
            unsigned k,p;
            if((m && hi) || !bass_pattern_preset_decode(r[v],&k,&p)) return 0;
        }
    }
    return 1; /* no markers: retain the whole old rsv[2] byte, unclaimed */
}
static int pilot_pattern_read(const uint8_t r[3],unsigned v,unsigned *n,int manifest_verified)
{
    if(v>=2 || !n || !pilot_pattern_valid(r,manifest_verified)) return 0;
    int lo=pilot_pattern_chunk(r[v]);
    if(lo<0) return 0;
    *n=(((r[2]>>(4u*v))&15u)<<6)+(unsigned)lo;
    return 1;
}
static int pilot_pattern_store(uint8_t r[3],unsigned v,unsigned n,int manifest_verified)
{
    if(!manifest_verified || v>=2 || n>=FIXED_N || !pilot_pattern_valid(r,1)) return 0;
    if(r[2] && pilot_pattern_chunk(r[0])<0 && pilot_pattern_chunk(r[1])<0) return 0;
    unsigned lo=n&63u, shift=4u*v;
    r[v]=(uint8_t)(lo<47u?82u+lo:159u+lo-47u);
    r[2]=(uint8_t)((r[2]&~(15u<<shift))|((n>>6)<<shift));
    return 1;
}
