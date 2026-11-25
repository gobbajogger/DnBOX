from pathlib import Path
p=Path('tree/firmware/src/app/ui.c')
s=Path('ui-before.c').read_text()
def one(a,b):
 global s
 assert s.count(a)==1,(a[:70],s.count(a))
 s=s.replace(a,b)
one('typedef struct { uint8_t kind, a, b, c; } pref_t;', '''#if X0X_S1_KEYBOARD_HOLD && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_S1WHITEKEYS = R_NOTELOCK + 63, R_S1MIDIVEL, R_S1MICROKEYS };
#endif
#if X0X_S1_KEYBOARD_HOLD && X0X_S1_SCALE_NATIVE
#include "sj6_137_host_witness.h"
extern unsigned s1c_scale_npo(unsigned);
#endif
#if X0X_SJ6_OTA_NATIVE
#include "../dsp/juno6.h"
extern int sj6_ota_get(unsigned);
#endif
typedef struct { uint8_t kind, a, b, c; } pref_t;''')
one('static const x0x_param_t *arp_desc(unsigned field);','static const x0x_param_t *arp_desc(unsigned owner, unsigned field);')
one('case R_ARP: return arp_desc(r.a);','case R_ARP: return arp_desc(r.b, r.a);')
one('    case R_NOTELOCK: return &NOTELOCK_P;', '''#if X0X_S1_KEYBOARD_HOLD && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1WHITEKEYS: { static const x0x_param_t p = {"WHITE KEYS",1,0,ONOFF_N}; return &p; }
    case R_S1MIDIVEL: { static const x0x_param_t p = {"MIDI VELOCITY",1,0,ONOFF_N}; return &p; }
#if X0X_S1_SCALE_NATIVE
    case R_S1MICROKEYS: { static const x0x_param_t p = {"TUNING KEYS",1,0,ONOFF_N}; return &p; }
#endif
#endif
    case R_NOTELOCK: return &NOTELOCK_P;''')
one('    case R_NOTELOCK: return engine_note_lock_capture_on();', '''#if X0X_S1_KEYBOARD_HOLD && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1WHITEKEYS: return !!(proj.set.bt & PROJ_S1_WHITE_KEYS);
    case R_S1MIDIVEL: return !!(proj.set.bt & PROJ_S1_MIDI_VELOCITY);
#if X0X_S1_SCALE_NATIVE
    case R_S1MICROKEYS: return !!(proj.set.ck_x1 & PROJ_S1_MICRO_KEYS);
#endif
#endif
    case R_NOTELOCK: return engine_note_lock_capture_on();''')
one('    case R_NOTELOCK: engine_note_lock_capture(v); break;', '''#if X0X_S1_KEYBOARD_HOLD && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1WHITEKEYS:
        proj.set.bt = (uint8_t)((proj.set.bt & ~PROJ_S1_WHITE_KEYS) | (v ? PROJ_S1_WHITE_KEYS : 0u)); break;
    case R_S1MIDIVEL:
        proj.set.bt = (uint8_t)((proj.set.bt & ~PROJ_S1_MIDI_VELOCITY) | (v ? PROJ_S1_MIDI_VELOCITY : 0u)); break;
#if X0X_S1_SCALE_NATIVE
    case R_S1MICROKEYS:
        proj.set.ck_x1 = (uint8_t)((proj.set.ck_x1 & ~PROJ_S1_MICRO_KEYS) | (v ? PROJ_S1_MICRO_KEYS : 0u)); break;
#endif
#endif
    case R_NOTELOCK: engine_note_lock_capture(v); break;''')
one('            do { static const pref_t refs[4] = {PR(R_FREEPLAY, 0, 0, 0), NONE, NONE, NONE};','''#if X0X_S1_KEYBOARD_HOLD && DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(engine_bass_bank(b)==ORIGINAL_SOUND_SYNTHONE)
                add_page(emit, "KEYS", PR(R_FREEPLAY,b,0,0), PR(R_S1WHITEKEYS,0,0,0), PR(R_S1MIDIVEL,0,0,0),
#if X0X_S1_SCALE_NATIVE
                         PR(R_S1MICROKEYS,0,0,0));
#else
                         NONE);
#endif
            else
#endif
            do { static const pref_t refs[4] = {PR(R_FREEPLAY, 0, 0, 0), NONE, NONE, NONE};''')
one('        k->pitch[physical]=(uint8_t)pitch;k->velocity[physical]=(uint8_t)velocity;', '''#if X0X_S1_KEYBOARD_HOLD
        if((proj.set.bt & PROJ_S1_WHITE_KEYS) && engine_bass_bank(b)==ORIGINAL_SOUND_SYNTHONE) {
            static const uint8_t degree[12]={0,0,1,1,2,3,3,4,4,5,5,6};
            pitch=25+7*(pitch/12)+degree[pitch%12];
        }
#if X0X_S1_SCALE_NATIVE
        if((proj.set.ck_x1 & PROJ_S1_MICRO_KEYS) && engine_bass_bank(b)==ORIGINAL_SOUND_SYNTHONE) {
            const uint8_t *scale=proj.sound.v[T_MST][S1_SCALE_ROW+b];
            unsigned code=scale[S1_SCALE_BYTE];
            int npo=(int)s1c_scale_npo(code);
#if X0X_S1_USER_SCALE
            if(code==S1C_SCALE_USER && scale[S1_USER_COUNT] &&
               scale[S1_USER_COUNT]<=S1C_USER_NOTES) npo=scale[S1_USER_COUNT];
#endif
            int base=41+ui.oct*npo;
            if(base<0)base=0;
            pitch=base+(int)physical;
            if(pitch>127)return;
        }
#endif
#endif
        k->pitch[physical]=(uint8_t)pitch;k->velocity[physical]=(uint8_t)velocity;''')
one('static const x0x_param_t *arp_desc(unsigned field) { return field < ARP_NF ? &ARP_P[field] : 0; }','''static const x0x_param_t *arp_desc(unsigned owner, unsigned field)
{
    if(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE) {
        static const x0x_param_t s1_direction={"DIRECTION",ARP_UPDN,0,ARP_MODE_N};
        static const x0x_param_t seq_enable={"SEQ ON",1,0,ONOFF_N};
        if(field==ARP_F_MODE)return &s1_direction;
        if(field==ARP_F_SEQ_ENABLE)return &seq_enable;
    }
    return field<sizeof(ARP_P)/sizeof(ARP_P[0]) && ARP_P[field].name ? &ARP_P[field] : 0;
}''')
one('    case ARP_F_MODE: return s.mode;','    case ARP_F_MODE: return s.mode;\n    case ARP_F_SEQ_ENABLE: return s.seq_on;')
one('                add_page(emit, "ARP SEQ", PR(R_ARP, ARP_F_LENGTH, b, 0), PR(R_ARP, ARP_F_INTERVAL, b, 0), PR(R_ARP, ARP_F_GATE, b, 0), NONE);','''                pref_t seq_enable=NONE;
#if X0X_SJ6_ARP_ADAPTER_NATIVE
                if(engine_bass_bank(b)==ORIGINAL_SOUND_SYNTHONE)
                    seq_enable=PR(R_ARP,ARP_F_SEQ_ENABLE,b,0);
#endif
                add_page(emit, "ARP SEQ", PR(R_ARP, ARP_F_LENGTH, b, 0), PR(R_ARP, ARP_F_INTERVAL, b, 0), PR(R_ARP, ARP_F_GATE, b, 0), seq_enable);''')
one('ph = as.on && engine_arp_view','ph = (as.on || (as.seq_on && engine_bass_bank(b)!=ORIGINAL_SOUND_SYNTHONE)) && engine_arp_view')
one('    if (kind == ORIGINAL_SOUND_JUNO) { juno_value_text(id, value, num, unit); return 1; }','''    if (kind == ORIGINAL_SOUND_JUNO) {
#if X0X_SJ6_OTA_NATIVE
        if(id==2u){
            int mode=sj6_ota_get(r.b);
            float x;
            if(mode>=0 && original_sound_control_value(kind,id,(unsigned)value,&x)){
                unit_value(X0X_U_HZ,juno6_sj6_panel_cutoff_hz(x,(unsigned)mode),num,unit);
                return 1;
            }
        }
#endif
        juno_value_text(id, value, num, unit);
        return 1;
    }''')
one('typedef struct { uint8_t kind, a, b, c; } pref_t;', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#include "sj6_137_host_witness.h"
extern int s1_typed_tuple_editable(void);
#endif
#if X0X_S1_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_S1SCALE=R_NOTELOCK+58 };
extern int s1_typed_scale_choose(unsigned,unsigned,int);
#endif
#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_SJ6LIMITER=R_NOTELOCK+54, R_SJ6KNOB=R_NOTELOCK+3 };
extern int sj6_native_limiter_get(unsigned,unsigned);
extern int sj6_native_limiter_choose(unsigned,unsigned,unsigned);
extern int engine_superj6_knob_position(unsigned,unsigned);
extern int engine_superj6_knob_turn(unsigned,unsigned,int,int);
extern float dnbx_sj6_vintage_cents(unsigned);
#endif
typedef struct { uint8_t kind, a, b, c; } pref_t;''')
one('    if (r.kind == R_BSTATUS) return 1;', '''#if X0X_S1_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_S1SCALE){
        if(r.b>=NBASS || !s1_typed_tuple_editable())return 1;
#if X0X_SJ6_SCALE_NATIVE
        if(live137(&proj.sound,r.b) && engine_bass_bank(r.b)==ORIGINAL_SOUND_JUNO)return 0;
#endif
        return !live_s1_mod(&proj.sound,r.b);
    }
#endif
#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_SJ6LIMITER)return sj6_native_limiter_get(r.b,r.c)<0 || !s1_typed_tuple_editable();
    if(r.kind==R_SJ6KNOB)return engine_superj6_knob_position(r.b,r.c)<0 || !s1_typed_tuple_editable();
