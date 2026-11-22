/* NEW BREAK family layout implementation candidate. Host-rendered experimental geometry; not fully admitted.
 * Coordinates follow the independently reviewed 240px proposal. Uses only
 * engine-owned waveform/state and existing project controls, never fixture ink.
 * Call main per existing69-row band; call footer in existing82-row knob strip.
 */
/* Bound display text by native font metrics, without changing slot identity.
 * Callers provide room for two dots plus terminator. */
static void break_layout_fit_name(char *out, unsigned cap, const char *prefix,
                                  const char *name, int width)
{
    unsigned n=0;
    while(*prefix && n+3<cap) out[n++]=*prefix++;
    while(*name && n+3<cap) out[n++]=*name++;
    out[n]=0;
    if(!*name && tw(&FONT_T,out)<=width)return;
    for(;;) {
        out[n]='.';out[n+1]='.';out[n+2]=0;
        if(tw(&FONT_T,out)<=width)return;
        if(!n)break;
        --n;
    }
    out[0]=0;
}
static void break_layout_main(int band)
{
    const brkpart_t *bp=&cur_pat()->brk;
    const char *const *names=engine_brk_slot_names();
    int ns=engine_brk_nslots(),count=engine_brk_count(0),slice,bank,div,running;
    int off=MAIN_Y+band*BAND_H;
    uint16_t col=part_col();
    char t[40],*q;
    engine_brk_state(&slice,&bank,&div,&running);
    int audition=engine_brk_held()!=0,active=running||audition;
    int choosing=ui.brk_choice_on&&engine_brk_preview_active()&&ui.brk_choice_pat==seq.ppat[PART_BRK];
    if(count<1)count=1;
    int page=ui.brk_page*8<count?ui.brk_page:(count-1)/8;
    if(!ui.outline_ok||ui.outline_slot!=bp->slot_a||
       ui.outline_revision!=engine_brk_outline_revision(0)){
        ui.outline_ok=(uint8_t)engine_brk_outline_snapshot(0,ui.outline,232,&ui.outline_revision);
        ui.outline_slot=bp->slot_a;
    }
    night_panel(3,24-off,234,118,0);
    /* Eight real A-owned slices. Labels have a separate ink band below80. */
    for(int c=0;c<8;c++){
        int local=page*8+c,x=9+c*222/8,w=(c+1)*222/8-c*222/8;
        if(active&&bank==0&&slice==local)cv_rect(x,32-off,w-1,45,dim(col,5));
        if(local<count)put_i(t,local+1);else put_s(t,"-");
        text_c(x+w/2,vc(&FONT_T,83-off,10),&FONT_T,t,active&&bank==0&&slice==local?C_WHITE:C_GRAY);
        if(c)cv_rect(x,32-off,1,45,C_LINE);
    }
    cv_rect(9,54-off,222,1,C_LINE);
    for(int x=0;x<222&&ui.outline_ok;x++){
        int local=page*8+x*8/222;
        int source_x=(page*8*232+x*8*232/222)/count;
        int h=local<count&&source_x<232?ui.outline[source_x]*21/255:0;
        if(h)cv_rect(9+x,54-off-h,1,2*h+1,active&&bank==0&&slice==local?C_WHITE:col);
    }
    for(int b=0;b<2;b++){
        const char *name=ns?names[(b?bp->slot_b:bp->slot_a)%ns]:"none";
        break_layout_fit_name(t,sizeof t,b?"B ":"A ",name,106);
        text_c(b?177:63,vc(&FONT_T,99-off,11),&FONT_T,t,active&&!choosing&&bank==b?C_WHITE:C_HI);
        if(active&&!choosing&&bank==b)cv_rect(b?124:10,111-off,106,1,col);
    }
    if(choosing&&ui.brk_choice_slot<ns){
        break_layout_fit_name(t,sizeof t,ui.brk_choice_bank?"PREVIEW B ":"PREVIEW A ",
                              names[ui.brk_choice_slot],220);
        cv_text(10,vc(&FONT_T,114-off,10),&FONT_T,t,C_WHITE);
        cv_text(10,vc(&FONT_T,126-off,10),&FONT_T,"SEL CHOOSE",C_WHITE);
        text_r(230,vc(&FONT_T,126-off,10),&FONT_T,"HOME CANCEL",C_WHITE);
    }else{
        q=put_i(put_s(t,"A "),page*8+1);q=put_i(put_s(q,"-"),count<8?count:page*8+8);put_i(put_s(q," / "),count);
        cv_text(10,vc(&FONT_T,114-off,10),&FONT_T,t,C_GRAY);
        q=put_s(t,ui.brk_count_target?"EDIT B ":"EDIT A ");put_i(q,(int)brk_slice_count(bp,ui.brk_count_target));
        text_r(230,vc(&FONT_T,114-off,10),&FONT_T,t,C_GRAY);
        t[0]=0;
        if(count!=(int)brk_slice_count(bp,0))put_s(t,"PENDING");
        else if(active){q=put_s(t,audition?"LIVE A ":bank?"PLAY B ":"PLAY A ");put_i(q,slice+1);}
        cv_text(10,vc(&FONT_T,126-off,10),&FONT_T,t,C_HI);
        text_r(230,vc(&FONT_T,126-off,10),&FONT_T,"MORE",C_HI);
    }
    /* Same physical panel crosses the strip boundary; no buffer repartition. */
    night_panel(3,146-off,234,64,0);
    /* Keep Q descenders above the main/knob strip boundary at y158. */
    cv_text(10,vc(&FONT_T,147-off,10),&FONT_T,pg.title[cur_page()],col);
    if(pg.n>1){q=put_i(t,cur_page()+1);put_i(put_s(q," / "),pg.n);text_r(230,vc(&FONT_T,147-off,10),&FONT_T,t,C_GRAY);}
}
static void break_layout_footer(void)
{
    const brkpart_t *bp=&cur_pat()->brk;
    int ph=seq.playing?eng_step[TRK_BRK]:-1,count=engine_brk_count(0);
    uint32_t locks=lock_marks(-1,-1,0);
    if(count<1)count=1;
    for(int i=0;i<16;i++){
        int x=9+i*14,on=(bp->steps>>i)&1u;char t[4];
        put_i(t,i+1);text_c(x+5,vc(&FONT_T,214-KNOB_Y,9),&FONT_T,t,C_GRAY);
        if(ui.view==V_SEQ)continue; /* slider lane owns the lower row on SEQ */
        if(bp->slice[i])put_i(t,(bp->slice[i]-1)%count+1);else put_s(t,"-");
        text_c(x+5,vc(&FONT_T,224-KNOB_Y,9),&FONT_T,t,bp->slice[i]?(on?C_HI:C_DIM):C_DIM);
        cv_rect(x,234-KNOB_Y,11,4,i==ph?C_WHITE:on?part_col():C_LINE);
        if((locks>>i)&1u)cv_rect(x+4,238-KNOB_Y,3,2,C_AMB);
    }
}
