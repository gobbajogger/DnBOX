#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Actual walker plus source-derived renderer invocation receipts; fail closed."""
import argparse,hashlib,importlib.util,json,os,shutil,subprocess,sys
from pathlib import Path
from PIL import Image
from source_inventory import inventory
BRANCHES=('firmware/src/app/ui.c::draw_adsr_inline::active-playhead',
          'firmware/src/app/smc_mixer.c::smc_ui_draw_peer::pickup-direction',
          'firmware/src/app/smc_mixer.c::smc_ui_draw::peek-pickup',
          'firmware/src/app/ui.c::draw_header::pickup-context')

def frame_method(name):
    if name.startswith('clip-grid-'):return 'NATIVE_GRID_PAINTER_WITH_FIXTURE_RAM_AND_PRODUCTION_HEADER_FOOTER'
    if name.startswith('clip-keys-'):return 'NATIVE_CK_PAGE_AND_PANEL_WITH_FIXTURE_RAM_AND_PRODUCTION_HEADER'
    if name.startswith('padcal-'):return 'NATIVE_PADCAL_PAINTER_WITH_FIXTURE_RAM_AND_PRODUCTION_HEADER_FOOTER'
    if name.startswith(('boot-default','boot-custom-')):return 'COMPLETE_BOOT_DISPLAY_RENDERER_WITH_HOST_LCD_ORACLE'
    if name.startswith('boot-update-'):return 'COMPLETE_OTA_DISPLAY_RENDERER_WITH_NOOP_DELAY_ONLY'
    if name.startswith('boot-'):return 'LITERAL_SOURCE_DISPLAY_PREFIX_NATIVE_HARDWARE_TAIL_NOT_RUN'
    if name.startswith('hardware-calibration-'):return 'LITERAL_SOURCE_DISPLAY_STATEMENTS_MATRIX_INPUT_AND_STORE_NOT_RUN'
    if name.startswith('preview-part'):return 'DIRECT_PREVIEW_PAINTER_COMPOSITION_NOT_PHYSICAL_UI_FRAME'
    if name.startswith(('live-part','window-part')):return 'COMPLETE_UI_FRAME_WITH_PHYSICAL_LANE_INPUT_FIXTURE'
    return 'COMPLETE_UI_FRAME_WITH_EXPLICIT_HOST_STATE_OR_INPUT_FIXTURE'

def hook_once(text,old,new):
    if text.count(old)!=1:raise ValueError('nonunique instrumentation hook: '+old[:100])
    return text.replace(old,new,1)

def exact_function(text,signature):
    from source_inventory import masked
    if text.count(signature)!=1:raise ValueError('nonunique native function '+signature)
    start=text.index(signature);clean=masked(text);opening=clean.index('{',start);depth=1;end=opening+1
    while depth:
        depth+=(clean[end]=='{')-(clean[end]=='}');end+=1
    return text[start:end]+'\n'

def verify_tree(source):
    reference=source/'production-tree.json'
    if not reference.is_file():return {'status':'NOT_SUPPLIED'}
    tree=json.loads(reference.read_text());expected={row['path']:row['sha1'] for row in tree['files']}
    for name,sha in expected.items():
        path=source/name;data=os.readlink(path).encode() if path.is_symlink() else path.read_bytes()
        if hashlib.sha1(b'blob '+str(len(data)).encode()+b'\0'+data).hexdigest()!=sha:raise ValueError('production tree mismatch: '+name)
    for path in (source/'firmware').rglob('*'):
        if path.is_file() and path.relative_to(source).as_posix() not in expected:raise ValueError('extra firmware source outside frozen tree')
    return {'status':'VERIFIED','tree':tree['tree'],'files':len(expected),'referenceSHA256':hashlib.sha256(reference.read_bytes()).hexdigest()}

