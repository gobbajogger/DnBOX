#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
static void s1_phaser_apply_one(unsigned owner,unsigned k,unsigned code)
{
    /* Live scalar edit, never restart the part or reset/reallocate its active Delay arena. */
    (void)s1c_front_set(&original_bass[owner].dsp.synthone.e,S1_PHASE_FIRST+k,s1_phase_value(k,code));
}
static void s1_phaser_apply(const sound_t *sound,unsigned owner)
{
    if(live_s1_mod(sound,owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE) {
        const uint8_t *cell=s1_phase_read(sound,owner);
        for(unsigned k=0;k<S1_PHASE_COUNT;k++)
            if(cell[k]<=S1_PHASE_MAX)s1_phaser_apply_one(owner,k,cell[k]);
    }
}
int s1_phaser_get(unsigned owner,unsigned k)
{
    if(k>=S1_PHASE_COUNT || !live_s1_mod(mot_base_snd,owner) ||
       original_bass[owner].kind!=ORIGINAL_SOUND_SYNTHONE)return -1;
    unsigned code=s1_phase_read(mot_base_snd,owner)[k];
    return code<=S1_PHASE_MAX ? (int)s1_phase_position(k,code) : -1;
}
int s1_phaser_choose(unsigned owner,unsigned k,unsigned pos)
{
    if(pos>S1_PHASE_MAX || s1_phaser_get(owner,k)<0)return 0;
    engine_project_hold(1);
    int before=s1_phaser_get(owner,k);
    int ok=before>=0 && (unsigned)before!=pos && s1_typed_tuple_editable() &&
           !ui_project_busy() && !engine_bass_preset_pending(owner) && !note_lock_applied[owner];
    if(ok) {
        unsigned code=s1_phase_encode(k,pos);
        s1_phase_write((sound_t*)mot_base_snd,owner)[k]=(uint8_t)code;
        s1_phaser_apply_one(owner,k,code);
    }
    engine_project_hold(0);
    return ok;
}
int s1_phaser_undo_ready(unsigned mask)
{
    if(!s1_typed_tuple_editable())return 0;
    for(unsigned v=0;v<NBASS;v++)
        if((mask&(1u<<v)) && live_s1_mod(mot_base_snd,v) &&
           (note_lock_applied[v] || engine_bass_preset_pending(v)))return 0;
    return 1;
}
void s1_phaser_undo(unsigned mask)
{
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_phaser_apply(mot_base_snd,v);
}
#endif
