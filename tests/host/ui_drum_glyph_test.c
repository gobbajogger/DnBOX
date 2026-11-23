/* SPDX-License-Identifier: GPL-3.0-only */
/* Drum part-page glyphs: every visible 909/808 control has a glyph role that
 * matches what it does, BRK and other views keep theirs, and a hit animates the
 * cell from the real voice envelope, then the cell goes back to its idle image. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
/* 1ec2d61a8 blends sloped 1 px strokes over at most three minor-axis pixels:
 * a partial-coverage halo may touch the ring just outside the cell, never solid ink. */
#define GLYPH_IN(x,y,px) (((x)>=10 && (x)<36 && (y)>=10 && (y)<=28) || \
    ((x)>=9 && (x)<37 && (y)>=9 && (y)<=29 && (px)!=swap16(C_HI)))
static void drain_ms(int ms) { int32_t a[512]; for (int i = 0; i < ms; i += 6) engine_render(a, 256); audio_due_ms = now_ms; run_ms(ms); }
static int role_of(int p, int k, const char *want)
{
    pref_t r = pg.r[p][k];
    if (r.kind == R_NONE) return 0;
    if (want) { const char *n = matrix_name(r); size_t i = 0;
        for (; want[i] && n[i]; i++) if ((n[i] | 32) != (want[i] | 32)) { fprintf(stderr, "name %s != %s\n", n, want); CHECK(0); }
        if (want[i] || n[i]) fprintf(stderr, "name %s != %s\n", n, want);
        CHECK(!want[i] && !n[i]); }
    return drum_strip_role(r);
}
enum { R22_GLYPH_W=46,R22_GLYPH_H=28 };
static void cell_copy(int k,uint16_t *out)
{   /* Current active R22 page0 path origin, excluding caption/value rows. */
    int cells=0;for(int i=0;i<4;i++)if(pg.r[0][i].kind!=R_NONE)cells++;
    CHECK(cells>=2 && k<cells);
    int gx=6+(228*(2*k+1))/(2*cells)-22;
    for(int y=0;y<R22_GLYPH_H;y++)
        memcpy(out+y*R22_GLYPH_W,fb+(54+y)*240+gx,R22_GLYPH_W*sizeof *out);
}
static uint32_t cell_hash(int k)
{
    uint16_t pixels[R22_GLYPH_W*R22_GLYPH_H];cell_copy(k,pixels);uint32_t h=2166136261u;
    for(unsigned i=0;i<sizeof pixels/sizeof pixels[0];i++)h=(h^pixels[i])*16777619u;
    return h;
}

static void test_dispatch(void)
{
    setup(V_PART, PART_909); ui.sel[PART_909] = 0; *page_of(V_PART) = 0; build_pages();
    CHECK(role_of(0, 0, "Tune") == DG_BDSWEEP); CHECK(role_of(0, 1, "Attack") == DG_ATTACK);
    CHECK(smc_same(pg.r[1][0], PR(R_ENG,T_909,0,2))); CHECK(adsr_row(1));
    CHECK(role_of(1, 0, "Decay") == DG_DECAY);
    ui.sel[PART_909] = 1; build_pages();
    CHECK(role_of(0, 0, "Tune") == DG_TUNE); CHECK(role_of(0, 1, "Tone") == DG_NTAIL);
    CHECK(role_of(0, 2, "Snappy") == DG_SNAPPY);
    CHECK(smc_same(pg.r[1][0], PR(R_ENG,T_909,1,3))); CHECK(adsr_row(1));
    setup(V_PART, PART_808); ui.sel[PART_808] = 0; *page_of(V_PART) = 0; build_pages();
    CHECK(role_of(0, 1, "Tone") == DG_LPF);
    ui.sel[PART_808] = 1; build_pages(); CHECK(role_of(0, 1, "Tone") == DG_SHELLS);
    ui.sel[PART_808] = 2; build_pages(); int found = 0; for (int k = 0; k < 4; k++) if (pg.r[0][k].kind != R_NONE && !strcmp(matrix_name(pg.r[0][k]), "SOUND")) { found = 1; CHECK(drum_strip_role(pg.r[0][k]) == DG_SOUND); } CHECK(found);
    /* every drawn control on every 909/808 voice page has a glyph */
    for (int part = PART_909; part <= PART_808; part++) for (int v = 0; v < 11; v++) {
        setup(V_PART, part); ui.sel[part] = (uint8_t)v; *page_of(V_PART) = 0; build_pages();
        for (int p = 0; p < pg.n; p++) for (int k = 0; k < 4; k++)
            if (pg.r[p][k].kind != R_NONE && strcmp(pg.title[p], "KNOBS 5-8")) CHECK(role_of(p, k, NULL) || orig_strip_slot(pg.r[p][k]));
    }
    setup(V_MIX, PART_909); build_pages();
    CHECK(drum_strip_role(find_ref(T_909,0,"Level")) != DG_NONE);
    CHECK(drum_strip_role(find_ref(T_808,0,"Level")) != DG_NONE);
    setup(V_PART, PART_BRK); build_pages();
    for (int p = 0; p < pg.n; p++) for (int k = 0; k < 4; k++) CHECK(!role_of(p, k, NULL));
    setup(V_FX, PART_909); build_pages();
    for (int p = 0; p < pg.n; p++) for (int k = 0; k < 4; k++) CHECK(!role_of(p, k, NULL));
    puts("PASS dispatch: 909/808 roles by control, BRK and other views untouched");
}
/* 837530d99: the kick AMP graph is the lower full-width panel; other voices keep it in the AMP row. */
enum { AMP_H=54 };
static int AMP_TOP=86;
/* Actual compositor full-row graph, excluding every caption/readout. The
 * ordinary first row is42px, then the active singleton AMP row is56px. */