def prepare(work,manifest):
    original_main=(work/'firmware/src/app/main_fm1.c').read_text()
    rows=manifest['renderers']; grouped={}
    for index,row in enumerate(rows):grouped.setdefault(row['path'],[]).append((index,row))
    for path,items in grouped.items():
        # Native painters are extracted by the established source-only fixtures;
        # instrument their exact bodies, never the substituted host painters.
        if path=='firmware/src/gfx.c':continue # geometry hooks authenticate original primitive bodies first
        p=work/path;s=p.read_text()
        for index,row in sorted(items,key=lambda item:item[1]['opening'],reverse=True):
            offset=row['opening']+1;s=s[:offset]+f' f1_seen[{index}]=1; '+s[offset:]
        p.write_text(s)
    p=work/'firmware/src/gfx.c';s=p.read_text()
    prefix='static unsigned char f1_seen[%d],f1_branch[%d];\n'%(len(rows),len(BRANCHES))
    prefix+='static void f1_reset(void){memset(f1_seen,0,sizeof f1_seen);memset(f1_branch,0,sizeof f1_branch);}\n'
    prefix+='static void f1_dump(const char *name, unsigned capture){printf("F1_FRAME {\\"id\\":\\"%s\\",\\"capture\\":%u,\\"renderers\\":[",name,capture);int comma=0;for(unsigned i=0;i<sizeof f1_seen;i++)if(f1_seen[i]){printf("%s%u",comma?",":"",i);comma=1;}printf("],\\"branches\\":[");comma=0;for(unsigned i=0;i<sizeof f1_branch;i++)if(f1_branch[i]){printf("%s%u",comma?",":"",i);comma=1;}puts("]}");}\n'
    # Gfx precedes ui.c in both production and host translation units.
    pos=s.index('\nstatic ');s=s[:pos]+'\n'+prefix+s[pos:]
    p.write_text(s)
    p=work/'firmware/src/app/ui.c';s=p.read_text()
    s=hook_once(s,'UI_DRAW void ui_frame(void)\n{','UI_DRAW void ui_frame(void)\n{\n f1_reset();')
    s=hook_once(s,'if(observed && st!=6) {','if(observed && st!=6) { f1_branch[0]=1;')
    s=hook_once(s,'char detail[12];header_pickup_detail(detail,pending);','f1_branch[3]=1;char detail[12];header_pickup_detail(detail,pending);')
    p.write_text(s)
    p=work/'firmware/src/app/smc_mixer.c';s=p.read_text()
    old='status_symbol=smc.last[smc_ui.strip]<smc_scaled(r)?UI_G_UP:UI_G_DOWN;'
    s=hook_once(s,old,'status_symbol=(f1_branch[1]=1,smc.last[smc_ui.strip]<smc_scaled(r)?UI_G_UP:UI_G_DOWN);')
    s=hook_once(s,'sc = C_HI;','sc = C_HI;f1_branch[2]=1;');p.write_text(s)
    p=work/'tests/host/smc_ui_test.c';s=p.read_text()
    lane_available='static uint8_t ui_step_lane;' in (work/'firmware/src/app/ui.c').read_text()
    s=hook_once(s,'snprintf(path, sizeof path, "%s/%s.png", dir, name); png_write(path);',
       '''snprintf(path, sizeof path, "%s/%s.png", dir, name); png_write(path);
    static unsigned f1_capture;
    snprintf(path,sizeof path,"%s/f1-%04u-%s.png",dir,f1_capture,name);png_write(path);
    f1_dump(name,f1_capture++);
    printf("F1_STATE {\\"capture\\":%u,\\"view\\":%u,\\"part\\":%u,\\"voice\\":%u,\\"page\\":%d,\\"overlay\\":%u,\\"help\\":%u,\\"lane\\":%d,\\"battery\\":%d,\\"playing\\":%u,\\"smcKind\\":%u,\\"smcPeek\\":%u}\\n",f1_capture-1,ui.view,ui.part,ui.part<NKIT?ui.sel[ui.part]:0,cur_page(),ui.overlay,ui.help,F1_CURRENT_LANE,plat_battery(),seq.playing,smc_ui.kind,smc_peek->on);
''')
    s=s.replace('F1_CURRENT_LANE','ui_step_lane' if lane_available else '-1');p.write_text(s)
    p=work/'tests/ui_geometry/capture.c';s=p.read_text()
    native=[]
    main=(work/'firmware/src/app/main_fm1.c').read_text()
    for symbol,filename in [('smk_clip_lcd_draw','actual_clip_grid.inc'),('smk_padcal_draw','actual_padcal.inc')]:
        from source_inventory import functions
        row=next(r for r in functions('main_fm1.c',main) if r['symbol']==symbol)
        anchor='uint8_t '+symbol+'(uint32_t w, uint32_t h)'
        text=main[main.index(anchor):row['end']]+'\n'
        (work/'tests/ui_geometry'/filename).write_text(text)
    for stem in ('ui_clip_grid_render_test','ui_padcal_render_test'):
        text=(work/'tests/host'/(stem+'.c')).read_text()
        marker='#undef smk_clip_lcd_draw' if 'grid' in stem else '#undef smk_padcal_draw'
        text=text[text.index(marker)+len(marker):]
        if 'padcal' in stem:text=text.replace('#include "app/apc40_lcd565.h"','')
        text=text.replace('int main(void)','int f1_'+stem+'(void)')
        text=text.replace('static void render_grid(const char *name)\n{','static void render_grid(const char *name)\n{\n f1_reset();')
        text=text.replace('static void render_state(const char *capture)\n{','static void render_state(const char *capture)\n{\n f1_reset();')
        native.append(text)
    # The CK hardware/MIDI fixture has a different state type. Extract only the
    # real pure state/formatting/keybed painters into a test namespace, retaining
    # the exact function bodies. Hardware registers and MIDI dispatch are absent.
    ck_start=main.index('typedef struct {',main.index('/* --------------------------------------------- clip mode keybed'))
    ck_end=main.index('static ck_t ck SMK_BIG;',ck_start)+len('static ck_t ck SMK_BIG;')
    ck_types=main[ck_start:ck_end].replace(' SMK_BIG','')
    ck_tables=main[main.index('static const char CK_MODE_N[]'):main.index('/* Set field f to v.')]
    ck_screens=main[main.index('/* ------- clip mode screen:'):main.index('/* every main loop pass',main.index('/* ------- clip mode screen:'))]
    ck_screens=ck_screens.replace(exact_function(ck_screens,'uint8_t smk_ck_ext(void)'),'')
    ck_constants=main[main.index('enum { CK_F_LATCH'):main.index('/* HUMAN timing:',main.index('enum { CK_F_LATCH'))]
    native.append('''
#define ck f1_native_ck
#define ck_get f1_native_ck_get
#define ck_val_txt f1_native_ck_val_txt
#define ck_page_draw f1_native_ck_page_draw
#define smk_ck_panel f1_native_ck_panel
#define CK_FLD_N F1_NATIVE_CK_FLD_N
#define CK_BASE ck_t *cp=&ck
#define CK_SET settings_t *sp=&proj.set
'''+ck_types+'\n'+ck_constants+'\n'+exact_function(main,'CK_FN static uint32_t ck_key_notes(uint32_t k, uint8_t *out)')+'\n'+ck_tables+'\n'+exact_function(main,'CK_FN static const char *ck_val_txt(uint32_t f, uint32_t v, char *buf)')+'\n'+ck_screens+'''
static void f1_native_clip_keys(void) {
 setup(V_SEQ,PART_909);clip_grid_mode=1;memset(&ck,0,sizeof ck);
 for(unsigned page=1;page<=3;page++)for(unsigned field=0;field<(page==3?9u:4u);field++) {
  ck.pg=page|(CK_PAGE_FIELDS[page-1][field]<<4);ck_t saved=ck;
  f1_reset();draw_header();
  cv_begin(240,2*BAND_H,C_BLACK);cv_band(0,2*BAND_H);ck_page_draw();cv_commit_rect(1,0,MAIN_Y);
  uint32_t sig=0;CHECK(smk_ck_panel(&sig,1));cv_commit_rect(2,0,KNOB_Y);
  CHECK(!memcmp(&saved,&ck,sizeof ck));
  char name[80];snprintf(name,sizeof name,"clip-keys-page-%u-field-%u",page,field);shot(name);
 }
}
#undef ck
#undef ck_get
#undef ck_val_txt
#undef ck_page_draw
#undef smk_ck_panel
#undef CK_FLD_N
#undef CK_BASE
#undef CK_SET
''')
    # One-shot boot/maintenance display code. Hardware tails (reboot, updater,
    # storage and calibration writes) are never executed by these display slices.
    irq=(work/'firmware/hal/fm1_irq.h').read_text()
    a=irq.index('typedef struct {',irq.index('#define FM1_CRASH_MAGIC'));b=irq.index('} fm1_crash_t;',a)+len('} fm1_crash_t;')
    boot_type=irq[a:b]
    boot=[]
    slices=[]
    for signature,stop in [('static void fm1_fault(const fm1_crash_t *c)','    t0 = fm1_ticks();'),
                           ('static void enter_uboot(const char *why)','    fm1_delay_ms(20);'),
                           ('static void safe_main(void)','    fm1_input_init();')]:
        body=exact_function(main,signature);body=body[:body.index(stop)]+'}\n'
        # A display-only slice must not claim invocation of the whole native
        # wrapper (which also contains hardware/storage/control-flow tails).
        import re
        body=re.sub(r' f1_seen\[\d+\]=1; ',' ',body)
        boot.append(body)
        original=exact_function(original_main,signature);original=original[:original.index(stop)]
        slices.append(dict(source='firmware/src/app/main_fm1.c',signature=signature,stopBefore=stop.strip(),
                           sha256=hashlib.sha256(original.encode()).hexdigest(),method='EXACT_DISPLAY_PREFIX; hardware/control-flow tail NOT_RUN'))
    # Repaint transitions over their real predecessor so unchanged pixels and
    # previously drawn labels remain part of the framebuffer.
    retry_start = '            draw_text_box(0, 172, 240, &FONT_S, "STARTING DnBOX...'
    notice_start = "        b[0] = 'P';"
    notice_guard = 'fm1_crash.count != x0x_dbg.crash_seen'
    def display_statements(source, start, end, guard):
        begin = source.index(start, source.index(guard))
        return source[begin:source.index(end, begin)]
    retry = display_statements(original_main, retry_start, '            usb_detach();', 'static void safe_main(void)')
    notice = display_statements(original_main, notice_start, '        fm1_wdt_feed();', notice_guard)
    for signature, statements in [('static void safe_main(void)', retry), ('fm1_main post-reset crash notice', notice)]:
        slices.append(dict(source='firmware/src/app/main_fm1.c', signature=signature,
                           sha256=hashlib.sha256(statements.encode()).hexdigest(),
                           method='EXACT_DISPLAY_STATEMENTS_OVER_REAL_PREDECESSOR; native reset and hardware tail NOT_RUN'))
    x0x=(work/'firmware/src/x0x.c').read_text()
    ota=exact_function(x0x,'static void ota_show(uint32_t step, int32_t code)')
    panel=(work/'firmware/src/app/panel.c').read_text()
    names=panel[panel.index('static const char *const B_NAME'):panel.index('#if BOARD_SMK37')]
    title=panel[panel.index('    lcd_fill(0, 0, 240, 240, C_BLACK);',panel.index('static void panel_setup')):panel.index('    while (fm1_in.buttons)',panel.index('static void panel_setup'))]
    button=panel[panel.index('        draw_text_box(0, 80, 240, &FONT_S, "PRESS"'):panel.index('        t0 = fm1_ms;',panel.index('        draw_text_box(0, 80, 240, &FONT_S, "PRESS"'))]
    encoder=panel[panel.index('        draw_text_box(0, 80, 240, &FONT_S, "TURN RIGHT"'):panel.index('        for (e = 0;',panel.index('        draw_text_box(0, 80, 240, &FONT_S, "TURN RIGHT"'))]
    slices.append(dict(source='firmware/src/app/panel.c',signature='static void panel_setup(void)',
                       sha256=hashlib.sha256((names+title+button+encoder).encode()).hexdigest(),method='EXACT_DISPLAY_STATEMENTS; real matrix input and persistent calibration NOT_RUN'))
    (work/'tests/ui_geometry/native-display-slices.json').write_text(json.dumps(slices,indent=2)+'\n')
    native.append('''
static void f1_boot_audio_stop(void) {}
static void f1_boot_delay(uint32_t ms) {(void)ms;}
#define fm1_audio_stop f1_boot_audio_stop
#define fm1_delay_ms f1_boot_delay
#define FM1_CRASH_MAGIC 0x43525348u
'''+boot_type+'\nstatic fm1_crash_t fm1_crash;\n'+exact_function(main,'static void hexs(char *b, uint32_t v)')+'\n'+'\n'.join(boot)+'\n'+ota+'''
#undef fm1_audio_stop
#undef fm1_delay_ms
static void f1_boot_displays(void) {
 f1_reset();draw_boot_screen(0);shot("boot-default");
 uint8_t custom[BOOT_REC_MAX]={BOOT_KIND_CUSTOM,0,5,5};
 memcpy(custom+4,"DnBOX",5);memcpy(custom+16,"STILL",5);
 f1_reset();draw_boot_screen(custom);shot("boot-custom-text");
 custom[1]=BOOT_F_PIC;boot_p16(custom+40,240);boot_p16(custom+42,96);custom[BOOT_PAY_FIXED]=0x80;
 CHECK(boot_payload_ok(custom,sizeof custom));f1_reset();draw_boot_screen(custom);shot("boot-custom-picture");
 CHECK(fb[20*240]==swap16(C_HI));
 memset(&fm1_crash,0,sizeof fm1_crash);f1_reset();safe_main();shot("boot-safe-mode");
 fm1_crash.magic=FM1_CRASH_MAGIC;fm1_crash.pc=0x12345678;fm1_crash.vec=1;fm1_crash.dbg=0x1000;
 for(int core=0;core<2;core++) {fm1_crash.core=core;f1_reset();safe_main();char name[64];snprintf(name,sizeof name,"boot-safe-crash-core-%d",core);shot(name);}
 f1_reset();safe_main();
'''+retry+'''
 shot("boot-safe-retry");
 for(int core=0;core<2;core++) {
  char b[16],name[64];fm1_crash.core=core;f1_reset();draw_boot_screen(0);
'''+notice+'''
  snprintf(name,sizeof name,"boot-post-crash-core-%d",core);shot(name);
 }
 f1_reset();fm1_fault(&fm1_crash);shot("boot-crash-diagnostics");
 f1_reset();enter_uboot("UBOOT");shot("boot-uboot");
 f1_reset();enter_uboot("UBOOT (USB)");shot("boot-uboot-usb");
 for(unsigned step=0;step<6;step++) {f1_reset();ota_show(step,0);char name[64];snprintf(name,sizeof name,"boot-update-step-%u",step);shot(name);}
 f1_reset();ota_show(9,1);shot("boot-update-dry-run");f1_reset();ota_show(9,0);shot("boot-update-failed");
}
'''+names+'''
static void f1_panel_displays(void) {
 for(unsigned i=0;i<NB;i++) {f1_reset();
'''+title+button+'''
 char name[80];snprintf(name,sizeof name,"hardware-calibration-button-%02u",i);shot(name);}
 for(unsigned i=0;i<NE;i++) {f1_reset();
'''+title+encoder+'''
 char name[80];snprintf(name,sizeof name,"hardware-calibration-encoder-%02u",i);shot(name);}
}
''')
    lane=work/'tests/host/ui_step_lane_preview_test.c'
    if lane.is_file():
        text=lane.read_text();text=text[text.index('#include "smc_ui_test.c"')+len('#include "smc_ui_test.c"'):]
        text=text.replace('int main(void)','int f1_step_lane_main(void)');native.append(text)
    s=hook_once(s,'#include "../host/smc_ui_test.c"',
       '#define smk_clip_lcd_draw f1_substitute_clip_draw\n#define smk_padcal_draw f1_substitute_padcal_draw\n#include "../host/smc_ui_test.c"\n#undef smk_clip_lcd_draw\n#undef smk_padcal_draw\n'+'\n'.join(native))
    # Reuse the authentic controller/pickup walker and its assertions. Additional
    # fixture shots have their original names and renderer identity receipts.
    s=hook_once(s,'    printf("INVENTORY %u ordinary/global/list/engine/help/modal states\\n",total);',
      '''    retained_fixture_main();
    geo_enabled=0;f1_ui_clip_grid_render_test();f1_ui_padcal_render_test();f1_native_clip_keys();f1_boot_displays();f1_panel_displays();
    F1_LANE_CALL
    setup(V_HOME,PART_909);ui.touched=4;ui.touch_until=plat_ms()+1000;
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();shot("readout-tempo");
    setup(V_HOME,PART_909);clip_grid_mode=clip_waiting=1;
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();shot("clip-waiting-for-host");
    setup(V_FX,PART_909);smc.view=V_FX;smc_ui_begin(2,0);smc_ui.kind=3;smc_ui.action=SMC_FILL_NOTE;
    CHECK(smc_fx_bus_now(SMC_FILL_NOTE)>=0);
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();shot("smc-fill-request");
    setup(V_HOME,PART_909);smc_view(V_MIX,0);pref_t peek_ref=smc_target(0);CHECK(peek_ref.kind!=R_NONE);pref_set(peek_ref,64);
    message(0x90,0x5D,127);message(0x80,0x5D,0);CHECK(smc.locked&&smc_peek->on&&ui.view==V_MIX);fader(0,0);
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();shot("smc-explicit-peek-pickup");
    setup(V_HOME,PART_909);say("PICK F1 UP",0);
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();shot("header-physical-pickup");
    for(unsigned bank=0;bank<4;bank++) {
      setup(V_PART,PART_303A);CHECK(!engine_bass_select_preset(0,bank,0,&proj.sound));
      for(int block=0;block<24;block++){int32_t audio[512];engine_render(audio,256);}
      CHECK(!engine_bass_preset_pending(0));build_pages();
      engine_bass_on_velocity(0,60,0,0,100);
      for(int block=0;block<4;block++){int32_t audio[512];engine_render(audio,256);}
      for(int page=0;page<pg.n;page++)if(adsr_row(page)) {
        *page_of(V_PART)=(uint8_t)page;build_pages();
        memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
        char name[80];snprintf(name,sizeof name,"live-engine-%u-page-%d",bank,page);shot(name);
      }
    }
    for(int level=0;level<4;level++) {
      setup(V_HOME,PART_909);host_battery_level=level;
      memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
      char id[64];snprintf(id,sizeof id,"battery-level-%d",level);shot(id);
    }
    host_battery_level=3;
    setup(V_HOME,PART_909);ui.home_live=1;
    memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();shot("home-live");
    printf("INVENTORY %u ordinary/global/list/engine/help/modal states\\n",total);''')
    s=s.replace('    F1_LANE_CALL','    f1_step_lane_main();' if lane.is_file() else '')
    p.write_text(s)