#endif
    if (r.kind == R_BSTATUS) return 1;''')
one('    case R_NOTELOCK: return &NOTELOCK_P;', '''#if X0X_S1_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1SCALE: {static const x0x_param_t p={"SCALE",S1C_SCALE_MAX,0,0};return &p;}
#endif
#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_SJ6LIMITER: {
        static const x0x_param_t rage={"RAGE AMOUNT",127,0,0}, vintage={"VINTAGE",127,0,0};
        return r.c==SJ6_VINTAGE_SLOT?&vintage:r.c==SJ6_RAGE_SLOT?&rage:0;
    }
    case R_SJ6KNOB: {
        static const x0x_param_t trem[]={{"DEPTH",127,0,0},{"RATE",127,64,0}};
        return r.c>=SJ6_TREM_DEPTH && r.c<=SJ6_TREM_RATE ? &trem[r.c-SJ6_TREM_DEPTH]:0;
    }
#endif
    case R_NOTELOCK: return &NOTELOCK_P;''')
one('    case R_NOTELOCK: return engine_note_lock_capture_on();', '''#if X0X_S1_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1SCALE:return proj.sound.v[T_MST][S1_SCALE_ROW+r.b][S1_SCALE_BYTE];
#endif
#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_SJ6LIMITER: {int p=sj6_native_limiter_get(r.b,r.c);return p>=0?p:0;}
    case R_SJ6KNOB: {int p=engine_superj6_knob_position(r.b,r.c);return p>=0?p:0;}
#endif
    case R_NOTELOCK: return engine_note_lock_capture_on();''')
one('    pattern_t *p = pref_pattern(r);\n    const x0x_param_t *d = pref_desc(r);', '''#if X0X_S1_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_S1SCALE){
        if((unsigned)v<=S1C_SCALE_MAX && !pref_read_only(r) &&
           s1_typed_scale_choose(r.b,(unsigned)v,0))mark_dirty();
        return;
    }
#endif
#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_SJ6LIMITER || r.kind==R_SJ6KNOB){
        if((unsigned)v<=127u && !pref_read_only(r) &&
           (r.kind==R_SJ6LIMITER?sj6_native_limiter_choose(r.b,r.c,(unsigned)v):
                                engine_superj6_knob_turn(r.b,r.c,v,0)))mark_dirty();
        return;
    }
#endif
    pattern_t *p = pref_pattern(r);
    const x0x_param_t *d = pref_desc(r);''')
one('            do { static const pref_t refs[4] = {PR(R_FREEPLAY, 0, 0, 0), NONE, NONE, NONE};','''#if X0X_S1_SCALE_NATIVE && X0X_SJ6_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(live137(&proj.sound,(unsigned)b) && engine_bass_bank(b)==ORIGINAL_SOUND_JUNO)
                add_page(emit,"KEYS",PR(R_FREEPLAY,b,0,0),PR(R_S1SCALE,T_303,b,0),NONE,NONE);
            else
#endif
            do { static const pref_t refs[4] = {PR(R_FREEPLAY, 0, 0, 0), NONE, NONE, NONE};''')
one('            add_page(emit, "SOUND",PR(R_BENGINE', '''#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(kind==ORIGINAL_SOUND_JUNO && engine_superj6_knob_position(b,SJ6_TREM_DEPTH)>=0)
                add_page(emit,"FX MOTION",PR(R_SJ6KNOB,T_303,b,SJ6_TREM_DEPTH),
                         PR(R_SJ6KNOB,T_303,b,SJ6_TREM_RATE),PR(R_SJ6LIMITER,T_303,b,SJ6_RAGE_SLOT),
                         PR(R_SJ6LIMITER,T_303,b,SJ6_VINTAGE_SLOT));
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
one('X0X_COLD static void pref_value_of(pref_t r, int v, char *num, char *unit)\n{', '''X0X_COLD static void pref_value_of(pref_t r, int v, char *num, char *unit)
{
#if X0X_S1_SCALE_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_S1SCALE){
        unit[0]=0;
#if X0X_S1_USER_SCALE
        if(v==S1C_SCALE_USER){
            const uint8_t *n=proj.sound.v[T_MST][S1_SCALE_ROW+r.b]+S1_USER_ID;
            for(unsigned i=0;i<S1C_USER_NAME;i++){num[i]=(char)n[i];if(!n[i])break;}
            num[S1C_USER_NAME]=0;
            if(!num[0])put_s(num,"USER");
            return;
        }
#endif
        put_s(num,s1c_scale_name((unsigned)v));return;
    }
#endif
#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_SJ6LIMITER && r.c==SJ6_VINTAGE_SLOT){
        put_d1(num,dnbx_sj6_vintage_cents((unsigned)v),1);put_s(unit,"cent");return;
    }
#endif''')
one('typedef struct { uint8_t kind, a, b, c; } pref_t;', '''#if X0X_SJ6_VELOCITY_SETTINGS && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_SJ6INPUTVEL=R_NOTELOCK+70 };
extern int sj6_input_velocity_choose(unsigned,unsigned,unsigned);
#endif
#if X0X_SJ6_BEND_TYPED_PROBE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_SJ6PB=R_NOTELOCK+6 };
extern int sj6_typed_pb_edit(unsigned,unsigned,unsigned);
#endif
#if X0X_SJ6_MOD_TYPED_PROBE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_SJ6MOD=R_NOTELOCK+7 };
extern int sj6_typed_mod_choose(unsigned,unsigned);
#endif
#if X0X_SJ6_MONO32_PROBE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_SJ6MONO32=R_NOTELOCK+15 };
extern int engine_sj6_mono32_get(unsigned,unsigned);
extern int engine_sj6_mono32_edit(unsigned,unsigned,unsigned);
#endif
typedef struct { uint8_t kind, a, b, c; } pref_t;''')
one('    if (r.kind == R_BSTATUS) return 1;', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    switch(r.kind){
#if X0X_SJ6_BEND_TYPED_PROBE
    case R_SJ6PB:
#endif
#if X0X_SJ6_MOD_TYPED_PROBE
    case R_SJ6MOD:
#endif
#if X0X_SJ6_VELOCITY_SETTINGS
    case R_SJ6INPUTVEL:
#endif
#if X0X_SJ6_BEND_TYPED_PROBE || X0X_SJ6_MOD_TYPED_PROBE || X0X_SJ6_VELOCITY_SETTINGS
        return r.b>=NBASS || engine_bass_bank(r.b)!=ORIGINAL_SOUND_JUNO ||
               !live137(&proj.sound,r.b) || !s1_typed_tuple_editable();
#endif
#if X0X_SJ6_MONO32_PROBE
    case R_SJ6MONO32:return engine_sj6_mono32_get(r.b,r.c)<0 || !s1_typed_tuple_editable();
#endif
    default:break;
    }
#endif
    if (r.kind == R_BSTATUS) return 1;''')
one('    case R_NOTELOCK: return &NOTELOCK_P;', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#if X0X_SJ6_BEND_TYPED_PROBE
    case R_SJ6PB: {
        static const x0x_param_t p[]={{"PB DOWN",48,0,0},{"PB UP",48,0,0}};
        return r.c<2?p+r.c:0;
    }
#endif
#if X0X_SJ6_MOD_TYPED_PROBE
    case R_SJ6MOD: {
        static const char *const names[]={"LFO Rate","LFO>DCO","LFO>Cutoff","Tremolo","LFO2 Rate","CUTOFF"};
        static const x0x_param_t p={"MOD WHEEL DESTINATION",5,0,names};
        return r.c==0?&p:0;
    }
#endif
#if X0X_SJ6_MONO32_PROBE
    case R_SJ6MONO32: {
        static const x0x_param_t p[]={{"MONO",1,0,ONOFF_N},{"LEGATO",1,0,ONOFF_N},{"GLIDE TIME",127,0,0}};
        return r.c<3?p+r.c:0;
    }
#endif
#if X0X_SJ6_VELOCITY_SETTINGS
    case R_SJ6INPUTVEL: {
        static const char *const curves[]={"SOFT","MEDIUM","HARD"};
        static const x0x_param_t p[]={{"FIXED VELOCITY",1,0,ONOFF_N},{"MIDI VEL CURVE",2,1,curves}};
        return r.c<2?p+r.c:0;
    }
#endif
#endif
    case R_NOTELOCK: return &NOTELOCK_P;''')
one('    case R_NOTELOCK: return engine_note_lock_capture_on();', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#if X0X_SJ6_BEND_TYPED_PROBE
    case R_SJ6PB: {
        if(r.b>=NBASS || r.c>=2 || !live137(&proj.sound,r.b))return 0;
        unsigned m=sj6_pb_decode(read137(&proj.sound,r.b)[SJ6_PB_DN_BYTE+r.c]);
        return r.c?(int)m:(int)(SJ6_PB_LIMIT-m);
    }
#endif
#if X0X_SJ6_MOD_TYPED_PROBE
    case R_SJ6MOD:return r.b<NBASS && live137(&proj.sound,r.b)?read137(&proj.sound,r.b)[SJ6_MOD_BYTE]:0;
#endif
#if X0X_SJ6_MONO32_PROBE
    case R_SJ6MONO32:{int p=engine_sj6_mono32_get(r.b,r.c);return p<0?0:p;}
#endif
#if X0X_SJ6_VELOCITY_SETTINGS
    case R_SJ6INPUTVEL:{
        if(r.b>=NBASS || !live137(&proj.sound,r.b))return 0;
        unsigned code=sj6_input_vel(&proj.sound,r.b), curve=code%3u;
        return r.c?(int)(curve<2u?1u-curve:2u):(int)(code/3u);
    }
#endif
#endif
    case R_NOTELOCK: return engine_note_lock_capture_on();''')
one('    pattern_t *p = pref_pattern(r);\n    const x0x_param_t *d = pref_desc(r);','''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#define SJ6_TYPED_WRITE(kind,call) case kind: {const x0x_param_t *d=pref_desc(r); \\
    if(d && (unsigned)v<=d->max && !pref_read_only(r) && (call))mark_dirty(); return;}
    switch(r.kind){
#if X0X_SJ6_BEND_TYPED_PROBE
    SJ6_TYPED_WRITE(R_SJ6PB,sj6_typed_pb_edit(r.b,r.c,(unsigned)v))
#endif
#if X0X_SJ6_MOD_TYPED_PROBE
    SJ6_TYPED_WRITE(R_SJ6MOD,sj6_typed_mod_choose(r.b,(unsigned)v))
#endif
#if X0X_SJ6_MONO32_PROBE
    SJ6_TYPED_WRITE(R_SJ6MONO32,engine_sj6_mono32_edit(r.b,r.c,(unsigned)v))
#endif
#if X0X_SJ6_VELOCITY_SETTINGS
    SJ6_TYPED_WRITE(R_SJ6INPUTVEL,sj6_input_velocity_choose(r.b,r.c,(unsigned)v))
#endif
    default:break;
    }
#undef SJ6_TYPED_WRITE
#endif
    pattern_t *p = pref_pattern(r);
    const x0x_param_t *d = pref_desc(r);''')
one('            add_page(emit, "SOUND",PR(R_BENGINE','''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(kind==ORIGINAL_SOUND_JUNO && live137(&proj.sound,b)){
#if X0X_SJ6_BEND_TYPED_PROBE
                add_page(emit,"WHEELS",PR(R_SJ6PB,T_303,b,0),PR(R_SJ6PB,T_303,b,1),
#if X0X_SJ6_MOD_TYPED_PROBE
                         PR(R_SJ6MOD,T_303,b,0),
#else
                         NONE,
#endif
#if X0X_SJ6_VELOCITY_SETTINGS
                         PR(R_SJ6INPUTVEL,T_303,b,0));
#else
                         NONE);
#endif
#endif
#if X0X_SJ6_MONO32_PROBE
                if(engine_sj6_mono32_get(b,0)>=0)
                    add_page(emit,"VOICE",PR(R_SJ6MONO32,T_303,b,0),PR(R_SJ6MONO32,T_303,b,1),
                             PR(R_SJ6MONO32,T_303,b,2),
#if X0X_SJ6_VELOCITY_SETTINGS
                             PR(R_SJ6INPUTVEL,T_303,b,1));
#else
                             NONE);
