# SPDX-License-Identifier: GPL-3.0-only
"""Checked test-only instrumentation. Frozen inputs never modified."""
from pathlib import Path
import re

HERE = Path(__file__).resolve().parent


def once(text, old, new):
    if text.count(old) != 1: raise ValueError(f'Hook anchor expected once, saw {text.count(old)}: {old[:100]}')
    return text.replace(old,new,1)


def function_end(text,start):
    # Ignore C comments/string literals when finding the closing brace.
    tokens=re.finditer(r'/\*.*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]',text[start:],re.S)
    depth=0; begun=False
    for match in tokens:
        t=match.group()
        if t=='{': depth+=1;begun=True
        elif t=='}':
            depth-=1
            if begun and depth==0:return start+match.end()
    raise ValueError('unterminated C function')


def wrap(text,signature,name,wrapper):
    start=text.index(signature); end=function_end(text,start)
    old=text[start:end]; renamed=once(old,name+'(', 'first_original_'+name+'(')
    return text[:start]+renamed+'\n'+wrapper+'\n'+text[end:]


def active_bank_anchor(text):
    """Recognize actual historical/current paint, never synthesize a panel."""
    anchors=['rbox(6, ay, 228, ah, %d, RGB(36,48,54));'%radius for radius in (4,9)]
    if sum(text.count(anchor) for anchor in anchors)!=1:
        raise ValueError('expected exactly one supported active-bank paint anchor')
    return next(anchor for anchor in anchors if anchor in text)


def instrument(root):
    gfx=root/'firmware/src/gfx.c'; text=gfx.read_text()
    sig='static int32_t cv_text(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)'
    text=once(text,sig,(HERE/'trace_gfx.inc').read_text()+'\n'+sig)
    wrapper=sig+'''\n{
    uint16_t before[CV_MAX];first_snapshot(before);first_record_label(x,y,f,s,c);
    int32_t result=first_original_cv_text(x,y,f,s,c);
    first_finish(before,first_value?"value_text":"text",(float)x,(float)y,s);
    return result;
}'''
    text=wrap(text,sig,'cv_text',wrapper);gfx.write_text(text)
    dv=root/'firmware/src/app/ui_drum_voice.inc';text=dv.read_text()
    draw='UI_DRAW ' if 'static UI_DRAW void dv_glyph(' in text else ''
    text=once(text,'int off = band * BAND_H, bh = dv_band_h(band);','int off = band * BAND_H, bh = dv_band_h(band);\n    first_offset=band==2?158:20;first_control=-1;first_row=-1;')
    text=once(text,'/* labels */','/* labels */\n            first_row=p;first_control=-1;')
    text=once(text,'if (r.kind == R_NONE) continue;','if (r.kind == R_NONE) continue;\n                first_control=k;first_ref[0]=r.kind;first_ref[1]=r.a;first_ref[2]=r.b;first_ref[3]=r.c;first_name=pref_desc(r)?pref_desc(r)->name:"";')
    text=once(text,'y += ah;','first_control=-1;\n            y += ah;')
    text=once(text,'/* lower panel: body pictogram, voice name, P1/16, 16-step strip */','first_row=-1;first_control=-1;\n    /* lower panel: body pictogram, voice name, P1/16, 16-step strip */')
    # Actual panel rectangles, including otherwise-empty body; no padded fake nodes.
    text=once(text,'rbox(3, by, 234, body_h, 9, SOUND_FACE);','{first_panel("control-banks",3,by,234,body_h);rbox(3, by, 234, body_h, 9, SOUND_FACE);}')
    anchor=active_bank_anchor(text)
    text=once(text,anchor,'{first_panel("active-bank",6,ay,228,ah);'+anchor+'}')
    text=once(text,'rbox(3, py, 234, ph, 9, SOUND_FACE);','first_panel("voice-body",3,py,234,ph);rbox(3, py, 234, ph, 9, SOUND_FACE);')
    text=once(text,'cv_rect(sx, sy, 11, 4, sc);','{uint16_t before[CV_MAX];first_snapshot(before);cv_rect(sx, sy, 11, 4, sc);first_finish(before,"step",sx,sy,0);}')
    sig='static '+draw+'void dv_tune_fork(float ox, float oy, float sx, float sy, float tune01, uint16_t col)'
    text=once(text,sig+'\n{',sig+'\n{\n    first_glyph_type="tuning-fork";')
    # Route marker belongs to actual Pitch branch if present, not inferred from name.
    pitch='if (d && name_is(d,"Pitch")) {'
    if pitch in text:text=once(text,pitch,pitch+'\n        first_glyph_type="carrier-wave";')
    sig='static '+draw+'void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)'
    wrapper=sig+'''\n{
    uint16_t before[CV_MAX];first_snapshot(before);first_glyph_type="role-path";
    first_original_dv_glyph(r,ev,ox,oy,sx,sy,col);
    first_ref[0]=r.kind;first_ref[1]=r.a;first_ref[2]=r.b;first_ref[3]=r.c;
    first_name=pref_desc(r)?pref_desc(r)->name:"";
    first_finish(before,"glyph",ox,oy,0);
}'''
    text=wrap(text,sig,'dv_glyph',wrapper)
    sig='static '+draw+'void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large)'
    wrapper=sig+'''\n{
    uint16_t before[CV_MAX];first_snapshot(before);first_value=1;
    first_original_dv_value_text(r,ev_i,cx,y,col,large);first_value=0;
    first_finish(before,"value",cx,y,0);
}'''
    text=wrap(text,sig,'dv_value_text',wrapper)
    # Real body paths are included in ink measurements, but panel fills are not.
    sig='static '+draw+'void dv_body_pictogram('
    start=text.index(sig); brace=text.index('{',start);signature=text[start:brace].strip()
    wrapper=signature+'''\n{
    uint16_t before[CV_MAX];first_snapshot(before);
    first_original_dv_body_pictogram(t,v,x,y,scale,env,col);
    first_finish(before,"body",x,y,0);
}'''
    # Parameter names are read from the actual function signature, not assumed.
    args=signature[signature.index('(')+1:signature.rindex(')')]
    names=[a.strip().split()[-1].lstrip('*') for a in args.split(',')]
    wrapper=signature+'\n{\n    uint16_t before[CV_MAX];first_snapshot(before);\n    first_original_dv_body_pictogram('+','.join(names)+');\n    first_finish(before,"body",'+names[2]+','+names[3]+',0);\n}'
    text=wrap(text,signature,'dv_body_pictogram',wrapper);dv.write_text(text)
    (root/'tests/host/first_slice_capture.c').write_text((HERE/'capture.c').read_text())
