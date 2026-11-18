/* Called after existing resolved-control helpers, on the normal knob canvas.
 * Existing glyph implementations remain explicitly unqualified until replaced
 * by the canonical per-type adapters; this change establishes family geometry.
 */
static void break_layout_controls(void)
{
    static const int16_t centers[4]={32,90,149,207};
    int pgi=cur_page();
    night_panel(3,146-KNOB_Y,234,64,0);
    for(int i=0;i<4;i++){
#if X0X_GFINAL_CHANCE_PROBE
        chance_knob_status[i]=CHANCE_NOT_TARGET;
#endif
        pref_t r=pg.r[pgi][i];const x0x_param_t *d=pref_desc(r);
        if(!d)continue;
        int cx=centers[i],val=pref_get(r),touched=ui.touched==i;
        char num[16],unit[8];
        text_c(cx,vc(&FONT_T,0,12),&FONT_T,caps_of(pref_name(r)),touched?C_WHITE:C_GRAY);
        if(lock_step()>=0){
            int lv=lock_of(r,lock_step());if(lv>=0)val=lv;
            if(lockable(r))dot(cx+24,5,1,lv>=0?C_AMB:C_LINE);
        }else if(has_motion(r)){
            int mv=motion_now(r);if(mv>=0)val=mv;
            dot(cx+24,5,1,mv>=0?C_WHITE:C_DIM);
        }
        /* Canonical body occupies173..191; preserve existing glyph value law
         * while moving its established y28 body to the new y15 local cell. */
        int old_oy=cv_oy;cv_oy-=13;
#if X0X_GFINAL_CHANCE_PROBE
        chance_knob_status[i]=(uint8_t)chance_native_knob(r,d,val,cx);
        if(chance_knob_status[i]==CHANCE_DRAWN) {
            /* Canonical body at local y15 via the existing cv_oy=-13. */
        } else
#endif
        if(!(perform_glyph(r,val,cx,part_col())||control_glyph(r,val,cx,part_col()))&&d->names&&d->max<12){
            int n=d->max+1,pw=n>6?3:6,w=n*(pw+2)-2;
            for(int k=0;k<n;k++)box(cx-w/2+k*(pw+2),33,pw,9,k==val?part_col():C_LINE);
        }
        cv_oy=old_oy;
        pref_value_of(r,val,num,unit);
        cell_value(cx,vc(&FONT_B,39,13),num,unit,touched?C_WHITE:C_HI);
    }
    if(ui.view==V_SEQ&&pgi==0){
        char t[20];
        put_i(put_s(t,"A SLICES "),(int)brk_slice_count(&cur_pat()->brk,0));
        cv_text(105,vc(&FONT_T,17,10),&FONT_T,t,C_GRAY);
        put_i(put_s(t,"B SLICES "),(int)brk_slice_count(&cur_pat()->brk,1));
        cv_text(105,vc(&FONT_T,31,10),&FONT_T,t,C_GRAY);
    }
    break_layout_footer();
}
