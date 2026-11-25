/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_LAYER1_FIXTURE_MAIN retained_tuning_main
#include "ui_superj6_layer1_tuning_test.c"

enum { LINK_FRAMES=4096 };
static int32_t off_pcm[LINK_FRAMES*2], on_pcm[LINK_FRAMES*2], play_pcm[LINK_FRAMES*2];

static void turn_to(unsigned col,unsigned slot,int value)
{
    if (pref_get(pg.r[cur_page()][col])!=value) {panel_turn(col,slot,value);render(1);}
}
static void stock_single_link(void)
{
    build_pages();
    unsigned count=0;
    for(int i=0;i<pg.n;i++)for(unsigned k=0;k<4;k++){
        pref_t r=pg.r[i][k];
        if(r.kind==R_SJ6KNOB && r.c==SJ6_FILTER_LINK){
            count++;
            CHECK(k==3 && !strcmp(page_title(&pg,i),"FILTER"));
            CHECK(!strcmp(pref_desc(r)->name,"LINK"));
        }
        CHECK(strcmp(page_title(&pg,i),"AMP LINK"));
    }
    CHECK(count==1);
}
static unsigned render_snap(unsigned owner,int32_t *pcm,float expected)
{
    engine_apply_sound(&proj.sound);render(4);
    juno6_t *s=parts[owner];
    juno6_t isolated=*s;
    CHECK(juno6_note_on(&isolated,57,100));
    engine_bass_on(owner,57,0,0);
    for(unsigned i=0;i<LINK_FRAMES;i+=256){
        int32_t mix[512];float l[256],r[256];
        engine_render(mix,256);
        CHECK(juno6_render_stereo(&isolated,l,r,256));
        for(unsigned j=0;j<256;j++) {
            memcpy(&pcm[2*(i+j)],&l[j],sizeof l[j]);
            memcpy(&pcm[2*(i+j)+1],&r[j],sizeof r[j]);
        }
    }
    unsigned found=0;
    for(unsigned v=0;v<DNBX_VOICES;v++)if(s->core.voice[v].gate){
        for(unsigned l=0;l<2;l++){
            const dnbx_layer_voice *q=&s->core.voice[v].layer[l];
            CHECK(q->filter.stage==DNBX_ENV_SUSTAIN);
            CHECK(fabsf(q->filter.level-expected)<0.0000001f);
            found++;
        }
    }
    engine_bass_off(owner);render(16);return found;
}
static void journey(unsigned owner)
{
    setup_tuning(owner);
    stock_single_link();
    visit("AMP ADSR");
    turn_to(0,SJ6_AMP_ATTACK,0);turn_to(1,SJ6_AMP_DECAY,0);
    turn_to(2,SJ6_AMP_SUSTAIN,31);turn_to(3,SJ6_AMP_RELEASE,0);
    visit("FILTER ADSR");
    turn_to(0,SJ6_FILTER_ATTACK,0);turn_to(1,SJ6_FILTER_DECAY,0);
    turn_to(2,SJ6_FILTER_SUSTAIN,93);turn_to(3,SJ6_FILTER_RELEASE,0);
    visit("FILTER");
    CHECK(pg.r[cur_page()][2].kind==R_ENG);
    knob(2,70);render(1);
    turn_to(3,SJ6_FILTER_LINK,0);
    CHECK(!project_save() && !project_load());visit("FILTER");
    dnbx_layer_params before[2],after[2];
    for(unsigned i=0;i<2;i++)CHECK(!dnbx_get_layer(&parts[owner]->core,i,&before[i]));
    CHECK(before[0].amp.sustain != before[0].filter.sustain);
    CHECK(before[0].amp.sustain == before[1].amp.sustain);
    CHECK(before[0].filter.sustain == before[1].filter.sustain);
    CHECK(render_snap(owner,off_pcm,before[0].filter.sustain)==2);
    for(unsigned i=0;i<2;i++)CHECK(!dnbx_get_layer(&parts[owner]->core,i,&before[i]));
    const sound_t peer_pre=proj.sound;
    dnbx_layer_params peer[2], peer_after;
    for(unsigned i=0;i<2;i++)CHECK(!dnbx_get_layer(&parts[1-owner]->core,i,&peer[i]));
    CHECK(!parts[1-owner]->core.filter_link);
    turn_to(3,SJ6_FILTER_LINK,1);
    CHECK(parts[owner]->core.filter_link && !parts[1-owner]->core.filter_link);
    for(unsigned i=0;i<2;i++){
        CHECK(!dnbx_get_layer(&parts[owner]->core,i,&after[i]));
        CHECK(!memcmp(&before[i].amp,&after[i].amp,sizeof before[i].amp));
        CHECK(!memcmp(&before[i].filter,&after[i].filter,sizeof before[i].filter));
        CHECK(!dnbx_get_layer(&parts[1-owner]->core,i,&peer_after));
        CHECK(!memcmp(&peer_after,&peer[i],sizeof peer_after));
    }
    for(unsigned row=1-owner;row<8;row+=2)
        CHECK(!memcmp(peer_pre.v[T_303][row],proj.sound.v[T_303][row],NPARAMS_MAX));
    CHECK(render_snap(owner,on_pcm,before[0].amp.sustain)==2);
    unsigned n=0;
    for(unsigned i=0;i<LINK_FRAMES*2;i++)n+=off_pcm[i]!=on_pcm[i];
    CHECK(n>LINK_FRAMES);
    CHECK(!project_save() && !project_load());
    visit("FILTER");
    CHECK(pref_get(pg.r[cur_page()][3])==1);
    CHECK(render_snap(owner,play_pcm,before[0].amp.sustain)==2);
    CHECK(!memcmp(play_pcm,on_pcm,sizeof on_pcm));
    turn_to(3,SJ6_FILTER_LINK,0);
    CHECK(!project_save() && !project_load());visit("FILTER");
    CHECK(pref_get(pg.r[cur_page()][3])==0);
    CHECK(render_snap(owner,play_pcm,before[0].filter.sustain)==2);
    CHECK(!memcmp(play_pcm,off_pcm,sizeof off_pcm));
    printf("SJ6 owner=%u one FILTER K4, follow vs independent, %u/8192 changed, ON/OFF persisted exact PCM\n",owner,n);
}
int main(void){journey(0);journey(1);printf("PASS SuperJ6 single FILTER LINK %u checks\n",checks);}
