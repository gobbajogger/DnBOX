from pathlib import Path
import sys
p=Path(sys.argv[1])
s=(p/'actual-clip-key-host.inc').read_text()
s=s.replace('static void lcd_sync(void);','''#define X0X_UI_SMALL_CANVAS 1
static void lcd_sync(void);''')
s=s.replace('static void ck_page_draw(void);','static void unused_legacy_ck_page_draw(void);')
# Only the retained older controller test calls the no-args entry point.
s=s.replace('ck_page_draw();','unused_legacy_ck_page_draw();')
import re
for name in ('mix_strip_names','part_fit'):
    m=re.search(r'static UI_DRAW [^\n]*\b'+name+r'\(',s)
    assert m
    end=s.index('\n}',m.start())+2
    s=s[:m.start()]+s[end:]
s=s.replace('static UI_DRAW ','static ')
h='''#include <assert.h>
enum { KNOB_Y=158 };
static unsigned blit_hash[4]; static uint8_t cv_tile_valid[4];
static uint16_t observed[240*82]; 
static unsigned packets, covered, mutations;
static void cv_lcd_full(uint32_t x,uint32_t y)
{
    assert(x==0 && cv_w==240 && y==KNOB_Y+covered);
    for(unsigned row=0;row<cv_h;row+=3u) {
        unsigned n=cv_h-row<3u?cv_h-row:3u;
        __builtin_memcpy(observed+covered*240,cv_px+row*240,n*240*2);
        covered+=n; ++packets;
        if(packets==1) {
            ++mutations; palette_set(5); proj.set.ck_root^=3u; ck.on=0; ck.pg=0; ck.arp_note=0;
        }
    }
}
static void unused_legacy_ck_page_draw(void) {}
'''
s=s.replace('#include "actual-clip-key-renderers.inc"',h+'\n#include "actual-clip-key-renderers.inc"')
(p/'actual-clip-key-host.inc').write_text(s)
