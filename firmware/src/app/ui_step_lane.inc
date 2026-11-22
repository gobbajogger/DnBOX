/* SPDX-License-Identifier: GPL-3.0-only */
/* Stored pattern/next-trigger values; no audio or project planes are added. */
typedef struct {
    const pattern_t *pat;
    const lane_t *motion;
    pref_t ref;
    int part,voice,kind,maximum,fallback,available;
} step_lane_context_t;
static void step_lane_prepare(step_lane_context_t *c,int part,int voice,int kind)
{
    pref_t ref=NONE;
    const x0x_param_t *desc;
    int target=-1,parameter=0,motion_voice=voice;
    memset(c,0,sizeof *c);c->maximum=127;
    c->part=part;c->voice=voice;c->kind=kind;
    if((unsigned)part>=NPARTS || (unsigned)kind>=UI_LANE_COUNT ||
       (part<NKIT && (unsigned)voice>=NDRUM))return;
    c->pat=pat_of(part);c->available=1;
    if(kind==UI_LANE_CHANCE) {
        target=T_PROB;motion_voice=(part<<4)|(part<NKIT?voice:0);
        c->maximum=c->fallback=100;
    } else if(kind==UI_LANE_VELOCITY) {
        if(part<NKIT) {target=T_VEL;motion_voice=(part<<4)|voice;}
        else if(part==PART_BRK) {
            target=T_VEL;motion_voice=PART_BRK<<4;c->fallback=127;
            c->ref=PR(R_ENG,T_VEL,motion_voice,0);
        }
    } else if(kind==UI_LANE_PITCH && part_is_303(part)) {
        /* Native stored note below, without an invented parameter lock. */
    } else {
        if(kind==UI_LANE_PAN)
            ref=part<NKIT?find_ref(part==PART_909?T_909:T_808,voice,"Pan"):PR(R_ENG,T_MIX,part,MX_PAN);
        else if(part<NKIT) {
            int engine=part==PART_909?T_909:T_808;
            ref=find_ref(engine,voice,"Pitch");
            if(ref.kind==R_NONE)ref=find_ref(engine,voice,"Tune");
        } else ref=find_ref(T_BRK,0,"Pitch");
        desc=pref_desc(ref);
        if(ref.kind!=R_ENG || !desc || desc->max<=0)c->available=0;
        else {target=ref.a;motion_voice=ref.b;parameter=ref.c;c->maximum=desc->max;c->fallback=pref_get(ref);c->ref=ref;}
    }
    if(target>=0) {
        int k=motion_find(proj.arr.lane,seq.ppat[part],part,target,motion_voice,parameter);
        if(k>=0)c->motion=&proj.arr.lane[k];
    }
}
static int step_lane_read(const step_lane_context_t *c,int step,int *conditional)
{
    int value=c->fallback;*conditional=0;
    if(!c->available || (unsigned)step>=NSTEPS)return -1;
    if(c->motion && c->motion->val[step]!=MOT_NONE)value=c->motion->val[step];
    if(c->kind==UI_LANE_CHANCE)*conditional=value>100;
    else if(c->kind==UI_LANE_VELOCITY) {
        if(c->part<NKIT)return value?value:sm_get(c->pat->drum[c->part].accent,step)?127:seq.accent_q7;
        if(c->part==PART_BRK)return value;
        return bstep_velocity(&c->pat->bass[c->part-PART_303A].step[step]);
    } else if(c->kind==UI_LANE_PITCH && part_is_303(c->part))return c->pat->bass[c->part-PART_303A].step[step].note;
    return value;
}
/* SEQ + a held step + K3 (FM-1 ALGO) edits the visible stored lane.
 * Plain K3, K2/chance and the four page knobs retain their established roles.
 * Generic parameter edits belong to the next trigger, without a live preview. */
