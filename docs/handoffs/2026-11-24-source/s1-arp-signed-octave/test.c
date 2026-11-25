/* SPDX-License-Identifier: GPL-3.0-only */
#include "arp_keyboard_helpers.h"
#include "s1c_part.h"

static unsigned notes[16], note_count, offs;
int __real_s1_part_note(s1_dry_part *, unsigned, unsigned);
int __wrap_s1_part_note(s1_dry_part *p, unsigned key, unsigned velocity)
{
    if(velocity && note_count<16)notes[note_count++]=key;
    if(!velocity)++offs;
    return __real_s1_part_note(p,key,velocity);
}
static void owner_case(unsigned owner)
{
    const unsigned peer=1-owner;
    const uint32_t step=1u<<WHITE_KEY[0];
    const uint64_t negative=UINT64_C(1)<<5, negative12=UINT64_C(1)<<12;
    engine_arp_steps st;
    uint8_t peer_wire[ARP_BLOCK_BYTES];
    setup(V_SEQ,PART_303A+owner);
    proj.set.bt |= PROJ_BT_CLK_IN;
    EXPECT(!engine_bass_select_preset(owner,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
    for(unsigned n=0;n<1024 && engine_bass_preset_pending(owner);++n)render(1);
    EXPECT(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE);
    *page_of(V_SEQ)=3;ui.oct=-1;build_pages();
    gesture(0,0);gesture(step,0);
    memcpy(peer_wire,proj.sound.v[T_303][ARP_BLOCK_VOICE0+peer],sizeof peer_wire);
    turn(2,1); /* original +octave toggle is independently owned */
    EXPECT(engine_arp_steps_read(owner,&st) && (st.oct_mask&1u) && (st.on_mask&1u));
    gesture(step,negative); /* C root=48; UI octave -1, local key 5 is pitch 29 */
    EXPECT(engine_arp_steps_read(owner,&st) && st.offset[0]==-7);
    gesture(step,0);gesture(step,negative12); /* -12 is not representable while boost is on. */
    EXPECT(engine_arp_steps_read(owner,&st) && st.offset[0]==-7 && (st.oct_mask&1u));
    gesture(0,0);
    EXPECT(engine_arp_steps_read(owner,&st) && (st.on_mask&1u));
    EXPECT(!memcmp(peer_wire,proj.sound.v[T_303][ARP_BLOCK_VOICE0+peer],sizeof peer_wire));
    engine_arp_cfg cfg;
    EXPECT(engine_arp_block_decode(proj.sound.v[T_303][ARP_BLOCK_VOICE0+owner], &cfg));
    EXPECT(cfg.offset[0]==-7 && (cfg.oct_mask&1u));
    cfg.mode=ARP_SEQ;cfg.on=1;cfg.length=1;
    EXPECT(engine_arp_preset(owner,&cfg)==S1_EDIT_QUEUED);
    render(4);
    EXPECT(!project_save() && !project_load());
    EXPECT(engine_arp_steps_read(owner,&st) && st.offset[0]==-7 && (st.oct_mask&1u));
    EXPECT(!memcmp(peer_wire,proj.sound.v[T_303][ARP_BLOCK_VOICE0+peer],sizeof peer_wire));
    note_count=offs=0;
    engine_bass_on(owner,48,0,0);render(4);
    EXPECT(note_count==1 && notes[0]==29);
    engine_bass_off(owner);render(4);EXPECT(offs>0);
    /* Two signed ranges including the original positive zero base. */
    static const int oct[4]={-1,-1,0,0}, key[4]={0,11,24,36}, raw[4]={-12,-1,0,12};
    for(unsigned i=0;i<4;++i){
        ui.oct=(int8_t)oct[i];gesture(step,0);gesture(step,UINT64_C(1)<<key[i]);gesture(0,0);
        EXPECT(engine_arp_steps_read(owner,&st) && st.offset[0]==raw[i] &&
               (st.oct_mask&1u) && (st.on_mask&1u));
    }
}
int main(void)
{
    owner_case(0);owner_case(1);
    if(failures){fprintf(stderr,"FAIL %u/%u\n",failures,expectations);return 1;}
    printf("PASS S1 both physical owner routes, independent OCT, reload and real backend notes %u checks\n",expectations);
    return 0;
}
