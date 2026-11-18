# SPDX-License-Identifier: GPL-3.0-only
import hashlib,json,pathlib,sys
out=pathlib.Path(sys.argv[1]);src=pathlib.Path('firmware/src');fixture=pathlib.Path('tests/host/smk_clip_knob_test.c')
s=fixture.read_text()
for anchor in ('#define C_BLACK','enum { O_NONE','static void ck_page_draw(void) {}','int main('):
 assert s.count(anchor)==1,('fixture anchor',anchor)
a=s.index('#define C_BLACK');b=s.index('enum { O_NONE',a)
s=s[:a]+'''static void lcd_sync(void);
static void lcd_blit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t*p){(void)x;(void)y;(void)w;(void)h;(void)p;}
#include "gfx.c"
'''+s[b:]
assert s.count('static void ck_page_draw(void) {}')==1
s=s.replace('static void ck_page_draw(void) {}','static void ck_page_draw(void);').replace('int main(', 'int retained_knob_main(')
u=(src/'app/ui.c').read_text()
for anchor in ('static uint16_t blend(','static uint16_t dim(','/* the radius that suits a box'):
 assert u.count(anchor)==1,('UI anchor',anchor)
a=u.index('static uint16_t blend(');b=u.index('{',a);depth=1;e=b+1
while depth:depth+=(u[e]=='{')-(u[e]=='}');e+=1
helpers=u[a:e]+'\n'+u[u.index('static uint16_t dim('):u.index('/* the radius that suits a box')].replace('#include "ui_glyph_renderer.h"','')
main=(src/'app/main_fm1.c').read_text()
for anchor in ('/* ------- clip mode screen:','/* every main loop pass'):
 assert main.count(anchor)==1,('main anchor',anchor)
a=main.index('/* ------- clip mode screen:');b=main.index('/* every main loop pass',a)
(out/'actual-clip-key-renderers.inc').write_text(main[a:b]);(out/'actual-clip-key-host.inc').write_text(s+'\n'+helpers+'\n#include "actual-clip-key-renderers.inc"\n')
files=[pathlib.Path('tools/gen_font.py'),pathlib.Path(__file__),pathlib.Path('tests/host/ui_clip_keys_render_test.c'),pathlib.Path('tests/host/ui_clip_keys_render_test.sh'),pathlib.Path('tests/host/smk_clip_knob_test.sh'),pathlib.Path('tests/host/smk_clip_knob_fake_usb.h'),fixture,src/'app/ui.c',src/'app/main_fm1.c',src/'gfx.c',src/'app/smk_clip_keys.h',src/'app/smk_clip_arp_share.c']
(out/'provenance.json').write_text(json.dumps({'inputs':{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in files},'generated':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in list(out.glob('*.inc'))+[out/'felucca_font.h']},'scope':'Actual graphics + CK renderer with existing mocked hardware fixture, no device I/O; 240x138 main and240x82 strip, not full-screen qualification.'},indent=2)+'\n')