X0X_COLD static int step_lane_turn(int e)
{
    int part=ui.part,step,conditional,value,next,k,handled=1,changed=0,full=0;
    uint32_t state=0;
    step_lane_context_t c;
    if(!e)return 0;
    /* Chance owns its complete BREAK transaction; never nest that guard. */
    if(ui_step_lane==UI_LANE_CHANCE) {prob_turn(e);return 1;}
    if(part==PART_BRK)state=plat_cq_lock();
    step=held_any_step();
    if(step<0) {handled=0;goto done;}
    step_lane_prepare(&c,part,part<NKIT?ui.sel[part]:0,ui_step_lane);
    value=step_lane_read(&c,step,&conditional);
    if(is_303())ui.step_edited=1;else ui.hold_used=1;
    if(value<0)goto done; /* unavailable native quantity; never substitute LEVEL */
    if(c.kind==UI_LANE_PITCH && is_303()) {
        held_note_by(e); /* native helper owns successful edits and dirty state */
        goto done;
    }
    next=value+e;
    next=next<1&&c.kind==UI_LANE_VELOCITY&&part!=PART_BRK?1:next<0?0:next>c.maximum?c.maximum:next;
    if(next==value)goto done;
    if(c.kind==UI_LANE_VELOCITY && part!=PART_BRK) {
        if(part<NKIT) {
            drum_vel_set_step(part,ui.sel[part],step,(uint8_t)next);
            k=motion_find(proj.arr.lane,seq.ppat[part],part,T_VEL,(part<<4)|ui.sel[part],0);
            changed=k>=0 && proj.arr.lane[k].val[step]==next;
        } else {
            cur_pat()->bass[bidx()].step[step].velocity=(uint8_t)next;
            changed=1;
        }
    } else {
        k=motion_find(proj.arr.lane,seq.ppat[part],part,c.ref.a,c.ref.b,c.ref.c);
        if(k<0)k=motion_alloc(proj.arr.lane,seq.ppat[part],part,c.ref.a,c.ref.b,c.ref.c);
        if(k<0) {full=1;goto done;}
        proj.arr.lane[k].val[step]=(uint8_t)next;
        /* Returning to unity releases sparse storage once no other step needs it. */
        if(part==PART_BRK && c.kind==UI_LANE_VELOCITY && next==127) {
            lane_t *lane=&proj.arr.lane[k];
            lane->val[step]=MOT_NONE;
            int j=0;
            while(j<NSTEPS && lane->val[j]==MOT_NONE)j++;
            if(j==NSTEPS)engine_motion_clear(k);
        }
        changed=1;
    }
done:
    if(part==PART_BRK)plat_cq_unlock(state);
    if(changed)mark_dirty();
    if(full)say("MOTION FULL",0);
    return handled;
}
static int step_lane_value(int part,int voice,int step,int kind,int *maximum,int *conditional)
{
    uint32_t guard=part==PART_BRK?plat_cq_lock():0;
    step_lane_context_t c;step_lane_prepare(&c,part,voice,kind);*maximum=c.maximum;
    int value=step_lane_read(&c,step,conditional);
    if(part==PART_BRK)plat_cq_unlock(guard);
    return value;
}
static int step_lane_has_note(int part,int voice,int step)
{
    const pattern_t *pat=pat_of(part);
    if(part<NKIT)return sm_get(pat->drum[part].hit[voice],step);
    if(part_is_303(part))return bstep_gate(&pat->bass[part-PART_303A].step[step])!=G_REST;
    return step<16 && !!(pat->brk.steps&(1u<<step));
}
typedef struct {
    int16_t value[16];
    uint16_t notes;
    int maximum;
} step_lane_snapshot_t;

/* Take the same coherent view for the strip and its redraw signature. Release
 * the audio guard before any painting or display I/O. */
static void step_lane_snapshot(step_lane_snapshot_t *snapshot,int kind)
{
    int part=ui.part,voice=part<NKIT?ui.sel[part]:0;
    uint32_t guard=part==PART_BRK?plat_cq_lock():0;
    step_lane_context_t context;
    step_lane_prepare(&context,part,voice,kind);
    snapshot->maximum=context.maximum;
    snapshot->notes=0;
    for(int c=0;c<16;c++) {
        int conditional,step=part==PART_BRK?c:step_of(c);
        snapshot->value[c]=(int16_t)step_lane_read(&context,step,&conditional);
        if(step_lane_has_note(part,voice,step))snapshot->notes|=(uint16_t)(1u<<c);
    }
    if(part==PART_BRK)plat_cq_unlock(guard);
}

static UI_DRAW void draw_step_lane(void)
{
    static const char *const labels[UI_LANE_COUNT]={"VELOCITY","CHANCE","PITCH","PAN"};
    int kind=ui_step_lane<UI_LANE_COUNT?ui_step_lane:UI_LANE_VELOCITY;
    step_lane_snapshot_t snapshot;step_lane_snapshot(&snapshot,kind);
    cv_rect(0,69,240,13,SOUND_FACE); /* replace only the old footer, preserve four knobs */
    cv_text(4,69,&FONT_T,labels[kind],SOUND_GRAY);
    /* BREAK's playback position remains visible even when the step is a rest. */
    if(ui.part==PART_BRK && seq.playing) {
        unsigned phase=eng_step[TRK_BRK];
        if(phase<16)cv_rect(67+phase*10,71,5,1,part_col());
    }
    for(int c=0;c<16;c++) {
        int maximum=snapshot.maximum,value=snapshot.value[c];
        int x=69+c*10;
        uint16_t ink=(snapshot.notes & (1u<<c))?part_col():SOUND_LINE;
        cv_rect(x,72,1,8,SOUND_LINE);
        if(value<0) {cv_rect(x-1,78,3,1,SOUND_GRAY);continue;}
        if(kind==UI_LANE_CHANCE && value>100) {box(x-2,72,5,7,ink);continue;}
        if(value>maximum)value=maximum;
        int h=(value*7+maximum/2)/maximum;
        cv_rect(x-1,79-h,3,h+1,ink);
    }
}
