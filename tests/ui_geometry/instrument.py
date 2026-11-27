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
     geo_label *e=&geo_tile[geo_ntile++];e->f=f;e->x=geo_x;e->y=geo_y-geo_row_base();e->col=c;strcpy(e->text,geo_text);e->value=geo_value_fn&&!(geo_skip_a&&!strcmp(geo_text,geo_skip_a))&&!(geo_skip_b&&!strcmp(geo_text,geo_skip_b))?geo_value_fn:0;e->ref=e->value?geo_value_ref:0;
    }
'''+a);p.write_text(s)
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    s=once(s,'static void cv_lcd_full(uint32_t x, uint32_t y)\n{','static void cv_lcd_full(uint32_t x, uint32_t y)\n{\n    geo_blit(x,y);')
    s=once(s,'    cv_lcd_pos_t pos = {x, y};','    geo_blit(x,y);\n    cv_lcd_pos_t pos = {x, y};')
    if 'static void cv_commit_main_band(' in s:
        v='    cv_lcd_pos_t pos = {0, MAIN_Y + band * BAND_H};'
        s=once(s,v,v+'\n    geo_blit((int)pos.x,(int)pos.y);')
        v='        paint(2,scene);'
        s=once(s,v,v+'\n        geo_blit((int)pos.x,(int)pos.y);')
    # Current parameter names are rendered once in their semantic face ink.
    p.write_text(s)

    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    scene='static UI_DRAW void dv_glyph_scene(' in s
    glyph='dv_glyph_scene' if scene else 'dv_glyph'
    glyph_sig='static UI_DRAW void '+glyph+'(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col'+(' DV_ARG' if scene else '')+')'
    value_args=' DV_FORWARD' if scene else ''
    a=glyph_sig+'\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_GLYPH"))return;\n    if(geo_enabled){if(presence_nglyph>=256)exit(2);unsigned *q=presence_glyph[presence_nglyph++];q[0]=r.kind;q[1]=r.a;q[2]=r.b;q[3]=r.c;}\n')
    a='static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large'+(' DV_ARG' if scene else '')+')\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_VALUE"))return;');p.write_text(s)

    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='        cv_text(8, vc(sel ? &FONT_B : &FONT_S, y + 1 - inset, 18), sel ? &FONT_B : &FONT_S, row_name, sel ? C_WHITE : C_GRAY);'
    s=once(s,a,'        if(!getenv("PRESENCE_MUTATE_LIST"))'+a.strip())
    a='static UI_DRAW void draw_param_widget(pref_t r, int x, int y, int width, int active, uint16_t col, int height, int night)\n{'
    s=once(s,a,a+'\n    if(getenv("PRESENCE_MUTATE_SYNTH"))return;');p.write_text(s)
    a='        text_c(31+e.slot[k]*59,y+8-off,&FONT_T,label,edited==k?hi:SOUND_GRAY);'
    # caption "A 27%" (stage letter, value; no knob number since FINAL-REFERENCES 23 Nov)
    s=once(s,a,'''        if(getenv("PRESENCE_MUTATE_ENV_KNOB"))label[1]='0';
        if(getenv("PRESENCE_MUTATE_ENV_STAGE"))label[0]='X';
        if(getenv("PRESENCE_MUTATE_ENV_PERCENT"))label[2]='X';
        if(!getenv("PRESENCE_MUTATE_ENV_MISSING"))
