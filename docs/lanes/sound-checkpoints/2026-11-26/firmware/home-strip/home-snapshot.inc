typedef struct {
 size_t bank;
 char status[24];
 char names[NPARTS][24];
 uint8_t part[NPARTS], flags[NPARTS][16], strength[NPARTS][16][8];
 uint16_t color[6], part_color[NPARTS];
} home_snapshot;
static void take_home(home_snapshot *h) {
 int c0=ui.pbank*16;
 int state=save_state();
 h->bank=c0;
 put_s(h->status,state==SAVE_SAVED?"SAVED":header_save_label(state));
 h->color[0]=SOUND_GRAY;h->color[1]=SOUND_FACE;h->color[2]=UI_N_SIDE;
 h->color[3]=C_WHITE;h->color[4]=C_AMB;h->color[5]=C_BLACK;
 for(int pos=0;pos<NPARTS;pos++) {
    int p=part_at(pos),cur=seq.ppat[p],cue=seq_cue_of(&seq,p);
    h->part[pos]=(uint8_t)p;h->part_color[pos]=PART_COL[p];
    const char *name=part_display(p);
    if(tw(&FONT_T,name)>HOME_X0-11)name=part_s(p);
    assert(strlen(name)<sizeof h->names[pos]);put_s(h->names[pos],name);
    for(int c=0;c<16;c++) {
      int pattern=c0+c;
      int on=part_used(&proj.pat[pattern],p);
      unsigned f=(cur==pattern)|((cue==pattern&&cue!=cur&&(ui.frame&16u))?2:0)|(on?4:0);
      if(seq.chain_a!=seq.chain_b&&pattern>=(seq.chain_a<seq.chain_b?seq.chain_a:seq.chain_b)&&
          pattern<=(seq.chain_a<seq.chain_b?seq.chain_b:seq.chain_a)) f|=8;
      h->flags[pos][c]=(uint8_t)f;
      if(on) home_pattern_strength8(&proj.pat[pattern],proj.arr.lane,pattern,p,
          proj.set.accent_q7,T_VEL,h->strength[pos][c]);
    }
 }
}
static void draw_home_snapshot(int band,const home_snapshot *h) {
 int off=band*BAND_H,c0=(int)h->bank;char title[24];
 put_i(put_s(put_i(put_s(title,"PATTERNS "),c0+1),"-"),c0+16);
 cv_text(8,6-off,&FONT_T,title,h->color[0]);
 int sw=tw(&FONT_T,h->status)+10;
 rbox(237-sw,3-off,sw,14,7,h->color[1]);
 text_r(232,6-off,&FONT_T,h->status,h->color[0]);
 rbox(3,20-off,234,112,UI_PANEL_RADIUS,h->color[1]);
 for(int c=0;c<16;c++) {
    char n[4];put_i(n,c0+c+1);text_c(HOME_X0+5+c*12,24-off,&FONT_T,n,h->color[0]);
 }
 for(int pos=0;pos<NPARTS;pos++) {
    int y=38+pos*18-off;
    cv_text(9,y+1,&FONT_T,h->names[pos],h->part_color[pos]);
    for(int c=0;c<16;c++) {
      int x=HOME_X0+c*12;unsigned f=h->flags[pos][c];
      rbox(x,y,10,12,2,h->color[2]);
      if(f&1)rframe(x,y,10,12,2,h->part_color[pos]);
      if(f&2)rframe(x,y,10,12,2,h->color[3]);
      if(f&4)for(int i=0;i<8;i++){
        uint8_t v=h->strength[pos][c][i];
        if(v)cv_rect(x+1+i,y+4,1,3,
          blend(h->color[5],h->part_color[pos],home_glyph_brightness(v,f&1)));
      }
      if(f&8)cv_rect(x,y+14,10,1,h->color[4]);
    }
 }
}
