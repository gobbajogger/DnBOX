/* Independent absolute-number/gate oracle for the restored sixteen-key strip.
 * Does not call sound_step_footer, step_of, bstep_gate, part_col or text_c. */
static void expect_sound_step_footer(void)
{
    int voice=ui.part-PART_303A;
    CHECK(voice>=0 && voice<2 && ui.spage>=0 && ui.spage<4);
    const bpart_t *pattern=&cur_pat()->bass[voice];
    cv_begin(240,16,SOUND_BASE);
    for(int column=0;column<16;column++) {
        int absolute=ui.spage*16+column,enabled=absolute<pattern->len;
        int x=8+14*column,gate=enabled?(pattern->step[absolute].flags&3):0;
        char number[8];snprintf(number,sizeof number,"%d",absolute+1);
        cv_text(x+6-tw(&FONT_T,number)/2,0,&FONT_T,number,enabled?SOUND_GRAY:SOUND_DIM);
        for(int row=13;row<=14;row++) {
            uint16_t ink=enabled?SOUND_LINE:SOUND_SIDE;
            if(enabled && (gate==1 || (gate==2 && row==13)))ink=PART_COL[ui.part];
            cv_rect(x,row,12,1,ink);
        }
        if(enabled && ui.held_step==absolute)cv_rect(x,12,12,1,SOUND_INK);
        if(enabled && seq.playing && eng_step[TRK_BASS0+voice]==absolute)cv_rect(x,15,12,1,SOUND_INK);
    }
    for(int y=0;y<16;y++)for(int x=0;x<240;x++)if(fb[(224+y)*240+x]!=cv_px[y*240+x]) {
        fprintf(stderr,"SOUND_STEP_FOOTER_MISMATCH x=%d y=%d\n",x,224+y);CHECK(0);
    }
}