static void amp_curve_copy(uint16_t *out)
{
    for(int y=AMP_TOP;y<AMP_TOP+AMP_H;y++) memcpy(out+(y-AMP_TOP)*222,fb+y*240+7,222*sizeof *out);
}
static int amp_marker_x(void)
{
    /* the canonical 3x3 live marker (axis ticks and labels are thinner) */
    for(int x=8;x<228;x++)for(int y=AMP_TOP+1;y<AMP_TOP+AMP_H-1;y++){
        int full=1;for(int j=-1;j<=1;j++)for(int i=-1;i<=1;i++)full&=fb[(y+j)*240+x+i]==swap16(SOUND_INK);
        if(full)return x;
    }
    return -1;
}
static void test_amp_motion(int voice)
{
    uint16_t idle[222*AMP_H], first[222*AMP_H], later[222*AMP_H];
    drum909_amp_snapshot a,b;
    AMP_TOP=voice==0?156:80;
    setup(V_PART,PART_909);ui.sel[PART_909]=(uint8_t)voice;build_pages();
    CHECK(smc_same(pg.r[1][0],PR(R_ENG,T_909,voice,voice==0?2:3)));
    CHECK(adsr_row(1));*page_of(V_PART)=1;
    pref_set(pg.r[1][0],64);drain_ms(6000);ui_frame();
    CHECK(engine_drum909_amp_view(voice,&a));CHECK(!a.active);amp_curve_copy(idle);
    engine_drum(PART_909,voice,1.0f);drain_ms(20);now_ms++;ui_frame();
    CHECK(engine_drum909_amp_view(voice,&a));CHECK(a.active);
    CHECK(a.hold_active||a.ramp_valid);CHECK(a.raw_level>0);
    amp_curve_copy(first);CHECK(memcmp(idle,first,sizeof idle));int first_x=amp_marker_x();CHECK(first_x>=0);
    drain_ms(150);now_ms++;ui_frame();
    CHECK(engine_drum909_amp_view(voice,&b));CHECK(b.active&&b.ramp_valid);
    CHECK(b.raw_level<a.raw_level);CHECK(b.ramp_left<b.ramp_ticks);
    amp_curve_copy(later);CHECK(memcmp(first,later,sizeof first));
    /* The live marker follows the real counter: it exists while the voice
     * sounds and moves right as the ramp advances (no marker when idle). */
    CHECK(b.raw_level>=0 && b.raw_level<=1);
    int later_x=amp_marker_x();CHECK(later_x>first_x);
    drain_ms(6000);now_ms++;ui_frame();
    CHECK(engine_drum909_amp_view(voice,&b));CHECK(!b.active);
    amp_curve_copy(later);CHECK(!memcmp(idle,later,sizeof idle));
    printf("PASS 909 voice%d full AMP: hit, actual counter/level motion, idle restoration\n",voice);
}
static void test_motion(int part)
{
    setup(V_PART, part); ui.sel[part] = 0; *page_of(V_PART) = 0; build_pages();
    drain_ms(300); ui_frame(); int decay_cell=part==PART_909?2:3;
    uint16_t idle_pixels[R22_GLYPH_W*R22_GLYPH_H],final_pixels[R22_GLYPH_W*R22_GLYPH_H];cell_copy(decay_cell,idle_pixels);
    uint32_t d0=cell_hash(decay_cell);
    uint32_t a; float e; CHECK(!engine_drum_view(part, 0, &a, &e));
    engine_drum(part, 0, 1.0f); drain_ms(20); now_ms++; ui_frame();
    CHECK(engine_drum_view(part, 0, &a, &e)); float e1 = e; CHECK(e1 > 0.2f);
    uint32_t h1 = cell_hash(part == PART_909 ? 2 : 3); CHECK(h1 != d0);
    drain_ms(150); now_ms++; ui_frame();
    CHECK(engine_drum_view(part, 0, &a, &e)); CHECK(e < e1);                 /* the tail shrinks */
    CHECK(cell_hash(part == PART_909 ? 2 : 3) != h1);                         /* the playhead moves */
    drain_ms(6000); now_ms++; ui_frame();
    CHECK(!engine_drum_view(part, 0, &a, &e)); cell_copy(decay_cell,final_pixels);CHECK(!memcmp(idle_pixels,final_pixels,sizeof idle_pixels));   /* idle again */
    printf("PASS motion %s: hit animates the decay cell, then it rests\n", part == PART_909 ? "909" : "808");
}
static uint32_t direct_drum(pref_t r,int value)
{
 cv_begin(240,KNOB_H,C_BLACK);drum_strip_glyph(r,value,10,10,C_HI,drum_strip_role(r));
 uint32_t h=2166136261u;
 for(int y=0;y<KNOB_H;y++)for(int x=0;x<240;x++)if(cv_px[y*240+x]!=swap16(C_BLACK)){
  if(!GLYPH_IN(x,y,cv_px[y*240+x]))fprintf(stderr,"drum bounds kit=%u voice=%u control=%u value=%d xy=%d,%d\n",r.a,r.b,r.c,value,x,y);
  CHECK(GLYPH_IN(x,y,cv_px[y*240+x]));
 }
 for(int y=10;y<=28;y++)for(int x=10;x<36;x++)h=(h^cv_px[y*240+x])*16777619u;
 return h;
}
/* Pitch motion occupies rows1..5; a two-row blank gutter keeps the lower
 * amplitude waveform readable at both high TUNE values and throughout a hit. */
