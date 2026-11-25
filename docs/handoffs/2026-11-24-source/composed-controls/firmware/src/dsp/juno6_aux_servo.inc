/* Real target IDs in the original Hack chooser. Use separate audio-effective
 * values so an LFO never writes project bytes, pots or authored readback. */
static int sj6_aux_servo_slot(unsigned id)
{
    switch(id){
    case 88:return 0;case 96:return 1;case 100:return 2;
    case 89:return 3;case 97:return 4;case 101:return 5;
    default:return -1;
    }
}
static float sj6_aux_servo_base(const juno6_t *s,unsigned id)
{
    int k=sj6_aux_servo_slot(id);
    float value=0.f;
    if(k<0)return -1.f;
    if(dnbx_read_raw(&s->core,id,&value)==DNBX_OK)return fm_clampf(value,0.f,1.f);
    if(k==4)return (s->lfo2_pwm&SJ6_LFO2_PWM_DEPTH_MASK)*(1.f/127.f);
    if(k==5)return s->aux_lfo[2]*(1.f/127.f);
    return value;
}
static void sj6_aux_servo_commit(juno6_t *s,unsigned mask,const float target[6],unsigned cursor,unsigned phase3)
{
    const unsigned old=s->aux_servo_mask;
    if(!(old|mask))return;
    s->aux_servo_mask=(uint8_t)mask;
    for(unsigned i=0;i<6u;i++){
        unsigned bit=1u<<i;
        if(!(old&bit) && !(mask&bit))continue;
        if((old&bit) && (mask&bit) && s->aux_servo[i]==target[i])continue;
        if(mask&bit)s->aux_servo[i]=target[i];
        if(i>=3u)continue; /* amounts have no phase accumulator */
        /* Anchor the old phase at the actual write point before changing the
         * step. This preserves a continuous integral over multiple spans. */
        const uint64_t step=s->lfo_step_q40[i];
        prepare_lfo_step(s,i);
        unsigned passed=i==2u?phase3:cursor;
#if X0X_SJ6_LFO1_DELAY_PORT
        if(!i && sj6_lfo1_waiting(s))passed=0;
#endif
        s->lfo_phase_q40[i]+=(step-s->lfo_step_q40[i])*passed;
    }
}
