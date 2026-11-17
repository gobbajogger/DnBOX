/* SPDX-License-Identifier: GPL-3.0-only */
/* Read-only presentation of existing stored step values. No new project planes. */
enum { UI_LANE_VELOCITY, UI_LANE_CHANCE, UI_LANE_PITCH, UI_LANE_PAN, UI_LANE_COUNT };
static uint8_t ui_step_lane;
static int step_lane_motion(int part,int voice,int step,int target,int parameter,int fallback)
{
    int k=motion_find(proj.arr.lane,seq.ppat[part],part,target,voice,parameter);
    return k>=0 && proj.arr.lane[k].val[step]!=MOT_NONE?proj.arr.lane[k].val[step]:fallback;
}
static int step_lane_value(int part,int voice,int step,int kind,int *maximum,int *conditional)
{
    const pattern_t *pat;
    pref_t ref=NONE;
    const x0x_param_t *desc;
    int value;
    *maximum=127;*conditional=0;
    if((unsigned)part>=NPARTS || (unsigned)step>=NSTEPS || (unsigned)kind>=UI_LANE_COUNT ||
       (part<NKIT && (unsigned)voice>=NDRUM))return -1;
    pat=pat_of(part);
    if(kind==UI_LANE_CHANCE) {
        value=step_lane_motion(part,(part<<4)|(part<NKIT?voice:0),step,T_PROB,0,100);
        *maximum=100;*conditional=value>100;
        return value;
    }
    if(kind==UI_LANE_VELOCITY) {
        if(part<NKIT) {
            value=step_lane_motion(part,(part<<4)|voice,step,T_VEL,0,0);
            return value?value:sm_get(pat->drum[part].accent,step)?127:seq.accent_q7;
        }
        if(part_is_303(part))return bstep_velocity(&pat->bass[part-PART_303A].step[step]);
        /* BREAK has no admitted per-step velocity path yet. Never call LEVEL velocity. */
        return -1;
    }
    if(kind==UI_LANE_PAN) {
        ref=part<NKIT?find_ref(part==PART_909?T_909:T_808,voice,"Pan"):PR(R_ENG,T_MIX,part,MX_PAN);
    } else if(part_is_303(part)) {
        return pat->bass[part-PART_303A].step[step].note;
    } else if(part<NKIT) {
        int target=part==PART_909?T_909:T_808;
        ref=find_ref(target,voice,"Pitch");
        if(ref.kind==R_NONE)ref=find_ref(target,voice,"Tune");
    } else ref=find_ref(T_BRK,0,"Pitch");
    desc=pref_desc(ref);
    if(ref.kind!=R_ENG || !desc || desc->max<=0)return -1;
    *maximum=desc->max;
    return step_lane_motion(part,ref.b,step,ref.a,ref.c,pref_get(ref));
}
static int step_lane_has_note(int part,int voice,int step)
{
    const pattern_t *pat=pat_of(part);
    if(part<NKIT)return sm_get(pat->drum[part].hit[voice],step);
    if(part_is_303(part))return bstep_gate(&pat->bass[part-PART_303A].step[step])!=G_REST;
    return step<16 && !!(pat->brk.steps&(1u<<step));
}
static UI_DRAW void draw_step_lane(void)
{
    static const char *const labels[UI_LANE_COUNT]={"VELOCITY","CHANCE","PITCH","PAN"};
    int kind=ui_step_lane<UI_LANE_COUNT?ui_step_lane:UI_LANE_VELOCITY;
    int part=ui.part,voice=part<NKIT?ui.sel[part]:0;
    cv_rect(0,69,240,13,SOUND_FACE); /* replace only the old footer, preserve four knobs */
    cv_text(4,69,&FONT_T,labels[kind],SOUND_GRAY);
    for(int c=0;c<16;c++) {
        int step=part==PART_BRK?c:step_of(c),maximum,conditional;
        int value=step_lane_value(part,voice,step,kind,&maximum,&conditional);
        int x=69+c*10;
        uint16_t ink=step_lane_has_note(part,voice,step)?part_col():SOUND_LINE;
        cv_rect(x,72,1,8,SOUND_LINE);
        if(value<0) {cv_rect(x-1,78,3,1,SOUND_GRAY);continue;}
        if(conditional) {box(x-2,72,5,7,ink);continue;}
        if(value>maximum)value=maximum;
        int h=(value*7+maximum/2)/maximum;
        cv_rect(x-1,79-h,3,h+1,ink);
    }
}
