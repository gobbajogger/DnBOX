/* SPDX-License-Identifier: GPL-3.0-only */
/* R22 common screen composer. Real typed refs/state only. Four parameter
 * tracks; a control may span tracks. Existing physical/page actions unchanged. */
static int r22_plain_page(void)
{
    if(ui.overlay!=O_NONE||ui.help)return 0;
#if BOARD_SMK37
    extern uint8_t smk_clip_mode_on(void);
    if(smk_clip_mode_on())return 0;
#endif
    return 1;
}
static int r22_control_value(pref_t r)
{
    return matrix_value(r);
}
static void r22_control_unit(pref_t r,int x,int y,int w,int h,int active,uint16_t accent)
{
    const x0x_param_t *d=pref_desc(r);if(!d)return;
    int cx=x+w/2,val=r22_control_value(r),visual=(int)(ui_ease_float(r,val)+.5f);
    uint16_t ink=active?accent:SOUND_WHITE;
    char label[32],num[24],unit[12],value[40];
    put_s(label,caps_of(pref_name(r)));
    cv_text_clip(cx-tw(&FONT_T,label)/2,y+2,&FONT_T,label,SOUND_INK,x+2,x+w-2);
    int saved_oy=cv_oy,rendered=0,mix=0,shape=0;
    if(r.kind==R_ENG&&r.a==T_303){
        shape=wave_shape(r,visual,&mix);
        if(shape){
            int slot=original_wave_slot((unsigned)engine_bass_sound_bank(&proj.sound,r.b),r.c);
            const ui_original_wave_cache *wave=slot>=0?original_wave_view(r,visual):0;
            if(wave)draw_original_wave(cx,y+18,wave,slot,ink,active?SOUND_SELECT:SOUND_FACE);
            else draw_wave_glyph(cx,y+18,shape,mix,ink,active?SOUND_SELECT:SOUND_FACE);
            rendered=1;
        }else if(orig_strip_slot(r)){orig_strip_glyph(r,visual,cx-13,y+18,ink);rendered=1;}
    }
    if(!rendered&&drum_strip_role(r)){drum_strip_glyph(r,visual,cx-13,y+18,ink);rendered=1;}
    if(!rendered){
        cv_oy=saved_oy+y-10; /* Existing approved glyph coordinates28..48. */
        rendered=perform_glyph(r,visual,cx,ink)||control_glyph(r,visual,cx,ink);
        cv_oy=saved_oy;
    }
    if(!rendered&&d->names&&d->max<12){
        int n=d->max+1,pw=n>6?3:5,span=n*(pw+2)-2;
        for(int i=0;i<n;i++)rbox(cx-span/2+i*(pw+2),y+24,pw,7,R22_RADIUS,i==val?ink:SOUND_LINE);
    }
    pref_value_of(r,val,num,unit);
    /* An explicitly unitless engine amount may be displayed as its normalized
     * amount, never invented Hz/ms. Typed integer/enum scheduling stays intact. */
    if(r.kind==R_ENG&&!is_pan(r)&&!d->names&&!unit[0]&&d->max==127){put_i(num,(val*100+63)/127);put_s(unit,"%");}
    put_s(put_s(value,num),unit);
    const felucca_font_t *font=&FONT_S;
    if(tw(font,value)>w-4)font=&FONT_T; /* Recorded as a typography limit, not silently accepted. */
    cv_text_clip(cx-tw(font,value)/2,y+h-18,font,value,SOUND_WHITE,x+1,x+w-1);
    int lv=lock_of(r,lock_step());
    if(lv>=0||(lock_step()<0&&has_motion(r)))dot(x+w-5,y+7,1,lv>=0?SOUND_AMBER:SOUND_GRAY);
}
static void r22_bank(int y,int h,const char *title,const pref_t refs[4],int active,int off)
{
    int py=y-off,count=0;char tag[32];
    for(int k=0;k<4;k++)if(refs[k].kind!=R_NONE)count++;
    if(!count)return;
    surface_panel(3,py,234,h,active,0,0,0);
    cv_text(11,py+3,&FONT_T,title,SOUND_GRAY);
    put_s(put_i(put_s(put_i(tag,cur_page()+1),"/"),pg.n),"");text_r(230,py+3,&FONT_T,tag,SOUND_GRAY);
    int cell=0;
    for(int k=0;k<4;k++)if(refs[k].kind!=R_NONE){
        int x=6+228*cell/count,w=228*(cell+1)/count-228*cell/count;
        r22_control_unit(refs[k],x,py+15,w,h-17,active,part_col());cell++;
    }
}
static void r22_held_step_row(int off)
{
    static const char *const names[]={"NOTE","GATE","ACCENT","SLIDE"};
    static const char *const gates[]={"REST","NOTE","TIE"};
    const bpart_t *bp=&cur_pat()->bass[bidx()];
    const bstep_t *st=&bp->step[ui.held_step];
    int y=142-off;char title[40];knob_tag(title);
    surface_panel(3,y,234,76,ui.touched>=0,0,0,0);
    cv_text(11,y+3,&FONT_T,title,SOUND_GRAY);
    for(int k=0;k<4;k++){
        int cx=34+k*57,gy=y+35;char value[16];uint16_t ink=ui.touched==k?part_col():SOUND_WHITE;
        text_c(cx,y+17,&FONT_T,names[k],SOUND_INK);
        if(k==0){
            int note=held_note_of(bp,ui.held_step)+bp->transpose-24;
            put_note(value,note);
            for(int j=0;j<3;j++)cv_rect(cx-16,gy-5+j*5,33,1,SOUND_LINE);
            int py=gy+6-(note%12)*12/11;
            cv_stroke_aa(cx-3,py,cx+3,py,3.f,ink);cv_stroke_aa(cx+4,py,cx+4,py-10,2.f,ink);
        }else if(k==1){
            int gate=bstep_gate(st);put_s(value,gates[gate<3?gate:0]);
            for(int j=0;j<3;j++)rbox(cx-17+j*13,gy-3,9,9,R22_RADIUS,j==gate?ink:SOUND_LINE);
        }else{
            int on=(st->flags&(k==2?BS_ACCENT:BS_SLIDE))!=0;put_s(value,on?"ON":"OFF");
            if(k==2){for(int j=0;j<4;j++)cv_stroke_aa(cx-15+j*10,gy+6,cx-15+j*10,gy+6-(on?4+j*3:3),2.f,ink);}
            else {cv_stroke_aa(cx-17,gy+5,cx-5,gy+5,2.f,ink);cv_stroke_aa(cx+5,gy-5,cx+17,gy-5,2.f,ink);if(on)cv_stroke_aa(cx-5,gy+5,cx+5,gy-5,2.f,ink);}
        }
        text_c(cx,y+56,&FONT_S,value,SOUND_WHITE);
    }
}
static void r22_knob_row(int off)
{
    if(ui.view==V_SEQ&&is_303()&&ui.held_step>=0){r22_held_step_row(off);return;}
    int p=cur_page();char title[40];knob_tag(title);
    r22_bank(142,76,title,pg.r[p],ui.touched>=0,off);
}
static void r22_home(int band)
{
    int off=band*BAND_H;
    cv_rect(0,0,240,band==2?KNOB_H:BAND_H,SOUND_BASE);
    int base=ui.pbank*16;
    for(int i=0;i<16;i++){
        int pat=base+i,x=3+(i%4)*59,y=4+(i/4)*15-off,selected=seq.ppat[ui.part]==pat;
        surface_panel(x,y,57,14,selected,0,0,0);char name[8];put_i(put_s(name,"P"),pat+1);
        cv_text(x+4,y+1,&FONT_T,name,selected?SOUND_INK:SOUND_GRAY);
        for(int pos=0;pos<NPARTS;pos++){
            int p=part_at(pos);if(part_used(&proj.pat[pat],p))cv_rect(x+30+pos*4,y+5,3,3,PART_COL[p]);
        }
        int cue=seq_cue_of(&seq,ui.part);if(cue==pat&&cue!=seq.ppat[ui.part]&&(ui.frame&16u))rframe(x,y,57,14,R22_RADIUS,SOUND_AMBER);
    }
    surface_panel(3,66-off,234,71,0,0,0,0);
    for(int pos=0;pos<NPARTS;pos++){
        int p=part_at(pos),y=68+pos*13-off,len=16,step=0;uint32_t bits=0;
        if(p<NKIT){const dpart_t *d=&pat_of(p)->drum[p];int base=seq.playing?(eng_step[TRK_DRUM+p]&~15):0;len=d->len;step=eng_step[TRK_DRUM+p]-base;for(int v=0;v<NDRUM;v++)bits|=d->hit[v][base>>5]>>(base&31);}
        else if(p==PART_BRK){bits=pat_of(p)->brk.steps;step=eng_step[TRK_BRK];}
        else {const bpart_t *bp=&pat_of(p)->bass[p-NKIT];int base=seq.playing?(eng_step[TRK_BASS0+p-NKIT]&~15):0;bits=bass_content_bits16(bp,base);len=bp->len;step=eng_step[TRK_BASS0+p-NKIT]-base;}
        cv_text_clip(11,y,&FONT_T,part_display(p),PART_COL[p],11,78);
        for(int s=0;s<16;s++){int x=82+s*9;uint16_t ink=s<len?((bits>>s)&1u?PART_COL[p]:SOUND_LINE):SOUND_BASE;rbox(x,y+2,7,7,R22_RADIUS,ink);if(seq.playing&&s==step)rframe(x,y+2,7,7,R22_RADIUS,SOUND_WHITE);}
    }
    r22_knob_row(off);
}
static void r22_fx(int band)
{
    int off=band*BAND_H;
    cv_rect(0,0,240,band==2?KNOB_H:BAND_H,SOUND_BASE);
    int p=cur_page();
    r22_bank(4,82,pg.title[p],pg.r[p],1,off);
    surface_panel(3,90-off,234,128,0,0,0,0);
    cv_text(11,94-off,&FONT_T,"SEND",SOUND_GRAY);
    text_c(112,94-off,&FONT_T,"REVERB",SOUND_GRAY);
    text_c(192,94-off,&FONT_T,"DELAY",SOUND_GRAY);
    /* Native240px: one four-control effect bank and allfive send rows.
     * Desktop may display bothbanks; page navigation retains everyreal ref. */
    for(int pos=0;pos<NPARTS;pos++){
        int part=part_at(pos),y=110+pos*20-off;
        cv_text_clip(11,y+2,&FONT_T,part_display(part),PART_COL[part],11,77);
        for(int send=0;send<2;send++){
            int x=80+send*80,raw=proj.sound.v[T_MIX][part][send?MX_DLY:MX_REV];
            night_send(x,y+7,42,raw,PART_COL[part]);char n[12];put_s(put_i(n,(raw*100+63)/127),"%");
            text_r(x+71,y+1,&FONT_S,n,SOUND_WHITE);
        }
    }
}
static void r22_mix(int band)
{
    int off=band*BAND_H,dm=drum_mix_page(),master=master_mix_page(),wave=wave_mix_page();
    cv_rect(0,0,240,band==2?KNOB_H:BAND_H,SOUND_BASE);
    if(master||wave){
        r22_bank(4,76,wave?"MASTER / WAVE":"MASTER",pg.r[cur_page()],ui.touched>=0,off);
        surface_panel(3,84-off,234,134,0,0,0,0);
        cv_text(11,88-off,&FONT_T,wave?"POST-MASTER AUDIO":"PEAK HISTORY / GAIN REDUCTION",SOUND_GRAY);
        int saved=cv_oy;cv_oy=saved+103-off;
        if(wave)draw_master_wave();else draw_mix(0);
        cv_oy=saved;
        char val[24];put_s(put_i(put_s(val,"GR "),(int)(engine_gr_db()+.5f)),"dB");cv_text(11,184-off,&FONT_S,val,SOUND_WHITE);
        put_s(put_i(put_s(val,"AUDIO "),plat_cpu_pct()),"%");cv_text(128,184-off,&FONT_S,val,SOUND_WHITE);
        return;
    }
    if(dm>=0){
        int t=dm?T_808:T_909;
        for(int v=0;v<NDRUM;v++){
            int x=3+(v%4)*59,y=4+(v/4)*45-off;pref_t lv=find_ref(t,v,"Level"),pn=find_ref(t,v,"Pan");
            int level=lv.kind?pref_get(lv):0,pan=pn.kind?pref_get(pn):64;
            surface_panel(x,y,57,42,v==ui.sel[dm],0,0,0);
            text_c(x+28,y+2,&FONT_T,engine_voice_name(t,v),PART_COL[dm]);
            night_send(x+6,y+17,44,level,PART_COL[dm]);char label[16],unit[8];pref_value_of(pn,pan,label,unit);
            text_c(x+28,y+28,&FONT_T,label,SOUND_WHITE);
        }
    }else{
        surface_panel(3,4-off,234,132,0,0,0,0);
        for(int p=0;p<4;p++){
            int cx=34+p*57,level=proj.sound.v[T_MIX][p][MX_LEVEL],pk=eng_peak[p]*53/32768;
            text_c(cx,7-off,&FONT_T,part_s(p),PART_COL[p]);
            rbox(cx-2,22-off,4,55,R22_RADIUS,SOUND_LINE);
            if(pk>0)cv_rect(cx-1,77-pk-off,2,pk,PART_COL[p]);
            rbox(cx-10,74-level*50/127-off,20,5,R22_RADIUS,SOUND_WHITE);
            char n[16],u[8],value[24];pref_value(PR(R_ENG,T_MIX,p,MX_LEVEL),n,u);put_s(put_s(value,n),u);text_c(cx,83-off,&FONT_T,value,SOUND_WHITE);
        }
        cv_text(11,106-off,&FONT_T,"BREAK",PART_COL[PART_BRK]);
        night_send(72,106-off,154,proj.sound.v[T_MIX][PART_BRK][MX_LEVEL],PART_COL[PART_BRK]);
        char n[16],u[8],value[24];pref_value(PR(R_ENG,T_MIX,PART_BRK,MX_LEVEL),n,u);put_s(put_s(value,n),u);text_r(226,121-off,&FONT_T,value,SOUND_WHITE);
    }
    r22_knob_row(off);
}
static int r22_main_page(int band)
{
    if(!r22_plain_page())return 0;
    if(ui.view==V_HOME){r22_home(band);return 1;}
    if(ui.view==V_FX){r22_fx(band);return 1;}
    if(ui.view==V_MIX){r22_mix(band);return 1;}
    return 0;
}