'''+a);p.write_text(s)


    p=root/'firmware/src/app/ui.c';s=p.read_text()
    a='        text_c(cx, vc(&FONT_XS, mix_kit >= 0 && i == 0 ? 66 : 13, 10), &FONT_XS, caps_of(pref_name(r)), touched ? C_WHITE : C_GRAY);'
    s=once(s,a,'        if(getenv("DUPLABEL_MUTATE"))text_c(cx+30, vc(&FONT_XS, mix_kit >= 0 && i == 0 ? 66 : 13, 10), &FONT_XS, caps_of(pref_name(r)), C_GRAY);\n        if(!getenv("PRESENCE_MUTATE_STRIP"))'+a.strip());p.write_text(s)
    a='            text_r(pickup?215:200, 3, &FONT_T, b, seq.ext ? C_AMB : C_HI);'
    s=once(s,a,'            if(!getenv("HEADER_MUTATE"))'+a.strip());p.write_text(s)

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
      for(int yy=cv_y0;yy<cv_y1;yy++)for(unsigned xx=0;xx<cv_w;xx++)if(stage_before[(yy-geo_row_base())*cv_w+xx]!=cv_px[(yy-geo_row_base())*cv_w+xx]){
        pixel_tile_owner[(yy-geo_row_base())*cv_w+xx]=stage==1||(stage==0&&(int)xx<=split)?ao:dd;
        pixel_tile_color[(yy-geo_row_base())*cv_w+xx]=cv_px[(yy-geo_row_base())*cv_w+xx];
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
    sig=glyph_sig
    wrapper=sig+'\n{\n    uint16_t before[CV_MAX];pixel_snapshot(before);\n    first_original_dv_glyph(r,ev,ox+(getenv("PIXEL_MUTATE_SHIFT")?80:0),oy,sx,sy,col);\n    uint32_t owner=1u+((uint32_t)r.kind<<24)+((uint32_t)r.a<<16)+((uint32_t)r.b<<8)+r.c;\n    if(geo_enabled&&getenv("PIXEL_MUTATE_BLANK"))memcpy(cv_px,before,cv_w*cv_h*sizeof *before);\n    pixel_finish(before,owner);\n    if(geo_enabled&&getenv("PIXEL_MUTATE_OVERPAINT"))memcpy(cv_px,before,cv_w*cv_h*sizeof *before);\n}\n'
    wrapper=wrapper.replace('first_original_dv_glyph(', 'first_original_'+glyph+'(').replace('oy,sx,sy,col);','oy,sx,sy,col'+value_args+');')
    s=module.wrap(s,sig,glyph,wrapper);p.write_text(s)

    # L11 pan: every pan the walk formats (is_pan or any control named Pan)
    # must read L50..L1, C, R1..R50 with no unit. PAN_MUTATE emits raw numbers.
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    sig='X0X_COLD static void pref_value_of(pref_t r, int v, char *num, char *unit)'
    wrapper=sig+"""
{
    first_original_pref_value_of(r,v,num,unit);
    geo_last_ref=1u+(((unsigned)r.kind<<24)|((unsigned)r.a<<16)|((unsigned)r.b<<8)|r.c);
    if(!geo_enabled)return;
    {   /* never a raw 0..127 pot readout: a unitless number equal to the stored value */
        const x0x_param_t *dd=pref_desc(r);
        if(getenv("RAW_MUTATE") && r.kind==R_ENG && r.a==T_909 && unit[0] && !is_pan(r)){put_i(num,v);unit[0]=0;}
        if(getenv("UNITNAME_MUTATE") && !strcmp(unit,"ms"))strcpy(unit,"msec");
        if(dd && !dd->names && dd->max==127 && num[0] && raw_nseen<512) {
            const char *q=num;int digits=*q!=0&&!unit[0];for(;*q;q++)if(*q<'0'||*q>'9')digits=0;
            raw_fmt *w=&raw_seen[raw_nseen++];snprintf(w->num,sizeof w->num,"%s",num);snprintf(w->name,sizeof w->name,"%s",dd->name);
            w->ref=((unsigned)r.kind<<24)|((unsigned)r.a<<16)|((unsigned)r.b<<8)|r.c;
            w->raw=digits && atoi(num)==v;
        }
    }
    if(unit[0] && num[0] && unit_nseen<512 && strlen(num)<16 && strlen(unit)<8) {
        unit_fmt *u=&unit_seen[unit_nseen++];strcpy(u->num,num);strcpy(u->unit,unit);
        u->ref=((unsigned)r.kind<<24)|((unsigned)r.a<<16)|((unsigned)r.b<<8)|r.c;
        if(getenv("UNIT_MUTATE"))unit[0]=0;   /* a formatter that loses its unit */
    }
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
    # Fast S1 cached formatting does not call pref_value_of while painting.
    # Tag its actual typed ref too, or the prior capture's last cell steals
    # ownership of every subsequent cached readout on the audited frame.
    fast='static void matrix_format(pref_t r,int raw,char *num,char *unit)\n{'
    if fast in s:
        s=once(s,fast,fast+'\n    geo_last_ref=1u+(((unsigned)r.kind<<24)|((unsigned)r.a<<16)|((unsigned)r.b<<8)|r.c);')
    p.write_text(s)

    # #62 type scale: tag the text a value formatter draws (not its unit or caption).
    # TYPESCALE_MUTATE draws the primary knob value (cell_value) in FONT_T, a compact face.
    for path,sig,call,fn,skips in (
        ('firmware/src/app/ui.c','static void cell_value(int cx, int y, const char *num, const char *unit, uint16_t c)','cx,y,num,unit,c',1,'unit,0'),
        ('firmware/src/app/ui.c','static UI_DRAW void strip_value(int cx, int y, char *num, const char *unit, uint16_t ink, const char *caption)','cx,y,num,unit,ink,caption',2,'unit,caption'),
        ('firmware/src/app/ui.c','static UI_DRAW void strip_value_fit(int x, int y, char *num, const char *unit, uint16_t ink)','x,y,num,unit,ink',3,'unit,0'),
        ('firmware/src/app/ui_drum_voice.inc','static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large'+(' DV_ARG' if scene else '')+')','r,ev_i,cx,y,col,large'+value_args,4,'0,0')):
        p=root/path;s=p.read_text();name=sig.split('(')[0].split()[-1]
        a,b=skips.split(',')
        ref='1u+(((unsigned)r.kind<<24)|((unsigned)r.a<<16)|((unsigned)r.b<<8)|r.c)' if fn==4 else 'geo_last_ref'
        # DUPREAD_MUTATE draws every primary knob readout a second time 20 px lower.
        twice='    if(getenv("DUPREAD_MUTATE"))first_original_%s(cx,y+20,num,unit,c);\n'%name if fn==1 else ''
        w=sig+'\n{\n    int was=geo_value_fn;unsigned wr=geo_value_ref;const char *sa=geo_skip_a,*sb=geo_skip_b;geo_value_fn=%d;geo_value_ref=%s;geo_skip_a=%s;geo_skip_b=%s;\n    first_original_%s(%s);\n%s    geo_value_fn=was;geo_value_ref=wr;geo_skip_a=sa;geo_skip_b=sb;\n}\n'%(fn,ref,a,b,name,call,twice)
        s=module.wrap(s,sig,name,w)
        if fn==1:
            s=once(s,'    x = cv_text(x, y, &FONT_B, num, c);','    x = cv_text(x, y, getenv("TYPESCALE_MUTATE")?&FONT_T:&FONT_B, num, c);')
        p.write_text(s)
    # Calm-page text mutants: a knob-number prefix, a listed abbreviation, a repeated label.
    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    s=once(s,'put_s(caption,"DECAY ")','put_s(caption,getenv("KPREFIX_MUTATE")?"K5 DECAY ":"DECAY ")');p.write_text(s)
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    # KNOBGROUP_MUTATE drops SuperJ6's last group (LFO/CHORUS): four controls unreachable.
    s=once(s,'                pref_t refs[4];\n                for (int col = 0; col < 4; col++) refs[col] = rows[row][col] < 0','                pref_t refs[4];\n                if(getenv("KNOBGROUP_MUTATE") && kind == ORIGINAL_SOUND_JUNO && row == count - 1) continue;\n                for (int col = 0; col < 4; col++) refs[col] = rows[row][col] < 0')
    # DEADSPACE_MUTATE leaves the 808/909 MIX strip panel undrawn.
    s=once(s,'static UI_DRAW void draw_drum_mix(int band, int k)\n{','static UI_DRAW void draw_drum_mix(int band, int k)\n{\n    if(getenv("DEADSPACE_MUTATE"))return;')
    s=once(s,'    if (r.kind >= R_DLEN && r.kind <= R_BTRANS)','    if(getenv("ABBREV_MUTATE")&&ui.view==V_SEQ&&r.kind==R_DLEN+5)return "TRANSP";\n    if (r.kind >= R_DLEN && r.kind <= R_BTRANS)')
    p.write_text(s)
    p=root/'firmware/src/app/ui_drum_voice.inc';s=p.read_text()
    a='static void dv_header_battery(int x, int y)\n{'
    s=once(s,a,a+'\n    if(getenv("BATTERY_MUTATE"))return;');p.write_text(s)
    # Row 7: the synth sequencer (303A/303B, every engine) stays on the R22
    # piano roll. R22_MUTATE drops the roll from SEQ, as an R23 step page would.
    p=root/'firmware/src/app/ui.c';s=p.read_text()
    sig='static UI_DRAW void draw_303(int band, int gen)'
    wrapper=sig+"""
{
    if(geo_enabled && !gen && ui.view==V_SEQ) {
        if(getenv("R22_MUTATE"))return;
        r22_roll_bands++;
    }
    first_original_draw_303(band,gen);
}
"""
    if 'static UI_DRAW void draw_303_scene(int band,const cv_303_scene *z)' in s:
        dsig='static UI_DRAW void draw_303_scene(int band,const cv_303_scene *z)'
        w=wrapper.replace(sig,dsig).replace('!gen','!z->gen').replace('first_original_draw_303(band,gen);','first_original_draw_303_scene(band,z);')
        s=module.wrap(s,dsig,'draw_303_scene',w)
    s=module.wrap(s,sig,'draw_303',wrapper);p.write_text(s)
