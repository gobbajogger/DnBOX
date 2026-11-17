# SPDX-License-Identifier: GPL-3.0-only
"""Checked instrumentation in a disposable source copy, never production code."""
from pathlib import Path
import importlib.util
HERE=Path(__file__).resolve().parent

def once(s,a,b):
    if s.count(a)!=1: raise ValueError(f'expected exactly one hook: {a[:70]}')
    return s.replace(a,b,1)

def instrument(root):
    p=root/'firmware/src/gfx.c';s=p.read_text()
    s=once(s,'static void cv_begin(', (HERE/'trace.inc').read_text()+'\nstatic void cv_begin(')
    s=once(s,'    uint32_t i, n;','    geo_ntile=0;memset(pixel_tile_owner,0,sizeof pixel_tile_owner);\n    uint32_t i, n;')
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

    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    a='static UI_DRAW void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_GLYPH"))return;\n    if(geo_enabled){if(presence_nglyph>=256)exit(2);unsigned *q=presence_glyph[presence_nglyph++];q[0]=r.kind;q[1]=r.a;q[2]=r.b;q[3]=r.c;}\n')
    a='static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_VALUE"))return;');p.write_text(s)

    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='        cv_text(8, vc(sel ? &FONT_B : &FONT_S, y + 1, 18), sel ? &FONT_B : &FONT_S, caps_of(pref_name(r)), sel ? C_WHITE : C_GRAY);'
    s=once(s,a,'        if(!getenv("PRESENCE_MUTATE_LIST"))'+a.strip())
    a='static UI_DRAW void draw_param_widget(pref_t r, int x, int y, int width, int active, uint16_t col, int height, int night)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_SYNTH"))return;');p.write_text(s)

    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='        night_label_c(cx-1, vc(&FONT_XS, 13, 10), &FONT_XS, caps_of(pref_name(r)), touched ? C_WHITE : C_GRAY);'
    s=once(s,a,'        if(!getenv("PRESENCE_MUTATE_STRIP"))'+a.strip());p.write_text(s)

    # Full-row drum envelopes are audited as real curve pixels, never exempted
    # from the glyph contract. Snapshot only the curve: baseline/text cannot
    # satisfy a missing-curve negative.
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='    for(int x=1;x<=214;x++) {'
    s=once(s,a,'''    uint16_t curve_before[CV_MAX];pixel_snapshot(curve_before);
    if(getenv("PIXEL_MUTATE_SHIFT")){x0+=80;prevx+=80;}
    if(!getenv("PRESENCE_MUTATE_GLYPH")&&!getenv("ENVELOPE_MUTATE_CURVE"))
'''+a)
    a='    char num[16],unit[8],label[32];pref_value_of(r,matrix_value(r),num,unit);'
    s=once(s,a,'''    if(getenv("PIXEL_MUTATE_SHIFT"))x0-=80;
    uint32_t curve_owner=1u+((uint32_t)r.kind<<24)+((uint32_t)r.a<<16)+((uint32_t)r.b<<8)+r.c;
    if(geo_enabled&&(getenv("PIXEL_MUTATE_BLANK")||getenv("ENVELOPE_MUTATE_BLANK")))memcpy(cv_px,curve_before,cv_w*cv_h*sizeof *curve_before);
    pixel_finish(curve_before,curve_owner);
    if(geo_enabled&&getenv("PIXEL_MUTATE_OVERPAINT"))memcpy(cv_px,curve_before,cv_w*cv_h*sizeof *curve_before);
    if(geo_enabled&&!getenv("PRESENCE_MUTATE_GLYPH")&&!getenv("ENVELOPE_MUTATE_CURVE")){if(presence_nglyph>=256)exit(2);unsigned *q=presence_glyph[presence_nglyph++];q[0]=r.kind;q[1]=r.a;q[2]=r.b;q[3]=r.c;}
'''+a);p.write_text(s)

    helper=root/'tests/ui_defects/first_slice/instrument.py'
    spec=importlib.util.spec_from_file_location('first_slice_wrappers',helper);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    sig='static UI_DRAW void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)'
    wrapper=sig+'\n{\n    uint16_t before[CV_MAX];pixel_snapshot(before);\n    first_original_dv_glyph(r,ev,ox+(getenv("PIXEL_MUTATE_SHIFT")?80:0),oy,sx,sy,col);\n    uint32_t owner=1u+((uint32_t)r.kind<<24)+((uint32_t)r.a<<16)+((uint32_t)r.b<<8)+r.c;\n    if(geo_enabled&&getenv("PIXEL_MUTATE_BLANK"))memcpy(cv_px,before,cv_w*cv_h*sizeof *before);\n    pixel_finish(before,owner);\n    if(geo_enabled&&getenv("PIXEL_MUTATE_OVERPAINT"))memcpy(cv_px,before,cv_w*cv_h*sizeof *before);\n}\n'
    s=module.wrap(s,sig,'dv_glyph',wrapper);p.write_text(s)