static uint32_t tune_wave_band(pref_t r,int value)
{
 direct_drum(r,value);
 /* the gutter holds no solid ink (1ec2d61a8 stroke edges may blend into it) */
 for(int y=16;y<=17;y++)for(int x=10;x<36;x++)CHECK(cv_px[y*240+x]!=swap16(C_HI));
 uint32_t h=2166136261u;
 for(int y=18;y<=26;y++)for(int x=10;x<36;x++)h=(h^cv_px[y*240+x])*16777619u;
 return h;
}
static void test_tune_band_separation(void)
{
 const int pots[2]={96,127};
 for(unsigned i=0;i<2;i++){
  setup(V_PART,PART_808);pref_t r=find_ref(T_808,0,"Tune");CHECK(drum_strip_role(r)==DG_TUNE);
  pref_set(r,pots[i]);drain_ms(300);tune_wave_band(r,pots[i]);
  engine_drum(PART_808,0,1.0f);drain_ms(20);
  uint32_t age;float first,late;CHECK(engine_drum_view(PART_808,0,&age,&first));
  uint32_t early=tune_wave_band(r,pots[i]);drain_ms(150);
  CHECK(engine_drum_view(PART_808,0,&age,&late));CHECK(late<first);
  CHECK(tune_wave_band(r,pots[i])!=early); /* Lower band still follows the real EG. */
 }
 puts("PASS TUNE pitch/wave gutter at75/100%, idle and real hit decay");
}

