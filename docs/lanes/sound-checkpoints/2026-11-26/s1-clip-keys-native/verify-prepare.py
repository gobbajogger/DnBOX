from pathlib import Path
import re,sys
d=Path(sys.argv[1]); u=Path('firmware/src/app/ui.c').read_text()
def extract(start):
 a=u.index(start); b=u.index('{',a); e=b+1; n=1
 while n:
  n+=(u[e]=='{')-(u[e]=='}');e+=1
 return u[a:e]
f=d/'actual-clip-key-host.inc'; s=f.read_text()
s=re.sub(r'static void lcd_blit\([^\n]+', 'static void lcd_blit(uint32_t,uint32_t,uint32_t,uint32_t,const uint16_t *);',s,count=1)
s=s.replace("static void lcd_sync(void) {}","")
k=s.index('#include "apc_b.inc"')
s=s[:k]+'''
#define MAIN_ROWS 138u
#define MAIN_Y 20u
#define UI_DRAW
#define HOME_STRIP_SPLIT 80u
static const char *part_s(int i){(void)i;return "";}
static const char *part_display(int i){return part_s(i);}
static int part_at(int i){return i;}
typedef void (*full_strip_painter)(const void *view);
static void commit_full_strips(uint16_t,full_strip_painter,const void *);
'''+s[k:]
f.write_text(s)
(d/'commit.inc').write_text(extract('static void cv_lcd_emit(')+'\n'+extract('static UI_DRAW void commit_full_strips(')+'\n'+extract('static void paint_clip_wait('))