#endif
#endif
            }
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
transport=Path('../remote-both-link/tree/firmware/src/app/ui.c').read_text()
start=transport.index('#if X0X_REMOTE_DNR3\n/* DNR3 Remote (REMOTE-FW-PROTOCOL 4.1)')
assert 'int ui_remote_pref(' not in s
s=s.rstrip()+'\n\n'+transport[start:]
one('typedef struct { uint8_t kind, a, b, c; } pref_t;', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
enum { R_S1PHASER=R_NOTELOCK+32, R_S1CWG=R_NOTELOCK+31, R_S1CMP=R_NOTELOCK+46 };
extern int s1_phaser_get(unsigned,unsigned);
extern int s1_phaser_choose(unsigned,unsigned,unsigned);
extern int s1_cmp_position(unsigned,unsigned);
extern int s1_cmp_choose(unsigned,unsigned,unsigned);
extern float s1_cmp_physical(unsigned,unsigned);
extern int s1_typed_cwg_position(unsigned);
extern int s1_typed_cwg_choose(unsigned,unsigned);
#if X0X_S1_AUTOPAN_PROBE
enum { R_S1PAN=R_NOTELOCK+12 };
extern int s1_typed_pan_read(unsigned,unsigned*,unsigned*,int*,float*);
extern int s1_typed_pan_choose(unsigned,int,unsigned);
static unsigned s1_pan_pos(unsigned v,unsigned *max){
    unsigned p=0; int sync; float hz;
    if(!s1_typed_pan_read(v,&p,max,&sync,&hz))*max=0;
    return p;
}
#endif
#endif
typedef struct { uint8_t kind, a, b, c; } pref_t;''')
one('    if (r.kind == R_BSTATUS) return 1;', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_S1PHASER)return s1_phaser_get(r.b,r.c)<0 || !s1_typed_tuple_editable();
    if(r.kind==R_S1CMP)return s1_cmp_position(r.b,r.c)<0 || !s1_typed_tuple_editable();
    if(r.kind==R_S1CWG)return s1_typed_cwg_position(r.b)<0 || !s1_typed_tuple_editable();
#if X0X_S1_AUTOPAN_PROBE
    if(r.kind==R_S1PAN)return r.b>=NBASS || !live_s1_mod(&proj.sound,r.b) || !s1_typed_tuple_editable();
#endif
#endif
    if (r.kind == R_BSTATUS) return 1;''')
one('    case R_NOTELOCK: return &NOTELOCK_P;', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1PHASER:{
        static const x0x_param_t p[4]={{"MIX",127,0,0},{"RATE",127,5,0},
            {"FEEDBACK",127,0,0},{"WIDTH",127,99,0}};
        return r.c<S1_PHASE_COUNT?&p[r.c]:0;
    }
    case R_S1CMP:{
        static const x0x_param_t p[]={{"RATIO",127,0,0},{"THRESHOLD",127,0,0},{"ATTACK",127,0,0},
            {"RELEASE",127,0,0},{"MASTER",127,0,0},{"RV INPUT",127,0,0}};
        return r.c<S1_CMP_COUNT ? &p[r.c<12u?r.c/3u:r.c-8u]:0;
    }
    case R_S1CWG:{static const x0x_param_t p={"WET MAKEUP",S1_CWG_MAX,S1_CWG_INIT,0};return r.c?0:&p;}
#if X0X_S1_AUTOPAN_PROBE
    case R_S1PAN:{
        static const x0x_param_t amount={"AMOUNT",127,0,0};
        static x0x_param_t rate={"RATE",127,0,0};
        if(r.b>=NBASS || r.c>=2 || !live_s1_mod(&proj.sound,r.b))return 0;
        if(!r.c)return &amount;
        unsigned max; (void)s1_pan_pos(r.b,&max); rate.max=(uint8_t)max;
        return &rate;
    }
#endif
#endif
    case R_NOTELOCK: return &NOTELOCK_P;''')
one('    case R_NOTELOCK: return engine_note_lock_capture_on();', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    case R_S1PHASER:{int x=s1_phaser_get(r.b,r.c);return x>=0?x:(int)s1_phase_init(r.c);}
    case R_S1CMP:{int x=s1_cmp_position(r.b,r.c);return x>=0?x:0;}
    case R_S1CWG:{int x=s1_typed_cwg_position(r.b);return x>=0?x:0;}
#if X0X_S1_AUTOPAN_PROBE
    case R_S1PAN:{
        unsigned max;
        return r.b>=NBASS||!live_s1_mod(&proj.sound,r.b)?0:r.c?(int)s1_pan_pos(r.b,&max):read137(&proj.sound,r.b)[S1_PAN_AMOUNT_BYTE];
    }
#endif
#endif
    case R_NOTELOCK: return engine_note_lock_capture_on();''')
one('    pattern_t *p = pref_pattern(r);\n    const x0x_param_t *d = pref_desc(r);','''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(r.kind==R_S1PHASER || r.kind==R_S1CMP || r.kind==R_S1CWG
#if X0X_S1_AUTOPAN_PROBE
       || r.kind==R_S1PAN
#endif
       ){
        const x0x_param_t *d=pref_desc(r);
        if(!d || (unsigned)v>d->max || pref_read_only(r))return;
        int ok=0;
        switch(r.kind){
        case R_S1PHASER:ok=s1_phaser_choose(r.b,r.c,(unsigned)v);break;
        case R_S1CMP:ok=s1_cmp_choose(r.b,r.c,(unsigned)v);break;
        case R_S1CWG:ok=s1_typed_cwg_choose(r.b,(unsigned)v);break;
#if X0X_S1_AUTOPAN_PROBE
        case R_S1PAN:ok=s1_typed_pan_choose(r.b,r.c,(unsigned)v);break;
#endif
        }
        if(ok)mark_dirty();
        return;
    }
#endif
    pattern_t *p = pref_pattern(r);
    const x0x_param_t *d = pref_desc(r);''')
