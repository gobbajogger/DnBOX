# SPDX-License-Identifier: GPL-3.0-only
"""Checked instrumentation in a disposable source copy, never production code."""
from pathlib import Path
HERE=Path(__file__).resolve().parent

def once(s,a,b):
    if s.count(a)!=1: raise ValueError(f'expected exactly one hook: {a[:70]}')
    return s.replace(a,b,1)

def instrument(root):
    p=root/'firmware/src/gfx.c';s=p.read_text()
    s=once(s,'static void cv_begin(', (HERE/'trace.inc').read_text()+'\nstatic void cv_begin(')
    s=once(s,'    uint32_t i, n;','    geo_ntile=0;\n    uint32_t i, n;')
    s=once(s,'{ lcd_blit(x, y, cv_w, cv_h, cv_px); }','{ geo_blit(x,y); lcd_blit(x, y, cv_w, cv_h, cv_px); }')
    s=once(s,'    if (r0 < cv_h)\n        lcd_blit','    geo_blit(x,y);\n    if (r0 < cv_h)\n        lcd_blit')
    a='    uint32_t r = c >> 11, g = (c >> 5) & 63u, b = c & 31u;'
    s=once(s,a,'    const char *geo_text=s; int geo_x=x,geo_y=y+cv_oy; unsigned geo_visible=0;\n'+a)
    s=once(s,'                if (a == 15u) {','                geo_visible++;\n                if (a == 15u) {')
    a='    return x;\n}\n\nstatic int32_t cv_text('
    s=once(s,a,'''    if(geo_enabled) {
     if(geo_ntile>=256 || strlen(geo_text)>=128){fprintf(stderr,"geometry text overflow\\n");exit(1);}
     geo_label *e=&geo_tile[geo_ntile++];e->f=f;e->x=geo_x;e->y=geo_y;e->col=c;strcpy(e->text,geo_text);
    }
'''+a);p.write_text(s)
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    s=once(s,'static void cv_lcd_full(uint32_t x, uint32_t y)\n{','static void cv_lcd_full(uint32_t x, uint32_t y)\n{\n    geo_blit(x,y);')
    s=once(s,'    cv_lcd_pos_t pos = {x, y};','    geo_blit(x,y);\n    cv_lcd_pos_t pos = {x, y};')
    # Only the semantic face must survive. The two decorative shadow layers are
    # intentionally overpainted by that face; their ink is not missing text.
    s=once(s,'    cv_text(x,y+1,f,s,UI_N_LEFT); /* shallow incised lower light */','    unsigned geo_saved=geo_ntile;\n    cv_text(x,y+1,f,s,UI_N_LEFT); /* shallow incised lower light */')
    s=once(s,'    cv_text(x,y,f,s,ink);       /* retain the native face, metrics and ink */','    geo_ntile=geo_saved;\n    cv_text(x,y,f,s,ink);       /* retain the native face, metrics and ink */')
    p.write_text(s)
