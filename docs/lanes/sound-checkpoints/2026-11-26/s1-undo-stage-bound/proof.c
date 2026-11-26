#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "undo.c"
#include "lz.c"
enum { PROJECT_BYTES=36420, SETTINGS_BYTES=40, REGION_BYTES=PROJECT_BYTES-SETTINGS_BYTES };
static uint8_t live[REGION_BYTES], shadow[REGION_BYTES], old[REGION_BYTES];
static undo_t u;
static unsigned notified,first,nlen;
static void changed(uint32_t x,uint32_t n){++notified;first=x;nlen=n;}
static void undo_evidence(void){
    uint32_t lo,hi;
    for(unsigned i=0;i<REGION_BYTES;i++)live[i]=(uint8_t)(i*131u+29u);
    undo_init(&u,live,shadow,sizeof live);
    live[9]^=1u;assert(undo_commit(&u)==1&&u.used==14&&u.n==1);
    memcpy(old,live,sizeof live);
    /* Large first writes, then oscillating values restore all except one before quiet commit. */
    for(unsigned i=1024;i<1024+5000;i++)live[i]^=0x55;
    for(unsigned i=1024;i<1024+5000;i++)if(i!=6019)live[i]^=0x55;
    assert(undo_commit(&u)==1&&u.used==28&&u.n==2);
    assert(undo_undo(&u,changed,&lo,&hi)&&lo==6019&&hi==6019);
    assert(notified==1&&first==6019&&nlen==1&&!memcmp(live,old,sizeof live));
    assert(undo_undo(&u,0,&lo,&hi)&&lo==9&&hi==9);
    assert(undo_redo(&u,0,&lo,&hi)&&lo==9&&hi==9);
    assert(undo_redo(&u,0,&lo,&hi)&&lo==6019&&hi==6019);
    puts("5,000 distinct touches with 4,999 restored: stock kept old entry plus new 14-byte Undo entry; exact undo/redo");
    for(unsigned i=1024;i<1024+5000;i++)live[i]^=0x55;
    assert(undo_commit(&u)==-1&&u.n==0&&u.cur==0&&u.used==0&&!memcmp(live,shadow,sizeof live));
    puts("5,000 net bytes genuinely different: stock clears history and retains edit, no rollback");
}
typedef struct{uint8_t data[26000];uint32_t used,read;} source;
static source z;
static int next(void *p){source *s=p;return s->read<s->used?s->data[s->read++]:-1;}
static void push(uint8_t b){assert(z.used<sizeof z.data);z.data[z.used++]=b;}
static void lz_repeat(uint32_t total){
    while(total>=LZ_MIN){
        unsigned k=total>LZ_MAXLEN?LZ_MAXLEN:total;
        push(0x80u | (k-LZ_MIN));push(0);push(0);total-=k;
    }
    if(total){push((uint8_t)(total-1u));while(total--)push(0);}
}
static void lz_far(void){
    const unsigned dist=24704u, raw=33668u, match=129u;
    push(127u);
    for(unsigned j=0;j<128u;j++)push((uint8_t)(j*7u+3u));
    push(1u);push(0xa9);push(0);
    lz_repeat(dist-130u);
    push(0x80u | (match-LZ_MIN));
    unsigned hi_code=z.used+1u;
    push((uint8_t)(dist-1u));push((uint8_t)((dist-1u)>>8));
    push(0);push(0);
    lz_repeat(raw-dist-match-1u);
    assert(z.used<15332u);
    lz_src_t src={next,&z};assert(lz_decompress(&src,live,raw)==0);
    assert(z.read==z.used&&!memcmp(live,live+dist,match));
    assert(live[dist+match-1u]==0xa9&&live[dist+match]==0);
    printf("Original C decoder: %u-byte packed stream fills %u bytes; true %u-byte history reference\n",z.used,raw,dist);
    z.data[hi_code]=0xffu;z.read=0;
    assert(lz_decompress(&src,live,raw)==-1);
    puts("Malformed beyond-produced distance rejected, not wrapped");
}
int main(void){undo_evidence();lz_far();}
