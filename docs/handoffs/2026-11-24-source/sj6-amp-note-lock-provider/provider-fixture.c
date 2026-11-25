/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_LAYER1_FIXTURE_MAIN retained_tuning_main
#include "ui_superj6_layer1_tuning_test.c"
#include "sj6_note_amp_provider.h"

typedef struct {
    unsigned owner, code, held, mixer;
    int kind;
    sj6_note_amp_t state;
} trace_t;
static trace_t trace[32];
static unsigned count;
static sj6_note_amp_t input_state[2], applied[2];
static int capture_mode[2];
static unsigned called[2], effective_called[2];

int sj6_note_amp_snapshot(unsigned owner, int effective, sj6_note_amp_t *out)
{
    CHECK(owner<2 && out);
    ++called[owner];
    effective_called[owner] += !!effective;
    if(capture_mode[owner]==SJ6_NOTE_AMP_PRESENT)*out=input_state[owner];
    return capture_mode[owner];
}
void sj6_note_amp_apply(unsigned owner, const sj6_note_amp_t *snapshot)
{
    CHECK(owner<2 && parts[owner] && count<32);
    trace_t *t=&trace[count++];
    t->owner=owner;t->code=parts[owner]->osc2_level_code;
    t->held=juno6_active(parts[owner]);
    t->kind=!!snapshot;
    const sj6_note_amp_t empty={{0,0,0,0},0,0};
    t->state=applied[owner]=snapshot?*snapshot:empty;
}
static void set_osc(unsigned v, unsigned code)
{
    CHECK(engine_superj6_knob_turn(v,SJ6_OSC2_LEVEL,code,0));
    render(1);
}
static int same_amp(const sj6_note_amp_t *a,const sj6_note_amp_t *b)
{
    return a->link==b->link && a->mask==b->mask && !memcmp(a->q16,b->q16,sizeof a->q16);
}
static void until(unsigned want)
{
    for(unsigned i=0;i<3500 && count<want;++i)render(1);
    CHECK(count>=want);
}
int main(void)
{
    setup_tuning(0);
    for(unsigned v=0;v<2;++v){
        engine_note_lock_capture(1);
        set_osc(v,47);
        CHECK(!engine_bass_note_capture(proj.arr.lane,0,v,1,&proj.sound));
        int i1=bass_note_find(proj.arr.lane,0,v,1);
        CHECK(i1>=0 && proj.arr.lane[i1].i==BASS_NOTE_VERSION);
        capture_mode[v]=SJ6_NOTE_AMP_PRESENT;
        input_state[v]=(sj6_note_amp_t){{0,0x1234,(uint16_t)(v+32768),65535},15,(uint8_t)(1+v)};
        /* A very different persisted AMP2 ARP entry must not supply this snapshot. */
        engine_arp_cfg stale;
        uint8_t *arp_block=proj.sound.v[T_303][ARP_BLOCK_VOICE0+v];
        (void)engine_arp_block_decode(arp_block,&stale);
        stale.amp2_mask=15;
        for(unsigned i=0;i<4;++i)stale.amp2[i]=444u;
        engine_arp_block_encode(&stale,arp_block);
        uint8_t stale_wire[NPARAMS_MAX];memcpy(stale_wire,arp_block,sizeof stale_wire);
        set_osc(v,71);
        CHECK(!seq_bass_sound_prepare(&seq,v,0,0));
        int i2=bass_note_find(proj.arr.lane,0,v,0);
        CHECK(i2>=0 && proj.arr.lane[i2].i==SJ6_NOTE_AMP_VERSION);
        lane_t good=proj.arr.lane[i2];
        sj6_note_amp_t decoded;
        CHECK(sj6_note_amp_decode(&decoded,good.val+BASS_NOTE_TAG,1));
        CHECK(same_amp(&input_state[v],&decoded));
        capture_mode[v]=SJ6_NOTE_AMP_UNAVAILABLE;
        CHECK(seq_bass_sound_prepare(&seq,v,0,0)<0);
        CHECK(!memcmp(&good,&proj.arr.lane[i2],sizeof good));
        capture_mode[v]=SJ6_NOTE_AMP_PRESENT;
        input_state[v].link=3;
        CHECK(seq_bass_sound_prepare(&seq,v,0,0)<0);
        CHECK(!memcmp(&good,&proj.arr.lane[i2],sizeof good));
        input_state[v].link=(uint8_t)(1+v);
        CHECK(called[v]==4 && effective_called[v]==3);
        engine_note_lock_capture(0);
        set_osc(v,19);
        CHECK(!memcmp(stale_wire,arp_block,sizeof stale_wire));
        CHECK(!project_save());
        CHECK(!project_load());
        /* Adopted Juno allocations are reinitialized in the same owner objects. */
        CHECK(engine_bass_bank(v)==ORIGINAL_SOUND_JUNO);
        CHECK(!memcmp(&good,&proj.arr.lane[i2],sizeof good));
        bpart_t *line=&proj.pat[0].bass[v];
        for(unsigned i=0;i<64;++i)line->step[i].flags=G_REST;
        line->len=16;line->rate=RATE_16;line->dir=DIR_FWD;line->transpose=24;
        for(unsigned i=0;i<3;++i){
            line->step[i].flags=G_NOTE;line->step[i].note=45+i;line->step[i].velocity=100;
        }
        seq.ppat[TRK_BASS0+v]=0;engine_note_lock_refresh();engine_bass_edit_hold(1);
        unsigned start=count;
        seq_start(&seq);
        until(start+1);
        CHECK(trace[start].owner==v && trace[start].kind==1 && trace[start].code==72);
        CHECK(!trace[start].held); /* overlay precedes the first key-on */
        CHECK(same_amp(&trace[start].state,&decoded));
        until(start+2);
        CHECK(trace[start+1].owner==v && trace[start+1].kind==0);
        CHECK(trace[start+1].held); /* prior onset still audible incl. release */
        CHECK(trace[start+1].code==72 && parts[v]->osc2_level_code==48);
        CHECK(!applied[v].mask && !applied[1-v].mask);
        seq_stop(&seq);engine_bass_edit_hold(0);render(4);
        CHECK(count==start+2 && parts[v]->osc2_level_code==20);
        /* Remove v1 so second onset has no lock: restore final base after old writes. */
        seq_bass_sound_clear(&seq,v,0,1);
        engine_note_lock_refresh();engine_bass_edit_hold(1);seq_start(&seq);
        until(start+3);
        CHECK(trace[start+2].owner==v && trace[start+2].kind==1);
        until(start+4);
        CHECK(trace[start+3].owner==v && trace[start+3].kind==0 && trace[start+3].code==20);
        seq_stop(&seq);engine_bass_edit_hold(0);render(4);
        /* Stop from a live v2, before another onset. */
        engine_bass_edit_hold(1);seq_start(&seq);until(start+5);
        CHECK(trace[start+4].kind==1 && trace[start+4].owner==v);
        seq_stop(&seq);engine_bass_edit_hold(0);render(4);
        CHECK(count==start+6 && trace[start+5].kind==0 && trace[start+5].owner==v && trace[start+5].code==20);
        seq_bass_sound_clear(&seq,v,0,0);engine_note_lock_refresh();
        for(unsigned i=0;i<64;++i)line->step[i].flags=G_REST;
    }
    printf("PASS opt-in authoritative capture, EEPROM-free host storage and real two-owner sequencer old/new lock ordering (%u checks)\n",checks);
    return 0;
}
