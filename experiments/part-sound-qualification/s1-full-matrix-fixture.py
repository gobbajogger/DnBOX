from pathlib import Path
import sys
r=Path(sys.argv[1]); app=r/'firmware/src/app'
p=app/'original_presets.inc'
s=p.read_text()
mark='static int s1_snapshot_locked(unsigned voice, s1_patch_snapshot *out)\n{'
assert s.count(mark)==1
s=s.replace(mark,'''/* Test-only: force precisely the real admission branch to be unavailable. */
unsigned s1_fx_test_matrix_lock,s1_fx_test_patch_lock,s1_fx_test_eg_lock;
unsigned s1_fx_test_not_ready;
'''+mark+'\n    if(s1_fx_test_not_ready)return 0;',1)
for api,field in [('engine_s1_matrix_snapshot','matrix'),('engine_s1_patch_snapshot','patch'),('engine_s1_envelope_view','eg')]:
    a=s.index('int '+api+'(')
    b=s.index('uint32_t lock=plat_cq_lock();',a)
    s=s[:b]+'s1_fx_test_'+field+'_lock++;\n    '+s[b:]
p.write_text(s)
p=app/'ui.c'
s=p.read_text()
a=s.index('static void cv_commit_main_band(unsigned band)')
b=s.index('static void cv_commit_knobs',a)
q=s[a:b];key='force, cv_lcd_emit, &pos);'
assert q.count(key)==1
q='static void s1_fx_test_between(unsigned);\n'+q.replace(key,key+'\n    s1_fx_test_between(band);')
s=s[:a]+q+s[b:]
p.write_text(s)
p=r/'tests/host/ui_synthone_frame_test.c'
s=p.read_text()
assert s.count('int main(void)')==1
s=s.replace('int main(void)','static int prior_s1_matrix_fixture(void)',1)
s+=r'''
extern unsigned s1_fx_test_matrix_lock,s1_fx_test_patch_lock,s1_fx_test_eg_lock,s1_fx_test_not_ready;
static unsigned fx_v,fx_mode,fx_calls;
static unsigned lock_matrix,lock_patch,lock_eg;
static void s1_fx_test_between(unsigned band)
{
    if(band||!fx_mode)return;
    ++fx_calls;
    CHECK(!s1_draw_frame);
    const unsigned w=fx_mode;
    if(w==1) {
        engine_bass_off_note(fx_v,60); blocks(2);
        edit(fx_v,26,9);
    }else{
        s1_fx_test_not_ready=0;
        engine_bass_on_velocity(fx_v,60,0,0,127); blocks(3);
    }
    palette_set(5);
    ui.part=PART_303A+(fx_v^1u);build_pages();
    *page_of(V_PART)=pg.n-1;
    ui.spage=1;ui.held_step=-1;
    /* Exclude injected engine activity while retaining any first-band reads. */
    lock_matrix=s1_fx_test_matrix_lock-lock_matrix;
    lock_patch=s1_fx_test_patch_lock-lock_patch;
    lock_eg=s1_fx_test_eg_lock-lock_eg;
}
static void draw_test_s1_matrix(void)
{
    memset(blit_hash,0,sizeof blit_hash);
    memset(cv_tile_valid,0,sizeof cv_tile_valid);
    draw_main();draw_knobs();
}
int main(void)
{
    setup(V_PART,PART_303A);
    choose_patch(0);choose_patch(1);
    for(unsigned v=0;v<2;v++)for(unsigned tail=0;tail<2;tail++)for(unsigned k=1;k<=2;k++) {
        fx_v=v; fx_mode=0; palette_set(0);ui.spage=0;
        engine_bass_off_note(v,60);blocks(128);
        show(v);
        *page_of(V_PART)=tail?pg.n-1:0;
        edit(v,26,112);
        if(k==1) {engine_bass_on_velocity(v,60,0,0,127);blocks(3);}
        else s1_fx_test_not_ready=1;
        draw_test_s1_matrix();
        memcpy(baseline[v],fb,sizeof fb);
        s1_fx_test_matrix_lock=s1_fx_test_patch_lock=s1_fx_test_eg_lock=0;
        fx_mode=k;
        draw_test_s1_matrix();
        fx_mode=0;
        CHECK(s1_fx_test_matrix_lock==lock_matrix+1);
        CHECK(s1_fx_test_patch_lock==lock_patch);
        CHECK(s1_fx_test_eg_lock==lock_eg);
        CHECK(!memcmp(baseline[v]+MAIN_Y*240,fb+MAIN_Y*240,sizeof(uint16_t)*(240-MAIN_Y)*240));
        draw_test_s1_matrix();
        CHECK(memcmp(baseline[v]+MAIN_Y*240,fb+MAIN_Y*240,sizeof(uint16_t)*(240-MAIN_Y)*240));
    }
    CHECK(fx_calls==8);
    puts("PASS both owner fixed-route held/release + patch change, unready/ready and one mask read across every three-strip frame");
    return 0;
}
'''
# Subtract injection delta, not total seen at end of callback.
s=s.replace('    const unsigned w=fx_mode;','    const unsigned w=fx_mode;\n    unsigned m=s1_fx_test_matrix_lock,p=s1_fx_test_patch_lock,e=s1_fx_test_eg_lock;')
s=s.replace('s1_fx_test_matrix_lock-lock_matrix','s1_fx_test_matrix_lock-m').replace('s1_fx_test_patch_lock-lock_patch','s1_fx_test_patch_lock-p').replace('s1_fx_test_eg_lock-lock_eg','s1_fx_test_eg_lock-e')
p.with_name('ui_s1_full_matrix_fault_test.c').write_text(s)

