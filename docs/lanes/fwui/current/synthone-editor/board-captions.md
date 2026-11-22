# Synth One physical envelope captions

FWUI `61827a714ca7b240c01a55ed347b5fd7b847c7c2` and Sound `80b7e1db59e5c3eceb2b3530e80c2cedf29c5b7f` were composed with the repository qualifier. Two native host builds used the production board caption selection. The FM-1 build used the FM-1 host. The SMK build used the standard SMK input fixture. Both used the production Sound engine, R22 renderer and generated Barlow font.

The observer independently formats the four expected slot labels from the applied parameter values and compares a 230 × 12 pixel strip to the actual RGB565 framebuffer. It checks both envelope rows on each bass part. FM-1 displays K1 through K4. SMK displays K5 through K8. Across 32 labels and 22,080 pixels the matching layouts produced zero mismatches. Substituting the other board’s legends changed 139 pixels in each of the eight row comparisons.

## Recorded output

```text
board=FM1 bass=A row=2 matched-layout mismatches=0 other-layout mismatches=139
board=FM1 bass=A row=3 matched-layout mismatches=0 other-layout mismatches=139
board=FM1 bass=B row=2 matched-layout mismatches=0 other-layout mismatches=139
board=FM1 bass=B row=3 matched-layout mismatches=0 other-layout mismatches=139
board=SMK bass=A row=2 matched-layout mismatches=0 other-layout mismatches=139
board=SMK bass=A row=3 matched-layout mismatches=0 other-layout mismatches=139
board=SMK bass=B row=2 matched-layout mismatches=0 other-layout mismatches=139
board=SMK bass=B row=3 matched-layout mismatches=0 other-layout mismatches=139
```

## Replay

Run the following block in the source directory produced by the F7 qualifier.

```sh
cat > tests/host/board_caption_probe.c <<'CAPTION_C'
#if defined(PROBE_NATIVE_SMK)
#define SMC_UI_MAIN unused_smc_fixture_main
#include "smc_ui_test.c"
#else
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#define CHECK(test) do { if (!(test)) { fprintf(stderr,"FAIL %s:%d: %s\n",__FILE__,__LINE__,#test); exit(1); } } while(0)
#endif

static unsigned match_pixels(unsigned first_knob, int row)
{
    unsigned mismatches=0;
    cv_begin(240,36,SOUND_SELECT);
    for (int col=0;col<4;col++) {
        pref_t r=pg.r[row][col];
        const s1_patch_descriptor *d=s1_patch_at(r.c);
        s1_patch_value current;
        CHECK(engine_s1_patch_read(r.b,d->factory_id,&current));
        char n[24]={0},u[12]={0},caption[48];
        unit_value(d->unit,s1_patch_display(d,current.physical),n,u);
        snprintf(caption,sizeof caption,"K%u %c %s",first_knob+col,"ADSR"[col],n);
        strip_value(31+59*col,11,caption,u,SOUND_INK);
    }
    for (int y=11;y<23;y++) for(int x=5;x<235;x++)
        mismatches+=cv_px[y*240+x]!=fb[(24+50*row+y)*240+x];
    return mismatches;
}

int main(void)
{
    int32_t sound[512];
    boot();
    now_ms=1000;
    for (unsigned owner=0;owner<2;owner++) {
        CHECK(!engine_bass_select_preset(owner,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound));
        for(unsigned i=0;i<1000 && engine_bass_preset_pending(owner);i++) engine_render(sound,256);
        CHECK(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE);
        ui.part=PART_303A+owner;set_view(V_PART);ui_input_only();
        for (int row=2;row<4;row++) {
            *page_of(V_PART)=row;
            ui.touched=-1;ui.msg_until=0;
            draw_header();draw_main();draw_knobs();
            unsigned right=match_pixels(BOARD_SMK37?5:1,row), wrong=match_pixels(BOARD_SMK37?1:5,row);
            printf("board=%s bass=%c row=%d matched-layout mismatches=%u other-layout mismatches=%u\n",BOARD_SMK37?"SMK":"FM1",'A'+owner,row,right,wrong);
            CHECK(!right && wrong>0);
        }
    }
    return 0;
}
CAPTION_C
SMC_UI_TEST_FILE=tests/host/board_caption_probe.c
X0X_SYNTHONE_PATCH=1
export SMC_UI_TEST_FILE X0X_SYNTHONE_PATCH
X0X_HOST_DEFS='-DX0X_SYNTHONE_PATCH=1 -DBOARD_SMK37=0' \
    SMC_UI_OUT="$PWD/build/host/physical-fm1" sh tests/host/smc_ui_test.sh
X0X_HOST_DEFS='-DX0X_SYNTHONE_PATCH=1 -DPROBE_NATIVE_SMK=1' \
    SMC_UI_OUT="$PWD/build/host/physical-smk" sh tests/host/smc_ui_test.sh
```
