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
    ui=root/'firmware/src/app/ui.c';body=ui.read_text()
    a='    for(int x=1;x<=span;x++) {' if '    for(int x=1;x<=span;x++) {' in body else '    for(int x=1;x<=214;x++) {'
    body=once(body,a,'''    uint16_t envelope_before[CV_MAX];first_snapshot(envelope_before);
    first_glyph_type="envelope-curve";first_ref[0]=r.kind;first_ref[1]=r.a;first_ref[2]=r.b;first_ref[3]=r.c;
    first_name=pref_desc(r)?pref_desc(r)->name:"";
'''+a)
    # Finish immediately after the actual curve, before a compact KIT row
    # can return and before its axis/caption/meter ink enters the snapshot.
    compact='    if(level_time<0)return;'
    a=compact if compact in body else '    char num[16],unit[8],label[32];pref_value_of(r,matrix_value(r),num,unit);'
    y='top' if 'int h,int level_time)' in body else 'top-off'
    body=once(body,a,f'    first_finish(envelope_before,"glyph",x0,{y},0);\n'+a);ui.write_text(body)
    dv=root/'firmware/src/app/ui_drum_voice.inc';text=dv.read_text()
    draw='UI_DRAW ' if 'static UI_DRAW void dv_glyph(' in text else ''
    text=once(text,'int off = band * BAND_H, bh = dv_band_h(band);','int off = band * BAND_H, bh = dv_band_h(band);\n    first_offset=band==2?158:20;first_control=-1;first_row=-1;')
    text=once(text,'/* labels */','/* labels */\n            first_row=p;first_control=-1;')
    text=once(text,'if(adsr_row(p)) {','if(adsr_row(p)) {\n                pref_t graph_ref=pg.r[p][0];first_row=p;first_control=0;\n                first_ref[0]=graph_ref.kind;first_ref[1]=graph_ref.a;first_ref[2]=graph_ref.b;first_ref[3]=graph_ref.c;\n                first_name=pref_desc(graph_ref)?pref_desc(graph_ref)->name:"";')
    text=once(text,'if (r.kind == R_NONE) continue;','if (r.kind == R_NONE) continue;\n                first_control=k;first_ref[0]=r.kind;first_ref[1]=r.a;first_ref[2]=r.b;first_ref[3]=r.c;first_name=pref_desc(r)?pref_desc(r)->name:"";')
    text=once(text,'y += ah;','first_control=-1;\n            y += ah;')
    text=once(text,'/* lower panel: body pictogram, voice name, P1/16, 16-step strip */','first_row=-1;first_control=-1;\n    /* lower panel: body pictogram, voice name, P1/16, 16-step strip */')
    # Actual panel rectangles, including otherwise-empty body; no padded fake nodes.
    text=once(text,'rbox(3, by, 234, body_h, UI_PANEL_RADIUS, SOUND_FACE);','{first_panel("control-banks",3,by,234,body_h);rbox(3, by, 234, body_h, UI_PANEL_RADIUS, SOUND_FACE);}')
    active_panels = (
        'rbox(6, ay, 228, ah, UI_PANEL_RADIUS, ui_theme_light?SOUND_SELECT:RGB(36,48,54));',
        'dv_bank_focus(ay,ah);',
    )
    matches = [anchor for anchor in active_panels if anchor in text]
    if len(matches) != 1:
        raise ValueError('Expected exactly one supported active-bank panel anchor')
    anchor = matches[0]
    text=once(text,anchor,'{first_panel("active-bank",6,ay,228,ah);'+anchor+'}')
    text=once(text,'rbox(3, py, 234, ph, UI_PANEL_RADIUS, SOUND_FACE);','first_panel("voice-body",3,py,234,ph);rbox(3, py, 234, ph, UI_PANEL_RADIUS, SOUND_FACE);')
    if 'dv_voice_steps(' in text:
        step='cv_rect(sx,step_y,11,4,live?C_WHITE:hit?col:SOUND_LINE);'
        text=once(text,step,'{uint16_t before[CV_MAX];first_snapshot(before);'+step+'first_finish(before,"step",sx,step_y,0);}')
    else:
        text=once(text,'cv_rect(sx, sy, 11, 4, sc);','{uint16_t before[CV_MAX];first_snapshot(before);cv_rect(sx, sy, 11, 4, sc);first_finish(before,"step",sx,sy,0);}')
    if 'dv_cp_stage_page(' in text:
        text=once(text,'rbox(3,24-off,234,213,UI_PANEL_RADIUS,SOUND_FACE);','first_panel("owner-stage-banks",3,24-off,234,213);rbox(3,24-off,234,213,UI_PANEL_RADIUS,SOUND_FACE);')
    sig='static '+draw+'void dv_tune_fork(float ox, float oy, float sx, float sy, float tune01, uint16_t col)'
    text=once(text,sig+'\n{',sig+'\n{\n    first_glyph_type="tuning-fork";')
    # Route marker belongs to actual Pitch branch if present, not inferred from name.
    pitch='if (r.kind == R_ENG && r.a == T_909 && r.b == DR_BD && r.c == 5) {'
    if pitch not in text:pitch='if (d && name_is(d,"Pitch")) {'
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
    # Full-width native envelope curves replaced many body pictograms. Observe
    # their actual raster writes as body content, never panel fills or fake ink.
    sig='static '+draw+'void dv_curve_row(drum_curve_t *v,int kick,int top,int bottom,uint16_t col)'
    if text.count(sig) != 1:
        raise ValueError('Expected exactly one native drum curve renderer')
    wrapper=sig+"""
{
    uint16_t before[CV_MAX];first_snapshot(before);
    first_original_dv_curve_row(v,kick,top,bottom,col);
    first_finish(before,"envelope-body",13,top,0);
}
"""
    text=wrap(text,sig,'dv_curve_row',wrapper)
    text=once(text,'    cv_poly_aa(points,used/2,col);',
        '    uint16_t curve_before[CV_MAX];first_snapshot(curve_before);\n'
        '    cv_poly_aa(points,used/2,col);\n'
        '    first_finish(curve_before,"envelope-curve",13,top,0);')
    # New owner-bound rows: record only the real polyline raster, separately
    # from physical captions/axes and the genuine live level tick.
    if 'dv_bound_stage(' in text:
        sig='static '+draw+'void dv_bound_stage(drum_curve_t *v,int stage,int top,int off,uint16_t col)'
        if text.count(sig)!=1:raise ValueError('Expected exactly one owner-bound stage renderer')
        text=once(text,'    cv_poly_aa(points,used,col);',
            '    uint16_t stage_before[CV_MAX];first_snapshot(stage_before);\n'
            '    cv_poly_aa(points,used,col);\n'
            '    first_finish(stage_before,"envelope-curve",13,top-off,0);')
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
