#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define x0x_lcd_tile_hash ref_hash
#define x0x_lcd_tile_commit ref_commit
#define x0x_lcd_tile_pack ref_pack
#include "owner48-display-hash-reference.h"
#undef X0X_LCD_TILES_H
#undef x0x_lcd_tile_hash
#undef x0x_lcd_tile_commit
#undef x0x_lcd_tile_pack
#include "owner48-display-hash-candidate.h"
typedef struct { uint32_t x,y,w,h; } box;
typedef struct { box b[300]; unsigned n; } events;
static void record(void *context,const uint16_t *px,uint32_t stride,uint32_t x,uint32_t y,uint32_t w,uint32_t h) {
    events *e=context;if(e->n>=300 || stride!=240 || (uint64_t)w*h*2>1536)abort();
    e->b[e->n++]=(box){x,y,w,h};(void)px;
}
static uint32_t rng=0x315478d7;
static uint32_t next(void) { rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng; }
int main(void) {
    static uint16_t px[240*82];static uint32_t old[90],now[90];
    uint64_t hashes=0,writes=0,frames=0;
    for(unsigned run=0;run<20000;++run){
        const unsigned rows=1+next()%82;
        for(unsigned i=0;i<100;++i){const unsigned k=next()%(240*82);px[k]=(uint16_t)next();}
        for(unsigned y=0;y<rows;y+=16){
            const unsigned h=rows-y<16?rows-y:16;
            for(unsigned col=0;col<15;++col){
                if(ref_hash(px,240,col*16,y,h)!=x0x_lcd_tile_hash(px,240,col*16,y,h))return 1;
                ++hashes;
            }
        }
        events a={0},b={0};
        const int force=run%29==0;
        ref_commit(px,rows,old,force,record,&a);
        x0x_lcd_tile_commit(px,rows,now,force,record,&b);
        if(memcmp(old,now,sizeof old)||a.n!=b.n||memcmp(a.b,b.b,a.n*sizeof a.b[0]))return 2;
        writes+=a.n;++frames;
    }
    printf("PASS %llu exact hashes, %llu frames, %llu identical DMA rectangles\n",
        (unsigned long long)hashes,(unsigned long long)frames,(unsigned long long)writes);
}