def run(source,out):
    source=source.resolve();out=out.resolve()
    if out.exists() or out.is_relative_to(source):raise ValueError('new output outside source required')
    out.mkdir(parents=True);tree_receipt=verify_tree(source);manifest=inventory(source)
    (out/'production-tree-receipt.json').write_text(json.dumps(tree_receipt,indent=2)+'\n')
    runner=out/'runner-inputs';runner.mkdir();runner_hashes={}
    for path in sorted(Path(__file__).parent.iterdir()):
        if not path.is_file() or path.suffix not in ('.py','.md'):continue
        shutil.copyfile(path,runner/path.name);runner_hashes[path.name]=hashlib.sha256(path.read_bytes()).hexdigest()
    (out/'runner-inputs.json').write_text(json.dumps(runner_hashes,indent=2)+'\n')
    closure={}
    for directory in ('firmware','host','tests','tools','experiments'):
        for path in sorted((source/directory).rglob('*')):
            if not path.is_file() or '__pycache__' in path.parts:continue
            closure[path.relative_to(source).as_posix()]=hashlib.sha256(path.read_bytes()).hexdigest()
    (out/'source-closure.json').write_text(json.dumps(closure,indent=2)+'\n')
    (out/'source-inventory.json').write_text(json.dumps(manifest,indent=2)+'\n')
    work=out/'source';shutil.copytree(source,work,symlinks=True,ignore=shutil.ignore_patterns('.git','build','__pycache__','node_modules','.output'))
    prepare(work,manifest)
    sys.path.insert(0,str(work/'tests/ui_geometry'))
    from instrument import instrument
    instrument(work)
    shutil.copyfile(work/'tests/ui_geometry/native-display-slices.json',out/'native-display-slices.json')
    from source_inventory import functions
    p=work/'firmware/src/gfx.c';text=p.read_text();positions=[]
    parsed=functions('firmware/src/gfx.c',text)
    for index,row in enumerate(manifest['renderers']):
        if row['path']!='firmware/src/gfx.c':continue
        matches=[r for r in parsed if r['symbol']==row['symbol']]
        if len(matches)!=1:raise ValueError('nonunique post-geometry graphics function '+row['id'])
        positions.append((matches[0]['opening']+1,index))
    for offset,index in sorted(positions,reverse=True):text=text[:offset]+f' f1_seen[{index}]=1; '+text[offset:]
    p.write_text(text)
    if verify_tree(source)!=tree_receipt:raise ValueError('input production tree changed during preparation')
    env={k:v for k,v in os.environ.items() if 'MUTATE' not in k}
    env.update(SMC_UI_OUT=str(out/'host'),SMC_UI_TEST_FILE='tests/ui_geometry/capture.c')
    with (out/'render.log').open('x') as log:
        proc=subprocess.run(['sh','tests/host/smc_ui_test.sh'],cwd=work,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=900)
    frames={};states={};observed=set();errors=[];(out/'renders').mkdir()
    for line in (out/'render.log').read_text().splitlines():
        if line.startswith('F1_STATE '):
            state=json.loads(line[9:]);capture=state.pop('capture')
            if type(capture)!=int or capture in states:raise ValueError('invalid or duplicate state receipt')
            states[capture]=state;continue
        if not line.startswith('F1_FRAME '):continue
        row=json.loads(line[9:]);name=row['id'];indices=row['renderers']
        if not indices or len(indices)!=len(set(indices)) or any(type(i)!=int or i<0 or i>=len(manifest['renderers']) for i in indices):raise ValueError('invalid renderer receipt')
        path=out/'host'/('f1-%04d-%s.png'%(row['capture'],name))
        if path.is_symlink():raise ValueError('symlink frame')
        raw_hash=hashlib.sha256(path.read_bytes()).hexdigest()
        saved=out/'renders'/path.name
        with Image.open(path) as im:
            im.load()
            if im.size!=(240,240) or im.format!='PNG' or all(a==b for a,b in im.convert('RGB').getextrema()):raise ValueError('invalid frame: '+name)
            pixels=im.convert('RGB').tobytes();im.save(saved,optimize=True)
        with Image.open(saved) as im:
            im.load()
            if im.convert('RGB').tobytes()!=pixels:raise ValueError('lossless PNG roundtrip changed pixels')
        branches=row.get('branches',[])
        if len(branches)!=len(set(branches)) or any(type(i)!=int or i<0 or i>=len(BRANCHES) for i in branches):raise ValueError('invalid branch receipt')
        item=dict(id=name,capture=row['capture'],evidenceKind=frame_method(name),path='renders/'+saved.name,sha256=hashlib.sha256(saved.read_bytes()).hexdigest(),nativePNGsha256=raw_hash,pixelSha256=hashlib.sha256(pixels).hexdigest(),renderers=[manifest['renderers'][i]['id'] for i in indices],branches=[BRANCHES[i] for i in branches])
        # Legacy controller fixtures intentionally replay a few same-name shots.
        # Retain distinct PNG bytes and receipt for every invocation separately.
        frames.setdefault(name,[]).append(item);observed.update(indices)
    if not frames:errors.append('no actual frame receipts')
    captures=set()
    for frameset in frames.values():
        for frame in frameset:
            capture=frame['capture']
            if capture in captures or capture not in states:raise ValueError('duplicate capture or missing actual UI state')
            frame['state']=states[capture];captures.add(capture)
    if captures!=set(states):raise ValueError('state receipt without actual frame')
    if proc.returncode:errors.append('renderer exited '+str(proc.returncode))
    missing=[r['id'] for i,r in enumerate(manifest['renderers']) if i not in observed]
    if manifest['unparsedPaintSites']:errors.append('unparsed painting sites require source audit')
    observed_branches={b for frameset in frames.values() for frame in frameset for b in frame['branches']}
    lines=['stable_id\tsource_sha256\tregistered\tsource_disposition\tobserved_frames\tstatus\tsample_state']
    for row in manifest['renderers']:
        reached=sorted(name for name,frameset in frames.items() if any(row['id'] in frame['renderers'] for frame in frameset))
        lines.append('\t'.join([row['id'],manifest['sources'][row['path']],str(row['registeredInTranslationUnit']),row['sourceDisposition'],str(len(reached)),'OBSERVED' if reached else 'NOT_RUN',reached[0] if reached else '']))
    (out/'stable-renderer-ids.tsv').write_text('\n'.join(lines)+'\n')
    report=dict(schema=1,status='FAIL' if errors else 'PASS',measurementScope='Source/frame integrity and measured runtime coverage only; external lane/R22 acceptance is not evaluated.',rendererExit=proc.returncode,
                sourceClosure=hashlib.sha256(json.dumps(closure,sort_keys=True).encode()).hexdigest(),productionTree=tree_receipt,runnerInputs=runner_hashes,
                renderers=len(manifest['renderers']),observedRenderers=len(observed),uncovered=missing,
                uncoveredDisposition={r['id']:r.get('sourceDisposition','REVIEW_REQUIRED') for r in manifest['renderers'] if r['id'] in missing},
                frames=frames,uniqueFrames=len(frames),stepLaneLabels=manifest['stepLaneLabels'],errors=errors,visualApproval=False,
                sourceCoverage={'status':'PARTIAL' if missing else 'COMPLETE','observedFunctions':len(observed),'inventoriedFunctions':len(manifest['renderers']),'unobservedFunctions':len(missing)},
                branchCoverage='PARTIAL',selectedBranches={b:('OBSERVED' if b in observed_branches else 'NOT_RUN') for b in BRANCHES},pixelConformance='NOT_RUN',F1='UNREVIEWED',R22VisualReview='UNREVIEWED',laneAcceptance='EXTERNAL_NOT_EVALUATED',semanticSites=manifest['semanticSites'],
                displaySlices=json.loads((out/'native-display-slices.json').read_text()),
                scope='Source renderer function inventory and actual host invocation receipts; no exhaustive branch or R22 acceptance. Native clip grid, clip-key pages/panel and pad-calibration painters use exact source bodies with explicit test RAM state. Boot safe/crash/UBOOT and hardware-calibration display slices exclude hardware/control-flow tails and do not certify invocation of their complete wrappers. No hardware/device qualification.')
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({k:v for k,v in report.items() if k!='frames'}));return 1 if errors else 0

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    try:raise SystemExit(run(a.source,a.output))
    except (ValueError,OSError,subprocess.SubprocessError) as e:print(json.dumps({'status':'FAIL','reason':str(e)}));raise SystemExit(1)
