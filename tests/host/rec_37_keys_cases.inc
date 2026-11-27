/* SPDX-License-Identifier: GPL-3.0-only */
static unsigned checks;
#define T(c) do{++checks;if(!(c)){fprintf(stderr,"FAIL %d %s\n",__LINE__,#c);exit(1);}}while(0)
static void audio_block(void){int32_t out[512];engine_render(out,256);}
static void physical(unsigned p,unsigned velocity)
{
    smk__key_event(p,velocity);held_keys=fm1_in.notes;
    for(unsigned k=0;k<NKEYS;k++)key_velocity[k]=fm1_in.key_vel[k+5];
}
static void pad(unsigned k,int down)
{
    if(down)fm1_in.pnotes|=1u<<k;else fm1_in.pnotes&=~(1u<<k);
    held_keys=fm1_in.notes=fm1_in.knotes|fm1_in.pnotes;
}
static void scan(void){ui_input_only();audio_block();ui_input_only();audio_block();}
static void tap(unsigned p){physical(p,93);physical(p,0);}
static void prep(void)
{
    ui.part=PART_303A;set_view(V_SEQ);build_pages();ui.oct=0;
    ui.freeplay[0]=0;ui.rec=1;scan();scan();
    T(live_rec && live_owner==0 && smk_ext_enabled==PLAT_PHYS_REC);
}
int main(void)
{
    boot();tick(300);prep();
    bpart_t *bp=&proj.pat[seq.ppat[PART_303A]].bass[0];
    bp->len=16;
    for(unsigned p=0;p<37;p++){
        ui.wpos[0]=0;memset(bp->step,0,sizeof bp->step);
        tap(p);scan();T(bst(0)->note==36+p && bstep_gate(bst(0))==G_NOTE);
        T(ui.wpos[0]==1 && !live_keys[0].n);
    }
    memset(bp->step,0,sizeof bp->step);ui.wpos[0]=0;
    static const unsigned order[]={36,17,0,20};
    for(unsigned p=0;p<4;p++)tap(order[p]);scan();
    for(unsigned p=0;p<4;p++)T(bst(p)->note==36+order[p]);
    T(ui.wpos[0]==4);
    /* Full FIFO: discard the burst instead of writing guessed/reordered
       notes; release/suppression and the next unambiguous tap recover. */
    ui.wpos[0]=0;memset(bp->step,0,sizeof bp->step);
    for(unsigned p=0;p<37;p++)physical(p,100);scan();
    T(ui.wpos[0]==0 && !live_keys[0].n && !(engine_bass_keys_held()&1));
    for(unsigned p=0;p<37;p++)physical(p,0);scan();
    tap(36);scan();T(bst(0)->note==72&&ui.wpos[0]==1);
    /* Pad alias still edits at its original pitch; same pitch remains owned
       through a physical release while pad stays held. */
    memset(bp->step,0,sizeof bp->step);ui.wpos[0]=0;
    pad(10,1);scan();T(bst(0)->note==51 && kbd_held_key[0]==11);
    physical(15,90);scan();T(kbd_held_key[0]==rec_key_token(15));
    physical(15,0);scan();T(kbd_held_key[0]==11 && (engine_bass_keys_held()&1));
    pad(10,0);scan();T(!(engine_bass_keys_held()&1));
    /* Held pitch survives octave change, then owner cutover releases A. */
    physical(36,96);scan();T(kbd_held_note[0]==72 && live_keys[0].n==1);
    ui.oct=1;scan();T(kbd_held_note[0]==72);
    physical(36,0);scan();T(!(engine_bass_keys_held()&1));
    ui.oct=0;scan();
    physical(0,96);scan();T(kbd_held_note[0]==36);
    ui.part=PART_303B;scan();T(live_owner==1 && !(engine_bass_keys_held()&1));
    physical(36,96);scan();T(kbd_held_note[1]==72 && (engine_bass_keys_held()&2));
    physical(0,0);scan();T(engine_bass_keys_held()&2);
    physical(36,0);scan();T(!(engine_bass_keys_held()&2));
    prep();
    /* Real audio-owned recorder under transport; every physical strike writes
       its new note into a real project step and won't duplicate on release. */
    btn(B_PLAY,1);btn(B_PLAY,0);tick(20);T(ui.rec&&seq.playing&&live_rec);
    for(unsigned p=0;p<37;p++) {
        at_step(TRK_BASS0,(int)(p%16),10);
        physical(p,110);scan();
        T(bst(p%16)->note==36+p && bstep_gate(bst(p%16))==G_NOTE);
        T(kbd_held_note[0]==36+p);
        physical(p,0);scan();T(!live_keys[0].n);
    }
    at_step(TRK_BASS0,5,10);tap(36);tap(17);tap(0);scan();T(bst(5)->note==36);
    at_step(TRK_BASS0,8,10);held_btn|=1u<<B_ENV;physical(36,90);scan();
    T(bst(8)->note==72 && (bst(8)->flags&BS_ACCENT));
    held_btn&=~(1u<<B_ENV);physical(36,0);scan();T(ui.view==V_SEQ);
    at_step(TRK_BASS0,9,10);physical(0,100);scan();
    T(bst(9)->note==36 && kbd_held_key[0]==rec_key_token(0));
    set_view(V_PART);scan();
    T(live_release_only && kbd_held_key[0]==rec_key_token(0));
    physical(0,0);scan();T(!kbd_held_key[0] && !live_keys[0].n);
    set_view(V_SEQ);scan();
    at_step(TRK_BASS0,10,10);pad(10,1);scan();T(bst(10)->note==51);
    pad(10,0);scan();T(!(engine_bass_keys_held()&1));
    T(!irq_depth);
    printf("PASS %u checks: physical 37-key stopped + playing REC, exact 36–72 mapping, mixed FIFO ordering, alias/release/ownership\n",checks);
    return 0;
}
