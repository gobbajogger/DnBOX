#ifndef ROOM_HBITS
#error select history representation
#endif
#if ROOM_HBITS == 8
typedef uint8_t room_word;
static room_word compress(float x){
    uint32_t f; memcpy(&f,&x,4);
    unsigned s=f>>31, v=f&0x7fffffffu;
    v += ((1u<<20)-1) + ((v>>21)&1u);
    unsigned e=v>>23;
    if (e<98u) return (room_word)(s<<7);
    if (e>129u) return (room_word)((s<<7)|127);
    return (room_word)((s<<7)|((e-98)<<2)|((v>>21)&3));
}
static float expand(room_word v){
    float x;
    uint32_t f=v&127u?((uint32_t)(v&128u)<<24)|(((uint32_t)((v>>2)&31u)+98u)<<23)|((v&3u)<<21):((uint32_t)(v&128u)<<24);
    memcpy(&x,&f,4);return x;
}
#elif ROOM_HBITS == 16
typedef uint16_t room_word;
static room_word compress(float x){
    uint32_t f;memcpy(&f,&x,4);
    return (room_word)((f + 0x7fffu + ((f>>16)&1u))>>16);
}
static float expand(room_word v){
    uint32_t f=(uint32_t)v<<16;float x;memcpy(&x,&f,4);return x;
}
#elif ROOM_HBITS == 169
typedef uint16_t room_word;
static room_word compress(float x){
    uint32_t f;memcpy(&f,&x,4);
    unsigned s=f>>31, v=f&0x7fffffffu;
    v += ((1u<<13)-1) + ((v>>14)&1u);
    unsigned e=v>>23;
    if(e<74u) return (room_word)(s<<15);
    if(e>137u) return (room_word)((s<<15)|32767u);
    return (room_word)((s<<15)|((e-74u)<<9)|((v>>14)&511u));
}
static float expand(room_word v){
    float x;
    uint32_t sign=(uint32_t)(v&32768u)<<16;
    uint32_t f=v&32767u?sign|((((v>>9)&63u)+74u)<<23)|((v&511u)<<14):sign;
    memcpy(&x,&f,4);return x;
}
#else
#error invalid width
#endif
static room_word *room_words(sp_revsc_dl *p){return (room_word*)p->buf;}