static void test_value_motion_bounds(void)
{
 for(int part=PART_909;part<=PART_808;part++)for(int voice=0;voice<NDRUM;voice++){
  setup(V_PART,part);ui.sel[part]=(uint8_t)voice;build_pages();if(part==PART_909 && voice==0)pref_set(PR(R_ENG,T_909,0,4),64);drain_ms(300);
  for(int p=0;p<pg.n;p++)for(int k=0;k<4;k++){
   pref_t r=pg.r[p][k];int role=drum_strip_role(r);if(!role)continue;
   uint32_t previous=0;const x0x_param_t *d=pref_desc(r);
   for(int v=0;v<5;v++){uint32_t h=direct_drum(r,v*d->max/4);if(v && role!=DG_SOUND){if(h==previous)fprintf(stderr,"drum value collision kit=%u voice=%u ctrl=%u role=%d value=%d\n",r.a,r.b,r.c,role,v*d->max/4);CHECK(h!=previous);}previous=h;}
  }
  engine_drum(part,voice,1.0f);drain_ms(8);
  for(int f=0;f<14;f++){
   for(int p=0;p<pg.n;p++)for(int k=0;k<4;k++){pref_t r=pg.r[p][k];if(drum_strip_role(r))direct_drum(r,matrix_value(r));}
   drain_ms(7);
  }
 }
 for(int part=PART_909;part<=PART_808;part++){
  setup(V_MIX,part);pref_t r=find_ref(part==PART_909?T_909:T_808,0,"Level");CHECK(drum_strip_role(r)==DG_LEVEL);
  int page=-1;for(int p=0;p<pg.n;p++)for(int k=0;k<4;k++)if(pg.r[p][k].kind==R_ENG && pg.r[p][k].a==r.a && pg.r[p][k].b==r.b && pg.r[p][k].c==r.c)page=p;
  CHECK(page>=0);*page_of(V_MIX)=(uint8_t)page;build_pages();drain_ms(300);ui_frame();
  uint32_t idle=direct_drum(r,100),sig=knobs_sig(),strip=2166136261u;
  for(int y=KNOB_Y+28;y<KNOB_Y+52;y++)for(int x=10;x<52;x++)strip=(strip^fb[y*240+x])*16777619u;
  engine_drum(part,0,1.0f);drain_ms(12);ui_frame();CHECK(direct_drum(r,100)!=idle);CHECK(knobs_sig()!=sig);
  uint32_t live_strip=2166136261u;
  for(int y=KNOB_Y+28;y<KNOB_Y+52;y++)for(int x=10;x<52;x++)live_strip=(live_strip^fb[y*240+x])*16777619u;
  CHECK(live_strip!=strip); /* Real LEVEL knob cell must use live dispatch too. */
 }
 puts("PASS drum continuous values, native bounds, real live roles and MIX level cache");
}
static void test_noise_tail_motion(void)
{
    setup(V_PART,PART_909);ui.sel[PART_909]=1;*page_of(V_PART)=0;build_pages();
    CHECK(drum_strip_role(pg.r[0][1])==DG_NTAIL);
    pref_set(pg.r[0][1],127);pref_set(find_ref(T_909,1,"Decay"),127);drain_ms(300);ui_frame();
    uint16_t idle[R22_GLYPH_W*R22_GLYPH_H],final[R22_GLYPH_W*R22_GLYPH_H];cell_copy(1,idle);
    uint32_t old=cell_hash(1),age;float first,late;
    engine_drum(PART_909,1,1.f);drain_ms(20);now_ms++;ui_frame();
    CHECK(engine_drum_view(0,1,&age,&first));CHECK(first>.2f);uint32_t early=cell_hash(1);CHECK(early!=old);
    /* Explicit clock-only diagnostic: no audio rendered, actual909 envelope
     * unchanged, proving that source age/40 drives the noise pattern. */
    now_ms+=40;audio_due_ms=now_ms;ui_frame();
    CHECK(engine_drum_view(0,1,&age,&late));CHECK(late==first);CHECK(cell_hash(1)!=early);
    drain_ms(150);now_ms++;ui_frame();CHECK(engine_drum_view(0,1,&age,&late));CHECK(late<first);
    drain_ms(6000);now_ms++;ui_frame();CHECK(!engine_drum_view(0,1,&age,&late));cell_copy(1,final);CHECK(!memcmp(idle,final,sizeof idle));
    puts("PASS actual Noise Tail, clock-only age-seed change, and byte-identical settled return");
}
static uint32_t pitch_curve_hash(void)
{
    uint32_t hash=2166136261u;
    for(int y=185;y<=208;y++)for(int x=13;x<=227;x++)
        hash=(hash^fb[y*240+x])*16777619u;
    return hash;
}