one('            add_page(emit, "SOUND",PR(R_BENGINE','''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(kind==ORIGINAL_SOUND_SYNTHONE && live_s1_mod(&proj.sound,b)){
                add_page(emit,"PHASER",PR(R_S1PHASER,T_303,b,0),PR(R_S1PHASER,T_303,b,1),
                         PR(R_S1PHASER,T_303,b,2),PR(R_S1PHASER,T_303,b,3));
                add_page(emit,"MASTER CMP",PR(R_S1CMP,T_303,b,0),PR(R_S1CMP,T_303,b,3),
                         PR(R_S1CMP,T_303,b,6),PR(R_S1CMP,T_303,b,9));
                add_page(emit,"RV INPUT CMP",PR(R_S1CMP,T_303,b,1),PR(R_S1CMP,T_303,b,4),
                         PR(R_S1CMP,T_303,b,7),PR(R_S1CMP,T_303,b,10));
                add_page(emit,"WET CMP",PR(R_S1CMP,T_303,b,2),PR(R_S1CMP,T_303,b,5),
                         PR(R_S1CMP,T_303,b,8),PR(R_S1CMP,T_303,b,11));
                add_page(emit,"CMP GAIN",PR(R_S1CMP,T_303,b,12),PR(R_S1CMP,T_303,b,13),
                         PR(R_S1CWG,T_303,b,0),NONE);
#if X0X_S1_AUTOPAN_PROBE
                add_page(emit,"AUTO PAN",PR(R_S1PAN,T_303,b,0),PR(R_S1PAN,T_303,b,1),NONE,NONE);
#endif
            }
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
one('enum { R_S1PATCH = R_NOTELOCK + 1 };','enum { R_S1PATCH = R_NOTELOCK + 1, R_S1LINK = R_NOTELOCK + 4 };')
one('    const s1_patch_descriptor *d = s1_patch_at(r.c);\n    return d && engine_s1_patch_read(r.b, d->factory_id, value);','''    if(r.kind==R_S1LINK)return engine_s1_patch_read(r.b,S1_LINK_ID,value);
    const s1_patch_descriptor *d = s1_patch_at(r.c);
    return d && engine_s1_patch_read(r.b, d->factory_id, value);''')
one('    if (value > S1_PATCH_STEPS) value = S1_PATCH_STEPS;\n    const s1_patch_descriptor *d = s1_patch_at(r.c);\n    int status = engine_s1_patch_set(r.b, owner.revision, d->factory_id, (unsigned)value);','''    if (value > owner.max_position) value = owner.max_position;
    const s1_patch_descriptor *d = s1_patch_at(r.c);
    if(r.kind!=R_S1LINK && !d)return;
    int status = engine_s1_patch_set(r.b, owner.revision, r.kind==R_S1LINK?S1_LINK_ID:d->factory_id, (unsigned)value);''')
one('#if X0X_SYNTHONE_PATCH\n    case R_S1PATCH: {\n        const s1_patch_descriptor *d = s1_patch_at(r.c);\n        static x0x_param_t pot','''#if X0X_SYNTHONE_PATCH
    case R_S1LINK:{static const x0x_param_t p={"LINK",1,0,ONOFF_N};return r.c?0:&p;}
    case R_S1PATCH: {
        const s1_patch_descriptor *d = s1_patch_at(r.c);
        static x0x_param_t pot''')
one('        pot.name = d->label;\n        return &pot;', '        pot.name = d->label;\n        pot.max = (uint8_t)s1_patch_steps(r.c);\n        return &pot;')
one('    case R_S1PATCH: {\n        const s1_patch_descriptor *d = s1_patch_at(r.c);\n        if (d && s1_draw_patch','    case R_S1LINK:{s1_patch_value x;return s1_read(r,&x)?x.position:0;}\n    case R_S1PATCH: {\n        const s1_patch_descriptor *d = s1_patch_at(r.c);\n        if (d && s1_draw_patch')
one('if (r.kind == R_S1PATCH) { s1_edit(r, v, 0); return; }','if (r.kind == R_S1PATCH || r.kind == R_S1LINK) { s1_edit(r, v, 0); return; }')
one('    if (r.kind == R_S1PATCH) { s1_edit(r, accel(knob, e, S1_PATCH_STEPS), 1); return; }','''    if (r.kind == R_S1LINK) { s1_edit(r, (e>0)-(e<0), 1); return; }
    if (r.kind == R_S1PATCH) { s1_edit(r, accel(knob, e, s1_patch_steps(r.c)), 1); return; }''')
one('            add_page(emit, "SOUND",PR(R_BENGINE','''#if X0X_SYNTHONE_PATCH
            if(kind==ORIGINAL_SOUND_SYNTHONE)
                add_page(emit,"FILTER LINK",PR(R_S1LINK,T_303,b,0),NONE,NONE,NONE);
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
# Authored scalar preferences, descriptor and read/write delegates from the composed UI patch.
# b/c ready before descriptor lookup; enable each only with its real provider.
scalar=[
('S1DLFB',13,'X0X_S1_FEEDBACK_PROBE','"FEEDBACK",127,14,0','read137(&proj.sound,r.b)[S1_FB_BYTE]?(int)read137(&proj.sound,r.b)[S1_FB_BYTE]-1:14','s1_typed_feedback_choose(r.b,(unsigned)v)'),
('S1DLYRES',45,'1','"INPUT RES",S1_DELAY_RES_MAX,0,0','s1_delay_res_get(r.b)','s1_delay_res_choose(r.b,(unsigned)v)'),
('S1ENVLFO',33,'1','"ENV LFO",S1_ENV_LFO_MAX,0,s1_env_lfo_names','s1_env_lfo_get(r.b)','s1_env_lfo_choose(r.b,(unsigned)v)'),
('S1DICT',20,'X0X_S1_DELAY_TRACK_TEST','"IN CUTOFF",S1_DICT_MAX,S1_DICT_INIT,0','s1_typed_dict_position(r.b)','s1_typed_dict_choose(r.b,(unsigned)v)'),
('S1CRUSH',11,'X0X_S1_CRUSH_LFO_PROBE','"CRUSH LFO",S1_CRUSH_LAST,0,s1_env_lfo_names','read137(&proj.sound,r.b)[S1_CRUSH_BYTE]','s1_typed_crush_choose(r.b,(unsigned)v)'),
('S1AA',35,'1','"ANTI-ALIAS",1,0,ONOFF_N','s1_aa_get(r.b)','s1_aa_choose(r.b,(unsigned)v)'),
('S1A4',8,'1','"A REF",60,30,0','(int)s1_a4_hz(read137(&proj.sound,r.b)[S1_A4_BYTE])-410','s1_typed_a4_choose(r.b,(unsigned)v+410u)'),
('S1RVFB',9,'1','"RV SIZE",128,64,0','(int)s1_rvfb_pos(read137(&proj.sound,r.b)[S1_RVFB_BYTE])','s1_typed_rvfb_choose(r.b,(unsigned)v)'),
('S1TRANS',10,'1','"TRANSPOSE",48,24,0','s1_trans_st(read137(&proj.sound,r.b)[S1_TRANS_BYTE])+24','s1_typed_trans_choose(r.b,v-24)'),
('S1HALF',59,'X0X_S1_HALFTIME','"SMOOTH TIME",127,S1_HALF_DEFAULT,0','s1_half_code(&proj.sound,r.b)?(int)s1_half_code(&proj.sound,r.b)-1:S1_HALF_DEFAULT','s1_half_choose(r.b,(unsigned)v)'),
('S1KEY_HOLD',56,'X0X_S1_KEYBOARD_HOLD','"KEY HOLD",1,0,ONOFF_N','engine_s1_key_hold(r.b)','engine_s1_key_set_hold(r.b,v)'),
('S1A4RESET',57,'X0X_S1_KEYBOARD_HOLD && X0X_S1_SCALE_NATIVE','"RESET TUNING",1,0,0','0','v==1 && s1_typed_scale_choose(r.b,0,1)'),
('S1MODWHEEL',5,'1','"MOD WHEEL",2,0,s1_mod_wheel_names','read137(&proj.sound,r.b)[3]','s1_typed_mod_choose(r.b,(unsigned)v)'),
]
import re
from collections import defaultdict
sections=defaultdict(str)
allimpl=Path('tree/firmware/src/app/original_presets.inc').read_text()+Path('tree/firmware/src/app/engine.c').read_text()+Path('tree/firmware/src/app/engine_arp.inc').read_text()
protos={}
for n,i,g,d,get,setp in scalar:
    for nm in re.findall(r'(?<!\w)((?:engine_s1|s1)\w+)\(',get+' '+setp):
        m=re.search(r'^(?:X0X_COLD )?(int '+re.escape(nm)+r'\([^{};]*\))\s*\{',allimpl,re.M)
        if m:protos[nm]=m[1]+';'
    q='R_'+n
    def add(k,t):sections[k]+=f'#if {g}\n{t}\n#endif\n'
    add('enum',f'enum {{ {q}=R_NOTELOCK+{i} }};')
    add('desc',f'    case {q}:{{static const x0x_param_t p={{{d}}};return r.c?0:&p;}}')
    add('get',f'    case {q}:return r.b<NBASS && live_s1_mod(&proj.sound,r.b)?({get}):0;')
    add('read',f'    if(r.kind=={q})return r.b>=NBASS || !live_s1_mod(&proj.sound,r.b) || !s1_typed_tuple_editable();')
    add('set',f'    if(r.kind=={q}){{const x0x_param_t *d=pref_desc(r); if(d && (unsigned)v<=d->max && !pref_read_only(r) && ({setp}))mark_dirty(); return;}}')
def inject(anchor,section):
    one(anchor,'#if DNBOX_TYPED_CONTROLS_NATIVE_TEST\n'+section+'#endif\n'+anchor)
inject('typedef struct { uint8_t kind, a, b, c; } pref_t;',
       'extern '+'\nextern '.join(protos.values())+'\n'+sections['enum']+'''#if X0X_S1_DELAY_TIME_NATIVE
enum { R_S1DLYTIME=R_NOTELOCK+34 };
extern int s1_delay_time_read(unsigned,unsigned*,unsigned*,float*);
extern int s1_delay_time_choose(unsigned,unsigned);
#endif
''')
inject('    if (r.kind == R_BSTATUS) return 1;',sections['read']+'''#if X0X_S1_DELAY_TIME_NATIVE
    if(r.kind==R_S1DLYTIME){unsigned p,max;float sec;
        return r.b>=NBASS || !s1_typed_tuple_editable() || s1_delay_time_read(r.b,&p,&max,&sec)==-2;}
#endif
''')
inject('    case R_NOTELOCK: return &NOTELOCK_P;',sections['desc']+'''#if X0X_S1_DELAY_TIME_NATIVE
    case R_S1DLYTIME:{
        static x0x_param_t d={"TIME",127,0,0};
        unsigned p,max;float sec;
        if(r.c || r.b>=NBASS || s1_delay_time_read(r.b,&p,&max,&sec)==-2)return 0;
        d.max=(uint8_t)max;return &d;
    }
#endif
''')
one('static const x0x_param_t *pref_desc(pref_t r)\n{','''static const x0x_param_t *pref_desc(pref_t r)
{
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
    static const char *const s1_env_lfo_names[]={"OFF","LFO 1","LFO 2","BOTH"};
    static const char *const s1_mod_wheel_names[]={"CUTOFF","LFO1 RATE","LFO2 RATE"};
#endif''')
inject('    case R_NOTELOCK: return engine_note_lock_capture_on();',sections['get']+'''#if X0X_S1_DELAY_TIME_NATIVE
    case R_S1DLYTIME:{
        unsigned p,max;float sec;
        return r.b>=NBASS || s1_delay_time_read(r.b,&p,&max,&sec)==-2?0:(int)p;
    }
#endif
''')
inject('    pattern_t *p = pref_pattern(r);\n    const x0x_param_t *d = pref_desc(r);',sections['set']+'''#if X0X_S1_DELAY_TIME_NATIVE
    if(r.kind==R_S1DLYTIME){
        if(v>=0 && !pref_read_only(r)){
            int rc=s1_delay_time_choose(r.b,(unsigned)v);
            if(rc>0)mark_dirty();else if(rc<0)say("DELAY TIME LIMIT",0);
        }
        return;
    }
#endif
''')
historical=Path('../restored-handoff/docs/handoffs/2026-11-24-source/composed-controls/patches/firmware/src/app/ui.c.patch').read_text().splitlines()
def original_lines(first,last):
    lines=historical[first-1:last]
    assert all(l.startswith(('+',' ')) for l in lines)
    return '\n'.join(l[1:] for l in lines)+'\n'
seam=[
('SJ6ROOM',49,'X0X_SJ6_NATIVE_ROOM','sj6_native_room_get','sj6_native_room_choose',original_lines(591,597)),
('SJ6CRUSH',53,'X0X_SJ6_NATIVE_ROOM','sj6_crush_get','sj6_crush_choose',original_lines(587,590)),
('SJ6OTA',51,'X0X_SJ6_OTA_NATIVE','sj6_ota_get','sj6_ota_choose','    case R_SJ6OTA:{static const x0x_param_t p={"FILTER TYPE",1,0,0};return r.c?0:&p;}\n'),
('SJ6UNISON',36,'X0X_SJ6_NATIVE_UNISON','engine_sj6_unison_get','engine_sj6_unison_edit',original_lines(664,668)),
('SJ6J60',37,'X0X_SJ6_NATIVE_J60','engine_sj6_j60_get','engine_sj6_j60_edit',original_lines(671,676)),
('SJ6PITCH',47,'X0X_SJ6_PITCH_ENV_NATIVE','sj6_pitch_get','sj6_pitch_choose',original_lines(708,712)),
('SJ6FILTER_EXTRA',48,'X0X_SJ6_VCF_PORT','sj6_filter_control_get','sj6_filter_control_choose',original_lines(715,718)),
('SJ6AUX',50,'X0X_SJ6_AUX_LFO_NATIVE','sj6_aux_get','sj6_aux_choose',original_lines(721,739)),
]
sections=defaultdict(str)
for n,i,g,get,setp,desc in seam:
    desc=desc[desc.index('    case R_'+n):]
    assert desc.rstrip().endswith('}') or desc.rstrip().endswith(';}')
    signatures={}
    for nm in (get,setp):
        m=re.search(r'^(?:X0X_COLD )?(int '+re.escape(nm)+r'\([^{};]*\))\s*\{',allimpl,re.M)
        assert m,nm
        signatures[nm]=m[1]
    assert f'R_{n}=R_NOTELOCK+{i}' in '\n'.join(historical).replace(' ','') or f'R_{n}=R_NOTELOCK+{i}' in '\n'.join(historical).replace(' ','').replace('};',',')
    q='R_'+n
    gt=f'{get}(r.b'+('' if n=='SJ6OTA' else ',r.c')+')'
    st=f'{setp}(r.b'+('' if n=='SJ6OTA' else ',r.c')+', (unsigned)v)'
    if n=='SJ6AUX':
        st=f'{setp}(r.b,r.c,(unsigned)(sj6_aux_field_route(r.c)?sj6_aux_route_wire((unsigned)v):v))'
    def add(k,t):sections[k]+=f'#if {g}\n{t}\n#endif\n'
    add('enum',f'enum {{ {q}=R_NOTELOCK+{i} }};\nextern '+ ';\nextern '.join(signatures.values())+';')
    add('desc',desc.rstrip())
    add('get',f'    case {q}:{{int x={gt};return x<0?0:x;}}' if n!='SJ6AUX' else f'    case {q}:{{int x={gt};return x<0?0:sj6_aux_field_route(r.c)?(int)sj6_aux_route_detent((unsigned)x):x;}}')
    add('read',f'    if(r.kind=={q})return r.b>=NBASS || {gt}<0 || !s1_typed_tuple_editable();')
    add('set',f'    if(r.kind=={q}){{const x0x_param_t *d=pref_desc(r); if(d && (unsigned)v<=d->max && !pref_read_only(r) && {st})mark_dirty(); return;}}')
inject('typedef struct { uint8_t kind, a, b, c; } pref_t;',
       '#if X0X_SJ6_AUX_LFO_NATIVE\n#include "../dsp/juno6_aux_catalog.h"\n'+original_lines(172,207)+'#endif\n'+sections['enum'])
inject('    if (r.kind == R_BSTATUS) return 1;',sections['read'])
inject('    case R_NOTELOCK: return &NOTELOCK_P;',sections['desc'])
inject('    case R_NOTELOCK: return engine_note_lock_capture_on();',sections['get'])
inject('    pattern_t *p = pref_pattern(r);\n    const x0x_param_t *d = pref_desc(r);',sections['set'])
inject('            add_page(emit, "SOUND",PR(R_BENGINE', '''#if X0X_SJ6_NATIVE_ROOM
            if(kind==ORIGINAL_SOUND_JUNO && sj6_native_room_get(b,SJ6_ROOM_ENABLE)>=0){
                add_page(emit,"REVERB",PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_ENABLE),
                         PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_SIZE),PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_LOCUT),
                         PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_MIX));
                add_page(emit,"STEREO FX",PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_TYPE),
                         PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_WIDEN),PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_RATE),
                         PR(R_SJ6ROOM,T_303,b,SJ6_ROOM_DEPTH));
            }
#endif
''')
one('enum { R_SJ6LIMITER=R_NOTELOCK+54, R_SJ6KNOB=R_NOTELOCK+3 };','enum { R_SJ6LIMITER=R_NOTELOCK+54 };')
one('typedef struct { uint8_t kind, a, b, c; } pref_t;', '''#if X0X_SUPERJ6_BUILTIN_INIT
#include "../dsp/juno6.h"
enum { R_SJ6KNOB=R_NOTELOCK+3 };
extern int engine_superj6_knob_position(unsigned,unsigned);
extern int engine_superj6_knob_turn(unsigned,unsigned,int,int);
#endif
typedef struct { uint8_t kind, a, b, c; } pref_t;''')
one('    if(r.kind==R_SJ6KNOB)return engine_superj6_knob_position(r.b,r.c)<0 || !s1_typed_tuple_editable();\n','')
one('    case R_SJ6KNOB: {\n        static const x0x_param_t trem[]={{"DEPTH",127,0,0},{"RATE",127,64,0}};\n        return r.c>=SJ6_TREM_DEPTH && r.c<=SJ6_TREM_RATE ? &trem[r.c-SJ6_TREM_DEPTH]:0;\n    }\n','')
one('    case R_SJ6KNOB: {int p=engine_superj6_knob_position(r.b,r.c);return p>=0?p:0;}\n','')
one('    if(r.kind==R_SJ6LIMITER || r.kind==R_SJ6KNOB){\n        if((unsigned)v<=127u && !pref_read_only(r) &&\n           (r.kind==R_SJ6LIMITER?sj6_native_limiter_choose(r.b,r.c,(unsigned)v):\n                                engine_superj6_knob_turn(r.b,r.c,v,0)))mark_dirty();','''    if(r.kind==R_SJ6LIMITER){
        if((unsigned)v<=127u && !pref_read_only(r) &&
           sj6_native_limiter_choose(r.b,r.c,(unsigned)v))mark_dirty();''')
body=transport[transport.index('    static const char *const wave[]',transport.index('static const x0x_param_t *sj6_knob_desc')):]
body=body[:body.index('    };')]
body=body.replace('knobs[SJ6_KNOBS]','knobs[SJ6_KNOBS+1]')
defs='''
        [SJ6_MASTER_LEVEL]={"LEVEL",127,127,0},
        [SJ6_LFO1_RATE]={"LFO1 RATE",127,64,0}, [SJ6_LFO2_RATE]={"LFO2 RATE",127,64,0},
        [SJ6_LFO3_RATE]={"LFO3 RATE",127,64,0}, [SJ6_LFO2_PWM_DEPTH]={"PWM DEPTH",SJ6_PWM_STEPS,0,0},
        [SJ6_LFO2_PWM_ROUTE]={"PWM ROUTE",2,0,pwm_route},
        [SJ6_LFO1_MODE]={"LFO1 MODE",1,0,clock_mode}, [SJ6_LFO2_MODE]={"LFO2 MODE",1,0,clock_mode},
        [SJ6_LFO3_MODE]={"LFO3 MODE",1,0,clock_mode},
        [SJ6_CHORUS_I]={"CHORUS I",1,0,enable}, [SJ6_CHORUS_II]={"CHORUS II",1,0,enable},
        [SJ6_CHORUS_RATE]={"CHORUS RATE",127,64,0}, [SJ6_CHORUS_MIX]={"CHORUS MIX",127,0,0},
        [SJ6_VIBRATO_DEPTH]={"VIBRATO",127,0,0}, [SJ6_VIBRATO_RATE]={"VIBRATE RATE",127,0,0},
        [SJ6_A_REFERENCE]={"A REF",SJ6_A_REFERENCE_STEPS,SJ6_A_REFERENCE_DEFAULT,0},
        [SJ6_LFO2_SHAPE]={"LFO2 WAVE",4,0,lfo_wave},
        [SJ6_LAYER1_SEMI]={"LAYER 1 SEMI", SJ6_SEMI_MAX, SJ6_SEMI_CENTER, 0},
        [SJ6_LAYER2_SEMI]={"LAYER 2 SEMI", SJ6_SEMI_MAX, SJ6_SEMI_CENTER, 0},
        [SJ6_LAYER1_FINE]={"LAYER 1 FINE", SJ6_FINE_MAX, SJ6_FINE_CENTER, 0},
        [SJ6_LAYER2_FINE]={"LAYER 2 FINE", SJ6_FINE_MAX, SJ6_FINE_CENTER, 0},
        [SJ6_OSC2_LEVEL]={"OSC 2 LVL",127,127,0}, [SJ6_OSC1_LEVEL]={"OSC 1 LVL",127,127,0},
        [SJ6_FILTER_VELOCITY]={"VELOCITY",127,0,0},
#if X0X_SJ6_VCF_PORT
        [SJ6_VCF_LFO1_AMOUNT]={"LFO1 AMOUNT",127,0,0},
#endif
        [SJ6_LFO2_PWM_BASE]={"PWM BASE",SJ6_PWM_STEPS,0,0},
'''
body=body.replace('    static const x0x_param_t knobs', '''    static const char *const pwm_route[]={"OFF","PWM 1","PWM 2"};
    static const char *const clock_mode[]={"FREE","SYNC"};
    static const char *const lfo_wave[]={"TRI","RAMP UP","RAMP DOWN","SQUARE","S&H"};
    static const x0x_param_t knobs''')
specials=original_lines(362,411).replace('slot<=SJ6_DELAY26','slot<=SJ6_DELAY27')
one('static const x0x_param_t *pref_desc(pref_t r)', '''#if X0X_SUPERJ6_BUILTIN_INIT
static const x0x_param_t *sj6_knob_desc(unsigned slot)
{
'''+specials+body+defs+'''    };
    return slot<sizeof knobs/sizeof *knobs && knobs[slot].name ? &knobs[slot]:0;
}
#endif
static const x0x_param_t *pref_desc(pref_t r)''')
one('    case R_NOTELOCK: return &NOTELOCK_P;', '''#if X0X_SUPERJ6_BUILTIN_INIT
    case R_SJ6KNOB:return sj6_knob_desc(r.c);
#endif
    case R_NOTELOCK: return &NOTELOCK_P;''')
one('    case R_NOTELOCK: return engine_note_lock_capture_on();', '''#if X0X_SUPERJ6_BUILTIN_INIT
    case R_SJ6KNOB:{int p=engine_superj6_knob_position(r.b,r.c);return p<0?0:p;}
#endif
    case R_NOTELOCK: return engine_note_lock_capture_on();''')
one('    if (r.kind == R_BSTATUS) return 1;', '''#if X0X_SUPERJ6_BUILTIN_INIT
    if(r.kind==R_SJ6KNOB){
        if(r.b>=NBASS || engine_bass_bank(r.b)!=ORIGINAL_SOUND_JUNO ||
           engine_superj6_knob_position(r.b,r.c)<0)return 1;
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
        if(r.c>=SJ6_KNOBS && (!live137(&proj.sound,r.b) || !s1_typed_tuple_editable()))return 1;
#endif
        return 0;
    }
#endif
    if (r.kind == R_BSTATUS) return 1;''')
one('    pattern_t *p = pref_pattern(r);\n    const x0x_param_t *d = pref_desc(r);','''#if X0X_SUPERJ6_BUILTIN_INIT
    if(r.kind==R_SJ6KNOB){
        const x0x_param_t *d=pref_desc(r);
        if(d && (unsigned)v<=d->max && !pref_read_only(r) &&
           engine_superj6_knob_turn(r.b,r.c,v,0))mark_dirty();
        return;
    }
#endif
    pattern_t *p = pref_pattern(r);
    const x0x_param_t *d = pref_desc(r);''')
one('#define MAXPAGES 14', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#define MAXPAGES 32 /* Archived all-controls source page capacity; fit not assessed here. */
#else
#define MAXPAGES 14
#endif''')
one('    if(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE) {\n        static const x0x_param_t s1_direction',original_lines(1631,1639)+'''#if X0X_SJ6_ARP_ADAPTER_NATIVE && X0X_SUPERJ6_BUILTIN_INIT && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(owner<NBASS && live137(&proj.sound,owner) && engine_bass_bank(owner)==ORIGINAL_SOUND_JUNO){
        static const char *const types[]={"UP","DOWN","UP+DN","UP+DN X2","RANDOM","RANDOM2","RANDOM3"};
        static const x0x_param_t mode={"MODE",6,0,types}, speed={"SPEED",12,SJ6_RATE_DEFAULT,0},
                                     seq_enable={"SEQ ON",1,0,ONOFF_N};
        if(field==ARP_F_MODE)return &mode;
        if(field==ARP_F_SPEED)return &speed;
        if(field==ARP_F_SEQ_ENABLE)return &seq_enable;
    }
#endif
#if X0X_SJ6_ARP_ADAPTER_NATIVE && X0X_SUPERJ6_BUILTIN_INIT
    if(engine_bass_bank(owner)==ORIGINAL_SOUND_SYNTHONE) {
        static const x0x_param_t s1_direction''')
one('    return field<sizeof(ARP_P)/sizeof(ARP_P[0])', '#endif\n    return field<sizeof(ARP_P)/sizeof(ARP_P[0])')
one('    case ARP_F_SEQ_ENABLE: return s.seq_on;','#if X0X_SJ6_ARP_ADAPTER_NATIVE && X0X_SUPERJ6_BUILTIN_INIT\n    case ARP_F_SEQ_ENABLE: return s.seq_on;\n#endif')
one('    ph = (as.on || (as.seq_on && engine_bass_bank(b)!=ORIGINAL_SOUND_SYNTHONE)) && engine_arp_view','''#if X0X_SJ6_ARP_ADAPTER_NATIVE && X0X_SUPERJ6_BUILTIN_INIT
        int sounding=as.on || (as.seq_on && engine_bass_bank(b)!=ORIGINAL_SOUND_SYNTHONE);
#else
        int sounding=as.on;
#endif
        ph = sounding && engine_arp_view''')
one('static int arp_get(unsigned voice, unsigned field)\n{','''static int arp_get(unsigned voice, unsigned field)
{
#if X0X_S1_FENCED_CLOCK
    if(field==ARP_F_SOURCE_RATE || field==ARP_F_SOURCE_FACTOR){
        int value=engine_arp_source_get(voice,field);
        return value<0?0:field==ARP_F_SOURCE_RATE?value-1:18-value;
    }
#endif''')
one('static void arp_set(unsigned voice, unsigned field, int v)\n{','''static void arp_set(unsigned voice, unsigned field, int v)
{
    const x0x_param_t *d=arp_desc(voice,field);
    if(!d || (unsigned)v>d->max)return;
#if X0X_S1_FENCED_CLOCK
    if(field==ARP_F_SOURCE_RATE || field==ARP_F_SOURCE_FACTOR){
        if(engine_arp_source_set(voice,field,field==ARP_F_SOURCE_RATE?v+1:18-v))mark_dirty();
        else say("ARP NOT CHANGED",0);
        return;
    }
#endif
#if X0X_SJ6_ARP_ADAPTER_NATIVE && X0X_SUPERJ6_BUILTIN_INIT && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    if(field==ARP_F_MODE && live137(&proj.sound,voice) &&
       engine_bass_bank(voice)==ORIGINAL_SOUND_JUNO && v>=ARP_SEQ)++v;
#endif''')
one('    case ARP_F_MODE: return s.mode;', '''    case ARP_F_MODE:
#if X0X_SJ6_ARP_ADAPTER_NATIVE && X0X_SUPERJ6_BUILTIN_INIT && DNBOX_TYPED_CONTROLS_NATIVE_TEST
        if(live137(&proj.sound,voice) && engine_bass_bank(voice)==ORIGINAL_SOUND_JUNO)
            return s.mode>ARP_SEQ?s.mode-1u:s.mode<ARP_SEQ?s.mode:0;
#endif
        return s.mode;''')
one('    if (r.kind == R_ARP && r.a == ARP_F_GATE)', '''#if X0X_S1_FENCED_CLOCK
    if (r.kind == R_ARP && r.a == ARP_F_SOURCE_RATE) {
        put_i(num,v+1);put_s(unit,"BPM");return;
    }
#endif
    if (r.kind == R_ARP && r.a == ARP_F_GATE)''')
one('            add_page(emit, "SOUND",PR(R_BENGINE','''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(kind==ORIGINAL_SOUND_SYNTHONE && live_s1_mod(&proj.sound,b)){
#if X0X_S1_FENCED_CLOCK && X0X_ARP_HOOKS
                add_page(emit,"ARP PARAMS",PR(R_ARP,ARP_F_SOURCE_RATE,b,0),PR(R_ARP,ARP_F_SOURCE_FACTOR,b,0),NONE,NONE);
#endif
                add_page(emit,"DELAY",
#if X0X_S1_FEEDBACK_PROBE
                    PR(R_S1DLFB,T_303,b,0),
#else
                    NONE,
#endif
#if X0X_S1_DELAY_TRACK_TEST
                    PR(R_S1DICT,T_303,b,0),
#else
                    NONE,
#endif
#if X0X_S1_DELAY_TIME_NATIVE
                    PR(R_S1DLYTIME,T_303,b,0),
#else
                    NONE,
#endif
                    PR(R_S1DLYRES,T_303,b,0));
                add_page(emit,"TUNING",PR(R_S1A4,T_303,b,0),PR(R_S1TRANS,T_303,b,0),
                    PR(R_S1AA,T_303,b,0),
#if X0X_S1_KEYBOARD_HOLD && X0X_S1_SCALE_NATIVE
                    PR(R_S1A4RESET,T_303,b,0));
#else
                    NONE);
#endif
                add_page(emit,"REVERB",PR(R_S1RVFB,T_303,b,0),NONE,NONE,NONE);
                add_page(emit,"FILTER MOD",PR(R_S1ENVLFO,T_303,b,0),NONE,NONE,NONE);
                add_page(emit,"WHEELS",PR(R_S1MODWHEEL,T_303,b,0),NONE,NONE,NONE);
#if X0X_S1_CRUSH_LFO_PROBE
                add_page(emit,"CRUSH MOD",PR(R_S1CRUSH,T_303,b,0),NONE,NONE,NONE);
#endif
#if X0X_S1_KEYBOARD_HOLD
                add_page(emit,"KEY HOLD",PR(R_S1KEY_HOLD,T_303,b,0),
#if X0X_S1_SCALE_NATIVE
                    PR(R_S1SCALE,T_303,b,0),
#else
                    NONE,
#endif
#if X0X_S1_HALFTIME
                    PR(R_S1HALF,T_303,b,0),
#else
                    NONE,
#endif
                    NONE);
#endif
            }
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
one('            for (int row = 0; row < count; row++) {\n                pref_t refs[4];', '''#if X0X_SUPERJ6_BUILTIN_INIT
            if(kind==ORIGINAL_SOUND_JUNO){
#define K(k) PR(R_SJ6KNOB,T_303,b,k)
#define E(k) PR(R_ENG,T_303,b,k)
                add_page(emit,"OSCILLATOR",K(SJ6_WAVE1),K(SJ6_WAVE2),K(SJ6_RANGE),K(SJ6_A_REFERENCE));
                add_page(emit,"PULSE WIDTH",K(SJ6_PWM1_ON),K(SJ6_PWM1),K(SJ6_PWM2_ON),K(SJ6_PWM2));
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST && X0X_SJ6_VCF_PORT
                add_page(emit,"FILTER",E(2),E(3),PR(R_SJ6FILTER_EXTRA,T_303,b,1),K(SJ6_FILTER_LINK));
                add_page(emit,"FILTER MOD",
#if X0X_SJ6_VELOCITY_PORT
                         K(SJ6_FILTER_VELOCITY),
#else
                         NONE,
#endif
                         K(SJ6_VCF_LFO1_AMOUNT),PR(R_SJ6FILTER_EXTRA,T_303,b,0),NONE);
#else
                add_page(emit,"FILTER",E(2),E(3),E(4),K(SJ6_FILTER_LINK));
#endif
                add_page(emit,"FILTER ADSR",K(SJ6_FILTER_ATTACK),K(SJ6_FILTER_DECAY),K(SJ6_FILTER_SUSTAIN),K(SJ6_FILTER_RELEASE));
                add_page(emit,"AMP ADSR",K(SJ6_AMP_ATTACK),K(SJ6_AMP_DECAY),K(SJ6_AMP_SUSTAIN),K(SJ6_AMP_RELEASE));
#if X0X_SJ6_NOISE_SUB_NATIVE
                add_page(emit,"OSC MIX",K(SJ6_NOISE_LEVEL),K(SJ6_SUB_LEVEL),NONE,NONE);
#endif
                add_page(emit,"OUTPUT",E(9),K(SJ6_MASTER_LEVEL),K(SJ6_OSC1_LEVEL),K(SJ6_OSC2_LEVEL));
                add_page(emit,"LFO RATE",K(SJ6_LFO1_RATE),K(SJ6_LFO2_RATE),K(SJ6_LFO3_RATE),K(SJ6_LFO2_SHAPE));
                add_page(emit,"LFO MODE",K(SJ6_LFO1_MODE),K(SJ6_LFO2_MODE),K(SJ6_LFO3_MODE),
#if X0X_SJ6_LFO1_DELAY_PORT
                         K(SJ6_LFO1_DELAY));
#else
                         NONE);
#endif
                add_page(emit,"LFO2 PWM",K(SJ6_LFO2_RATE),K(SJ6_LFO2_PWM_DEPTH),K(SJ6_LFO2_PWM_ROUTE),K(SJ6_LFO2_PWM_BASE));
#if X0X_SJ6_LFO1_WAVE_NATIVE
                add_page(emit,"LFO",K(SJ6_LFO1_ENABLE),K(SJ6_LFO1_SNAPBACK),K(SJ6_LFO1_WAVE),NONE);
#else
                add_page(emit,"LFO",E(10),E(11),NONE,NONE);
#endif
                add_page(emit,"CHORUS",K(SJ6_CHORUS_I),K(SJ6_CHORUS_II),K(SJ6_CHORUS_RATE),K(SJ6_CHORUS_MIX));
#if X0X_SJ6_NATIVE_DELAY
                add_page(emit,"DELAY",K(SJ6_DELAY24),K(SJ6_DELAY25),K(SJ6_DELAY26),K(SJ6_DELAY27));
                add_page(emit,"DELAY CLOCK",K(SJ6_DELAYSYNC),NONE,NONE,NONE);
#endif
                add_page(emit,"VIBRATO",K(SJ6_VIBRATO_DEPTH),K(SJ6_VIBRATO_RATE),NONE,NONE);
                add_page(emit,"TUNING",K(SJ6_LAYER1_SEMI),K(SJ6_LAYER2_SEMI),K(SJ6_LAYER1_FINE),K(SJ6_LAYER2_FINE));
#undef E
#undef K
            } else
#endif
            for (int row = 0; row < count; row++) {
                pref_t refs[4];''')
one('            add_page(emit, "SOUND",PR(R_BENGINE','''#if X0X_SUPERJ6_BUILTIN_INIT && DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(kind==ORIGINAL_SOUND_JUNO && live137(&proj.sound,b)){
#define J(k,c) PR(k,T_303,b,c)
#if X0X_SJ6_NATIVE_UNISON
                if(engine_sj6_unison_get(b,0)>=0)
                    add_page(emit,"UNISON",J(R_SJ6UNISON,0),J(R_SJ6UNISON,1),J(R_SJ6UNISON,2),J(R_SJ6UNISON,3));
#endif
#if X0X_SJ6_NATIVE_J60
                if(engine_sj6_j60_get(b,0)>=0)
                    add_page(emit,"CHORUS J60",J(R_SJ6J60,0),J(R_SJ6J60,1),J(R_SJ6J60,2),J(R_SJ6J60,3));
#endif
#if X0X_SJ6_PITCH_ENV_NATIVE
                add_page(emit,"PITCH ADSR",J(R_SJ6PITCH,0),J(R_SJ6PITCH,1),J(R_SJ6PITCH,2),J(R_SJ6PITCH,3));
                add_page(emit,"PITCH MOD",J(R_SJ6PITCH,4),NONE,NONE,NONE);
#endif
#if X0X_SJ6_NATIVE_ROOM
                add_page(emit,"EFFECT",J(R_SJ6CRUSH,0),J(R_SJ6CRUSH,1),NONE,NONE);
#endif
#if X0X_SJ6_OTA_NATIVE
                add_page(emit,"FILTER MODE",J(R_SJ6OTA,0),NONE,NONE,NONE);
#endif
#if X0X_SJ6_AUX_LFO_NATIVE
                add_page(emit,"LFO SETUP",J(R_SJ6AUX,0),J(R_SJ6AUX,1),J(R_SJ6AUX,2),J(R_SJ6AUX,3));
                add_page(emit,"LFO ROUTING",J(R_SJ6AUX,5),J(R_SJ6AUX,6),J(R_SJ6AUX,7),J(R_SJ6AUX,8));
                add_page(emit,"LFO CONTROL",J(R_SJ6AUX,14),J(R_SJ6AUX,4),
#if X0X_SJ6_LFO_TRIGGER_NATIVE
                         J(R_SJ6AUX,12),J(R_SJ6AUX,13));
                add_page(emit,"LFO TRIGGER",J(R_SJ6AUX,9),J(R_SJ6AUX,10),J(R_SJ6AUX,11),NONE);
#else
                         NONE,NONE);
#endif
#endif
#undef J
            }
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
def historical_result(first,last):
    return '\n'.join(l[1:] for l in historical[first-1:last] if l.startswith(('+',' ')))+'\n'
start=s.index('static void do_undo(int redo)\n{')
end=s.index('/* SAVE while playing',start)
base_undo=s[start:end]
undo=base_undo
def u(a,b):
    global undo
    assert undo.count(a)==1,(a[:60],undo.count(a))
    undo=undo.replace(a,b)
u('    undo_wait=0;undo_bass_mask=0;\n    int entry=redo?undo.cur:undo.cur-1;\n    if(entry>=0 && entry<undo.n) {',
  historical_result(2621,2729).rstrip())
# Layer the forward joined source hunks inside the original archive validation.
fields=historical_result(2732,3076)
fields=fields.replace('                if(memcmp(lim,sj6_limiter_read(&proj.sound,v),sizeof lim))undo_limiter|=1u<<v;',
'''                if(memcmp(lim,sj6_limiter_read(&proj.sound,v),sizeof lim))undo_limiter|=1u<<v;
#if X0X_SJ6_VINTAGE_NATIVE
                if(!undo_bounded_field(entry,lim_off+SJ6_VINTAGE_BYTE-SJ6_LIM_BYTE,127u,
                    sj6_vintage_read(&proj.sound,v),1u<<v,&undo_limiter,"UNDO VINTAGE INVALID"))return;
#endif
#if X0X_SJ6_VELOCITY_SETTINGS
                if(!undo_bounded_field(entry,lim_off+SJ6_INPUT_VEL_BYTE-SJ6_LIM_BYTE,SJ6_INPUT_VEL_MAX,
                    sj6_input_vel(&proj.sound,v),1u<<v,&undo_sj6_input_vel,"UNDO VELOCITY INVALID"))return;
#endif''')
fields=fields.replace('if(p>S1C_SCALE_MAX || (p && !s1_typed_init_cell(target.cell[v]))){','''if(p>S1C_SCALE_MAX || (p && !s1_typed_init_cell(target.cell[v])
#if X0X_SJ6_SCALE_NATIVE
                   && !(target.cell[v][29]==ORIGINAL_SOUND_JUNO && !original_sound_sj6_factory(target.cell[v]))
#endif
                   )){''')
u('            if(memcmp(target.cell[v],proj.sound.v[T_303][v],NPARAMS_MAX)', fields+'            if(memcmp(target.cell[v],proj.sound.v[T_303][v],NPARAMS_MAX)')
u('    unsigned typed_hold_changes=0;','    unsigned typed_hold_changes=0,undo_sj6_input_vel=0;')
pre=historical_result(3080,3092).replace('| undo_zero;','| undo_zero | undo_sj6_input_vel;')+'        }\n#endif\n'
u('        if(undo_bass_mask) {',pre+'        if(undo_bass_mask) {')
u('#endif\n    undo_lanes = 0;', '#endif\n'+historical_result(3097,3171)+'    undo_lanes = 0;')
u('        if(undo_bass_mask)engine_project_hold(0);\n#endif','''        if(undo_bass_mask)engine_project_hold(0);
#ifdef DNBOX_137_HOST_WITNESS
        else if(typed_hold_changes)engine_project_hold(0);
#endif
#endif''')
u('    if(undo_bass_mask) {engine_bass_base_restore(undo_bass_mask);engine_project_hold(0);}\n#endif',
   historical_result(3184,3258).rstrip())
helpers=historical_result(2567,2588)
one('static undo_t undo X0X_POOL;','static undo_t undo X0X_POOL;\n'+helpers.rstrip())
# Archived helpers require the same editing scopes and live audio holds; declare
# actual measured-port callbacks without importing the engine implementation.
undocb=set(re.findall(r'\b([a-zA-Z_]\w*(?:_undo|_undo_ready|_undo_apply))\(',undo)) - {'do_undo','undo_bounded_field','undo_undo'}
protos=[]
for cb in sorted(undocb):
    if f'extern int {cb}(' in undo or f'extern void {cb}(' in undo:continue
    m=re.search(r'^(?:X0X_COLD )?((?:int|void) '+re.escape(cb)+r'\([^{};]*\))\s*\{',allimpl,re.M)
    if m: protos.append('extern '+m[1]+';')
one(base_undo,'\n'.join(protos)+'\n'+historical_result(2599,2612)+undo)
one('#define MAXPAGES 32 /* Archived all-controls source page capacity; fit not assessed here. */',
    '#define MAXPAGES 36 /* 33 current J6 pages plus source-join slots. Fit not assessed here. */')
one('#else\n#define MAXPAGES 14\n#endif\n/* page groups:', '#elif X0X_SUPERJ6_BUILTIN_INIT\n#define MAXPAGES 24\n#else\n#define MAXPAGES 14\n#endif\n/* page groups:')
historical_ui=Path('../source-ui-exact/historical-best.c').read_text()
e=historical_ui.index('            undo_peek(&undo,entry,tag,target.tag[v].bytes,BASS_SOUND_TAG_BYTES);',historical_ui.index('static void do_undo'))
es=historical_ui.index('\n',e)+1
ee=historical_ui.index('            if(memcmp(target.cell[v]',es)
extension_checks=historical_ui[es:ee].split("#ifdef DNBOX_137_HOST_WITNESS",1)[0]
assert '#ifdef DNBOX_137_HOST_WITNESS' not in extension_checks
one('            undo_peek(&undo,entry,tag,target.tag[v].bytes,BASS_SOUND_TAG_BYTES);\n',
    '            undo_peek(&undo,entry,tag,target.tag[v].bytes,BASS_SOUND_TAG_BYTES);\n'+extension_checks)
one('        if(undo_s1_mod && engine_bass_wheel_active()) {', '''        /* No live per-group publication API exists for these three historical
         * sidecars. Use the existing held base restore on the affected
         * owners, including its active-note/lock/input admission. */
        undo_bass_mask |= undo_delay | undo_sj6_unison | undo_j60;
        if(undo_s1_mod && engine_bass_wheel_active()) {''')
one('    if ((typed_hold_changes) && !undo_bass_mask) engine_project_hold(1);','    if ((typed_hold_changes) && !undo_bass_mask) engine_project_hold(1);')
one('    return field<sizeof(ARP_P)/sizeof(ARP_P[0])', '''#if X0X_SUPERJ6_BUILTIN_INIT
    if(owner<NBASS && engine_bass_bank(owner)==ORIGINAL_SOUND_JUNO){
        static const char *const beats[]={"SPEED","1/16","3/32","1/8","3/16","1/4","3/8","1/2",
            "3/4","1","1.5","2","3","4","8","12","16","24","32"};
        static const x0x_param_t division={"BEATS",ARP_SAGA_DIVISIONS,0,beats};
        if(field==ARP_F_DIVISION)return &division;
#if (X0X_SJ6_ZERO_GATE_PROBE || X0X_SJ6_NATIVE_ZERO_GATE) && DNBOX_TYPED_CONTROLS_NATIVE_TEST
        static const x0x_param_t gate={"GATE",100,50,0};
        if(field==ARP_F_GATE && live137(&proj.sound,owner))return &gate;
#endif
    }
#endif
    return field<sizeof(ARP_P)/sizeof(ARP_P[0])''')
one('    case ARP_F_GATE: return s.gate ? s.gate - 1 : 0;', '''    case ARP_F_GATE:
#if (X0X_SJ6_ZERO_GATE_PROBE || X0X_SJ6_NATIVE_ZERO_GATE) && DNBOX_TYPED_CONTROLS_NATIVE_TEST
        if(live137(&proj.sound,voice))return s.gate;
#endif
        return s.gate?s.gate-1:0;
#if X0X_SUPERJ6_BUILTIN_INIT
    case ARP_F_DIVISION:
#if X0X_SJ6_ARP_ADAPTER_NATIVE
        return s.division>=SJ6_RATE_FIRST?0:s.division;
#else
        return s.division;
#endif
#endif''')
one('    if (field == ARP_F_RANGE || field == ARP_F_LENGTH || field == ARP_F_GATE) v++;', '''    int typed_gate=0;
#if (X0X_SJ6_ZERO_GATE_PROBE || X0X_SJ6_NATIVE_ZERO_GATE) && DNBOX_TYPED_CONTROLS_NATIVE_TEST
    typed_gate=field==ARP_F_GATE && live137(&proj.sound,voice);
    if(typed_gate && !s1_typed_tuple_editable())return;
#endif
    if(field==ARP_F_RANGE || field==ARP_F_LENGTH || (field==ARP_F_GATE && !typed_gate))v++;''')
one('                pref_t seq_enable=NONE;','''                pref_t seq_enable=NONE;
#if X0X_SUPERJ6_BUILTIN_INIT
                if(engine_bass_bank(b)==ORIGINAL_SOUND_JUNO)
                    seq_enable=PR(R_ARP,ARP_F_DIVISION,b,0);
#endif''')
one('    if (r.kind == R_ARP && r.a == ARP_F_GATE) {','''    if (r.kind == R_ARP && r.a == ARP_F_GATE) {
#if (X0X_SJ6_ZERO_GATE_PROBE || X0X_SJ6_NATIVE_ZERO_GATE) && DNBOX_TYPED_CONTROLS_NATIVE_TEST
        if(live137(&proj.sound,r.b)){put_i(num,v);put_s(unit,"%");return;}
#endif''')
one('    if(r.kind==R_SJ6LIMITER && r.c==SJ6_VINTAGE_SLOT){','''    if(r.kind==R_SJ6LIMITER && r.c==SJ6_VINTAGE_SLOT){
        *num=*unit=0;''')
one('    case R_SJ6LIMITER: {\n        static const x0x_param_t rage=', '''    case R_SJ6LIMITER: {
        static const x0x_param_t p[SJ6_LIM_COUNT]={{"THRESHOLD",127,57,0},{"HEADROOM",127,0,0},
            {"ATTACK",127,0,0},{"GAIN",127,64,0}};
        if(r.c<SJ6_LIM_COUNT)return &p[r.c];
        static const x0x_param_t rage=''')
one('            add_page(emit, "SOUND",PR(R_BENGINE','''#if X0X_SJ6_VINTAGE_NATIVE && X0X_SJ6_ALT3_NATIVE && DNBOX_TYPED_CONTROLS_NATIVE_TEST
            if(kind==ORIGINAL_SOUND_JUNO && sj6_native_limiter_get(b,SJ6_LIM_THRESH)>=0)
                add_page(emit,"LIMITER",PR(R_SJ6LIMITER,T_303,b,SJ6_LIM_THRESH),
                         PR(R_SJ6LIMITER,T_303,b,SJ6_LIM_HEADROOM),PR(R_SJ6LIMITER,T_303,b,SJ6_LIM_ATTACK),
                         PR(R_SJ6LIMITER,T_303,b,SJ6_LIM_GAIN));
#endif
            add_page(emit, "SOUND",PR(R_BENGINE''')
inject('    const x0x_param_t *d = pref_desc(r);\n    *num = *unit = 0;','''#if X0X_SJ6_NATIVE_ROOM
    extern float sj6_room_locut(unsigned);
    extern float sj6_room_rate_clocked(unsigned,float,int);
    *num=*unit=0;
'''+original_lines(1137,1151)+'''#endif
#if X0X_SJ6_OTA_NATIVE
    if(r.kind==R_SJ6OTA){*num=*unit=0;put_s(num,v?"JUNO OTA":"NORMAL");return;}
#endif
''')
s=s.replace('ORIGINAL_SJ6_EXT_USED','(ORIGINAL_SJ6_EXTRA_SIZE+ORIGINAL_SJ6_LFO_AUTHORED_SIZE)')
s=s.replace('ORIGINAL_SJ6_EXT_BITS','(ORIGINAL_SJ6_EXTRA_BIT|ORIGINAL_SJ6_AUTHORED_LFO_BIT)')
one('#define LIST_MAX 64', '''#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
#define LIST_MAX (MAXPAGES*4+MAXMORE+4)
#else
#define LIST_MAX 64
#endif''')
p.write_text(s)
