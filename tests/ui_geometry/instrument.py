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
    # Current parameter names are rendered once in their semantic face ink.
    p.write_text(s)

    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    a='static UI_DRAW void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_GLYPH"))return;\n    if(geo_enabled){if(presence_nglyph>=256)exit(2);unsigned *q=presence_glyph[presence_nglyph++];q[0]=r.kind;q[1]=r.a;q[2]=r.b;q[3]=r.c;}\n')
    a='static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_VALUE"))return;');p.write_text(s)

    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='        cv_text(8, vc(sel ? &FONT_B : &FONT_S, y + 1 - inset, 18), sel ? &FONT_B : &FONT_S, caps_of(pref_name(r)), sel ? C_WHITE : C_GRAY);'
    s=once(s,a,'        if(!getenv("PRESENCE_MUTATE_LIST"))'+a.strip())
    a='static UI_DRAW void draw_param_widget(pref_t r, int x, int y, int width, int active, uint16_t col, int height, int night)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_SYNTH"))return;');p.write_text(s)
    a='        text_c(31+e.slot[k]*59,y+8-off,&FONT_T,label,edited==k?hi:SOUND_GRAY);'
    s=once(s,a,'''        if(getenv("PRESENCE_MUTATE_ENV_KNOB"))label[1]='0';
        if(getenv("PRESENCE_MUTATE_ENV_STAGE"))label[3]='X';
        if(getenv("PRESENCE_MUTATE_ENV_PERCENT"))label[5]='X';
        if(!getenv("PRESENCE_MUTATE_ENV_MISSING"))
'''+a);p.write_text(s)


    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='        text_c(cx, vc(&FONT_XS, mix_kit >= 0 && i == 0 ? 66 : 13, 10), &FONT_XS, caps_of(pref_name(r)), touched ? C_WHITE : C_GRAY);'
    s=once(s,a,'        if(!getenv("PRESENCE_MUTATE_STRIP"))'+a.strip());p.write_text(s)

    # Full-row drum envelopes are audited as real curve pixels, never exempted
    # from the glyph contract. Snapshot only the curve: baseline/text cannot
    # satisfy a missing-curve negative.
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='    for(int x=1;x<=span;x++) {' if '    for(int x=1;x<=span;x++) {' in s else '    for(int x=1;x<=214;x++) {'
    s=once(s,a,'''    uint16_t curve_before[CV_MAX];pixel_snapshot(curve_before);
    if(getenv("PIXEL_MUTATE_SHIFT")){x0+=80;prevx+=80;}
    if(!getenv("PRESENCE_MUTATE_GLYPH")&&!getenv("ENVELOPE_MUTATE_CURVE"))
'''+a)
    compact='    if(level_time<0)return;'
    a=compact if compact in s else '    char num[16],unit[8],label[32];pref_value_of(r,matrix_value(r),num,unit);'
    s=once(s,a,'''    if(getenv("PIXEL_MUTATE_SHIFT"))x0-=80;
    uint32_t curve_owner=1u+((uint32_t)r.kind<<24)+((uint32_t)r.a<<16)+((uint32_t)r.b<<8)+r.c;
    if(geo_enabled&&(getenv("PIXEL_MUTATE_BLANK")||getenv("ENVELOPE_MUTATE_BLANK")))memcpy(cv_px,curve_before,cv_w*cv_h*sizeof *curve_before);
    pixel_finish(curve_before,curve_owner);
    if(geo_enabled&&getenv("PIXEL_MUTATE_OVERPAINT"))memcpy(cv_px,curve_before,cv_w*cv_h*sizeof *curve_before);
    if(geo_enabled&&!getenv("PRESENCE_MUTATE_GLYPH")&&!getenv("ENVELOPE_MUTATE_CURVE")){if(presence_nglyph>=256)exit(2);unsigned *q=presence_glyph[presence_nglyph++];q[0]=r.kind;q[1]=r.a;q[2]=r.b;q[3]=r.c;}
'''+a);p.write_text(s)

    # Actual bound-stage polylines have parameter ownership, not fake small
    # cells. Combined MA assigns each real phase's pixels to its bound control.
    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    if 'dv_bound_stage(' in s:
        a='    cv_poly_aa(points,used,col);'
        s=once(s,a,r'''
    uint16_t stage_before[CV_MAX];pixel_snapshot(stage_before);
    if(getenv("PIXEL_MUTATE_SHIFT"))for(int z=0;z<used;z++)points[2*z]+=80;
    if(!getenv("PRESENCE_MUTATE_GLYPH")&&!getenv("ENVELOPE_MUTATE_CURVE"))cv_poly_aa(points,used,col);
    if(geo_enabled&&(getenv("PIXEL_MUTATE_BLANK")||getenv("ENVELOPE_MUTATE_BLANK")))memcpy(cv_px,stage_before,cv_w*cv_h*sizeof *stage_before);
    if(geo_enabled&&!getenv("PRESENCE_MUTATE_GLYPH")&&!getenv("ENVELOPE_MUTATE_CURVE")){
      pref_t a=PR(R_ENG,T_808,D8_CP,3),d=PR(R_ENG,T_808,D8_CP,2);
      uint32_t ao=1u+((uint32_t)a.kind<<24)+((uint32_t)a.a<<16)+((uint32_t)a.b<<8)+a.c;
      uint32_t dd=1u+((uint32_t)d.kind<<24)+((uint32_t)d.a<<16)+((uint32_t)d.b<<8)+d.c;
      int split=14+(int)(214.f*v->rise_end/window);
      for(int yy=cv_y0;yy<cv_y1;yy++)for(unsigned xx=0;xx<cv_w;xx++)if(stage_before[yy*cv_w+xx]!=cv_px[yy*cv_w+xx]){
        pixel_tile_owner[yy*cv_w+xx]=stage==1||(stage==0&&(int)xx<=split)?ao:dd;
        pixel_tile_color[yy*cv_w+xx]=cv_px[yy*cv_w+xx];
      }
      for(int k=0;k<2;k++)if(stage==0 || stage==k+1){
        pref_t r=k?d:a;if(presence_nglyph>=256)exit(2);unsigned *q=presence_glyph[presence_nglyph++];q[0]=r.kind;q[1]=r.a;q[2]=r.b;q[3]=r.c;
      }
    }
    if(geo_enabled&&getenv("PIXEL_MUTATE_OVERPAINT"))memcpy(cv_px,stage_before,cv_w*cv_h*sizeof *stage_before);
''')
        p.write_text(s)

    helper=root/'tests/ui_defects/first_slice/instrument.py'
    spec=importlib.util.spec_from_file_location('first_slice_wrappers',helper);module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    sig='static UI_DRAW void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)'
    wrapper=sig+'\n{\n    uint16_t before[CV_MAX];pixel_snapshot(before);\n    first_original_dv_glyph(r,ev,ox+(getenv("PIXEL_MUTATE_SHIFT")?80:0),oy,sx,sy,col);\n    uint32_t owner=1u+((uint32_t)r.kind<<24)+((uint32_t)r.a<<16)+((uint32_t)r.b<<8)+r.c;\n    if(geo_enabled&&getenv("PIXEL_MUTATE_BLANK"))memcpy(cv_px,before,cv_w*cv_h*sizeof *before);\n    pixel_finish(before,owner);\n    if(geo_enabled&&getenv("PIXEL_MUTATE_OVERPAINT"))memcpy(cv_px,before,cv_w*cv_h*sizeof *before);\n}\n'
    s=module.wrap(s,sig,'dv_glyph',wrapper);p.write_text(s)

    # L11 pan: every pan the walk formats (is_pan or any control named Pan)
    # must read L50..L1, C, R1..R50 with no unit. PAN_MUTATE emits raw numbers.
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    sig='X0X_COLD static void pref_value_of(pref_t r, int v, char *num, char *unit)'
    wrapper=sig+"""
{
    first_original_pref_value_of(r,v,num,unit);
    if(!geo_enabled)return;
    const x0x_param_t *d=r.kind==R_ENG?pref_desc(r):0;
    int named=d&&(d->name[0]=='P'||d->name[0]=='p')&&(d->name[1]=='a'||d->name[1]=='A')&&(d->name[2]=='n'||d->name[2]=='N')&&!d->name[3];
    if(!named&&!is_pan(r))return;
    if(getenv("PAN_MUTATE")){put_i(num,v);unit[0]=0;}
    char want[8];
    if(v==64)strcpy(want,"C");
    else snprintf(want,sizeof want,"%c%d",v<64?'L':'R',v<64?((64-v)*50+32)/64:((v-64)*50+31)/63);
    pan_format_checks++;
    if(strcmp(num,want)||unit[0]||v<0||v>127){pan_format_bad++;printf("PAN_FORMAT_BAD ref=%u:%u:%u:%u v=%d got=[%s%s] want=[%s]\\n",r.kind,r.a,r.b,r.c,v,num,unit,want);}
}
"""
    s=module.wrap(s,sig,'pref_value_of',wrapper)
    p.write_text(s)
    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    a='static void dv_header_battery(int x, int y)\n{'
    s=once(s,a,a+'\n    if(getenv("BATTERY_MUTATE"))return;');p.write_text(s)