/* The live pitch marker in the KIT bank's PITCH ENV graph (its band moves with the layout). */
static int kick_live_y(int x){for(int y=180;y<216;y++)if(fb[y*240+x]==swap16(SOUND_INK))return y;return -1;}
static void test_kick_pitch_row(void)
{
    setup(V_PART,PART_909);ui.sel[PART_909]=DR_BD;*page_of(V_PART)=0;build_pages();
    /* The full-width kick pitch graph belongs to the KIT bank (837530d99: the
     * approved first bank shows the AMP envelope). */
    int kit_page=-1;for(int p=0;p<pg.n;p++)if(pg.group[p]==GR_KIT){kit_page=p;break;}
    CHECK(kit_page>=0);*page_of(V_PART)=kit_page;build_pages();
    pref_t depth=PR(R_ENG,T_909,DR_BD,4);
    pref_set(depth,0);drain_ms(6100);ui_frame();
    uint32_t shallow=pitch_curve_hash();
    pref_set(depth,127);drain_ms(30);ui_frame();CHECK(shallow!=pitch_curve_hash());
    float ms,hz;
    CHECK(!engine_bd909_pitch_view(&ms,&hz));
    engine_drum(PART_909,DR_BD,1);drain_ms(10);ui_frame();
    CHECK(engine_bd909_pitch_view(&ms,&hz));CHECK(ms>0 && ms<40 && hz>49);
    int x=13+(int)(214.f*ms/160.f+.5f);
    int y=kick_live_y(x);CHECK(y>=0);
    uint32_t early=pitch_curve_hash();
    float old_ms=ms,old_hz=hz;
    pref_set(depth,0); /* Preview changes; running voice retains its trigger. */
    CHECK(engine_bd909_pitch_view(&ms,&hz));CHECK(ms==old_ms && hz==old_hz);
    drain_ms(50);ui_frame();CHECK(engine_bd909_pitch_view(&ms,&hz));
    CHECK(ms>40 && ms<160 && hz<old_hz);CHECK(early!=pitch_curve_hash());
    x=13+(int)(214.f*ms/160.f+.5f);int y2=kick_live_y(x);
    CHECK(y2>y);                                    /* lower live pitch: the marker sits lower */
    shot("kick-pitch-live");
    drain_ms(6100);ui_frame();CHECK(!engine_bd909_pitch_view(&ms,&hz));
    CHECK(shallow==pitch_curve_hash());shot("kick-pitch-idle");
    puts("PASS full-width 909 kick pitch sweep, live oscillator, independent next-hit edit");
}
static void test_secondary_decay_rows(void)
{
    engine_decay_view_t v;
    CHECK(!engine_drum909_decay_view(DR_RS,&v));
    for(int voice=DR_LT;voice<=DR_RD;voice++) {
        if(voice==DR_RS)continue;
        setup(V_PART,PART_909);ui.sel[PART_909]=voice;*page_of(V_PART)=0;build_pages();
        int pot=voice<=DR_HT?1:(voice==DR_CH || voice==DR_OH)?0:2;
        pref_t r=PR(R_ENG,T_909,voice,pot);
        CHECK(!strcmp_s(pref_name(r),voice==DR_CP?"Tail":"Decay"));
        pref_set(r,127);drain_ms(6100);ui_frame();
        CHECK(engine_drum909_decay_view(voice,&v));CHECK(v.elapsed_ms<0);
        uint32_t long_curve=2166136261u;
        for(int y=153;y<=194;y++)for(int x=33;x<=227;x++)
            long_curve=(long_curve^fb[y*240+x])*16777619u;
        float max_ms=v.decay_ms;
        engine_drum(PART_909,voice,1);drain_ms(12);ui_frame();
        CHECK(engine_drum909_decay_view(voice,&v));
        if(voice==DR_CP)CHECK(v.elapsed_ms<0); /* fourth pulse starts the editable tail */
        else CHECK(v.elapsed_ms>=0);
        drain_ms(40);ui_frame();
        CHECK(engine_drum909_decay_view(voice,&v));CHECK(v.elapsed_ms>0 && v.level>0);
        float t=v.elapsed_ms,level=v.level;
        float expected=v.peak*fm_powf(.00001f/v.peak,t/max_ms);
        CHECK(fabsf(v.level-expected)<.002f);
        CHECK(max_ms>0 && t>=0 && t<=max_ms && level>=0 && level<=1);
        int x=33+(int)(194.f*t/max_ms+.5f);
        int top=153,bottom=194;
        int y=bottom-(int)((bottom-top)*level+.5f);
        CHECK(x>=33 && x<=227 && y>=top && y<=bottom);
        if(fb[y*240+x]!=swap16(SOUND_INK))fprintf(stderr,"909_LIVE_MARKER_MISSING voice=%d x=%d y=%d\n",voice,x,y);
        CHECK(fb[y*240+x]==swap16(SOUND_INK));
        pref_set(r,0);drain_ms(5);ui_frame();
        CHECK(engine_drum909_decay_view(voice,&v));CHECK(v.elapsed_ms>t && v.level<=level);
        CHECK(v.decay_ms<max_ms);
        if(voice==DR_OH) {
            engine_drum(PART_909,DR_CH,1);drain_ms(12);
            CHECK(engine_drum909_decay_view(voice,&v));CHECK(v.elapsed_ms<0);
        }
        drain_ms(6100);ui_frame();CHECK(engine_drum909_decay_view(voice,&v));CHECK(v.elapsed_ms<0);
        uint32_t short_curve=2166136261u;
        for(int y=153;y<=194;y++)for(int x=33;x<=227;x++)
            short_curve=(short_curve^fb[y*240+x])*16777619u;
        CHECK(long_curve!=short_curve);
    }
    puts("PASS eight editable 909 decay graphs using production ramp samples");
}
static void test_snare_noise_row(void)
{
    engine_decay_view_t v;
    setup(V_PART,PART_909);ui.sel[PART_909]=DR_SD;*page_of(V_PART)=0;build_pages();
    pref_t tone=PR(R_ENG,T_909,DR_SD,1);
    pref_set(tone,127);drain_ms(6100);ui_frame();
    CHECK(engine_drum909_decay_view(DR_SD,&v));CHECK(v.elapsed_ms<0);
    float max_ms=v.decay_ms+24.f;
    engine_drum(PART_909,DR_SD,1);drain_ms(4);ui_frame();
    CHECK(engine_drum909_decay_view(DR_SD,&v));CHECK(v.elapsed_ms>0 && v.elapsed_ms<24 && v.level==1);
    int x=13+(int)(214.f*v.elapsed_ms/max_ms+.5f);
    CHECK(fb[185*240+x]==swap16(SOUND_INK));
    drain_ms(50);ui_frame();CHECK(engine_drum909_decay_view(DR_SD,&v));
    CHECK(v.elapsed_ms>24 && v.level<1 && v.level>0);
    CHECK(fabsf(v.level-fm_powf(.00001f,(v.elapsed_ms-24.f)/v.decay_ms))<.001f);
    int y=208-(int)(23.f*v.level+.5f);
    x=13+(int)(214.f*v.elapsed_ms/max_ms+.5f);CHECK(fb[y*240+x]==swap16(SOUND_INK));
    pref_set(tone,0);drain_ms(5);ui_frame();CHECK(engine_drum909_decay_view(DR_SD,&v));
    CHECK(v.elapsed_ms>60 && v.decay_ms+24.f<max_ms);
    drain_ms(6100);ui_frame();CHECK(engine_drum909_decay_view(DR_SD,&v));CHECK(v.elapsed_ms<0);
    puts("PASS 909 snare noise hold and actual tone decay");
}
static void test_808_circuit_curves(void)
{
    const int tracks[]={D8_SD,D8_RS,D8_CP,D8_CP,D8_CB,D8_CY,D8_OH,D8_CH};
    drum_curve_t view;
    setup(V_PART,PART_808);
    CHECK(!engine_drum808_curve(D8_BD,&view));CHECK(!engine_drum808_curve(D8_MT,&view));
    for(unsigned i=0;i<sizeof tracks/sizeof *tracks;i++){
        int voice=tracks[i];
        setup(V_PART,PART_808);ui.sel[PART_808]=voice;*page_of(V_PART)=0;build_pages();
        if(voice==D8_CP || voice==D8_RS)pref_set(find_ref(T_808,voice,"Sound"),i==3);
        pref_t decay=find_ref(T_808,voice,"Decay");
        pref_set(decay,127);drain_ms(6200);ui_frame();
        CHECK(engine_drum808_curve(voice,&view));CHECK(view.elapsed<0);
        float long_fall=view.fall;
        engine_drum(PART_808,voice,1);
        drain_ms(4);ui_frame();
        CHECK(engine_drum808_curve(voice,&view));CHECK(view.elapsed>0);
        CHECK(fabsf(view.level-dv_curve_level(&view,view.elapsed))<.04f);
        drain_ms(26);ui_frame();
        CHECK(engine_drum808_curve(voice,&view));CHECK(view.elapsed>0);
        float expected=dv_curve_level(&view,view.elapsed);
        CHECK(fabsf(view.level-expected)<.04f);
        /* This fixed two-bank fixture has plot y153..194 after reserving
         * the native FONT_T time band. The percentage scale owns x13..29;
         * real time runs x33..227, not the retired x13..227 unlabelled plot. */
        CHECK(view.window>0 && view.elapsed>=0 && view.elapsed<=view.window);
        CHECK(view.level>=0 && view.level<=1);
        int x=33+(int)(194.f*view.elapsed/view.window+.5f);
        int y=194-(int)(41.f*view.level+.5f);
        if(voice==D8_CP) {
            /* Stage-only CP and combined MA target rows keep actual level
             * separate from an edited-target horizontal time coordinate. */
            drum_curve_t bound;
            CHECK(engine_drum808_param_curve(D8_CP,2,&bound)==(i==3?D8S_MA:D8S_CP)+1);
            CHECK(bound.elapsed>=0 && bound.level==view.level);
            x=230;y=164-(int)((i==3?48.f:20.f)*bound.level+.5f);
            CHECK(y>=(i==3?116:144) && y<=164);
        } else CHECK(x>=33 && x<=227 && y>=153 && y<=194);
        if(fb[y*240+x]!=swap16(SOUND_INK))fprintf(stderr,"808_LIVE_MARKER_MISSING voice=%d x=%d y=%d elapsed=%f level=%f\n",voice,x,y,view.elapsed,view.level);
        CHECK(fb[y*240+x]==swap16(SOUND_INK));
        char shot_name[48];snprintf(shot_name,sizeof shot_name,"808-circuit-live-%u-voice%d",i,voice);shot(shot_name);
        float age=view.elapsed;
        pref_set(decay,0);drain_ms(4);ui_frame();CHECK(engine_drum808_curve(voice,&view));
        CHECK(view.fall<long_fall);CHECK(view.elapsed>age);
        if(voice==D8_RS){
            pref_set(find_ref(T_808,voice,"Sound"),1);drain_ms(5);
            CHECK(!engine_drum808_curve(voice,&view));
        }
    }
    puts("PASS eight editable 808 circuit envelopes, switch isolation and real audio-frame cursor");
}
static void test_shared_return_fade(void)
{
    pref_t r=PR(R_ENG,T_909,1,1);uint32_t saved_now=now_ms,saved_last=dg_last[0][1];
    now_ms=100000;CHECK(dg_live_back(r,1)==1.f);now_ms+=100;float a=dg_live_back(r,0);CHECK(a>0.32f&&a<0.34f);
    now_ms+=100;float b=dg_live_back(r,0);CHECK(b>a&&b<1.f);now_ms+=101;CHECK(dg_live_back(r,0)==1.f);
    now_ms=saved_now;dg_last[0][1]=saved_last;
    puts("PASS shared300ms return-fade clock contract");
}
int main(void) { test_dispatch(); test_808_circuit_curves(); test_secondary_decay_rows(); test_snare_noise_row(); test_kick_pitch_row(); test_amp_motion(0); test_amp_motion(1); test_motion(PART_808); test_tune_band_separation(); test_value_motion_bounds();test_noise_tail_motion();test_shared_return_fade(); printf("PASS ui-drum-glyph (%u checks)\n", checks); return 0; }
