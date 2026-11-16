/* SPDX-License-Identifier: GPL-3.0-only */
/* Frozen published d13 R22 regression source, not an approved visual golden.
 * Reference snapshot state is isolated; only its cache is disabled. */
static part_header_snapshot_t r22_reference_snapshot;
static uint8_t r22_reference_snapshot_valid;
static const char *const DV_NAME_909_r22_reference[NDRUM] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP", "CLOSED HAT", "OPEN HAT", "CRASH", "RIDE"};
static const char *const DV_NAME_808_r22_reference[NDRUM] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP", "COWBELL", "CYMBAL", "OPEN HAT", "CLOSED HAT"};


static const char *dv_voice_name_r22_reference(int t, int v)
{
    if (v < 0 || v >= NDRUM) return "KIT";
    return t == T_808 ? DV_NAME_808_r22_reference[v] : DV_NAME_909_r22_reference[v];
}

static void dv_full_title_r22_reference(char *t, int part, int v)
{
    put_s(put_s(t, part == PART_808 ? "808 " : "909 "), dv_voice_name_r22_reference(part == PART_808 ? T_808 : T_909, v));
}

static void dv_header_battery_r22_reference(int x, int y)
{
    int lvl = plat_battery();
    int bars, i;
    uint16_t ink = SOUND_GRAY;
    bars = lvl < 0 ? 0 : lvl > 3 ? 3 : lvl;
    if (bars < 0) bars = 0;
    /* r20 battery: 14 x 8 outline, three cells. */
    rbox(x, y, 14, 8, 1, SOUND_FACE);
    cv_rect(x, y, 14, 1, ink);
    cv_rect(x, y+7, 14, 1, ink);
    cv_rect(x, y, 1, 8, ink);
    cv_rect(x+13, y, 1, 8, ink);
    cv_rect(x+14, y+2, 2, 4, ink);
    for (i = 0; i < 3; i++) {
        uint16_t c = (i < bars) ? ink : SOUND_LINE;
        cv_rect(x+2+i*4, y+2, 3, 4, c);
    }
}

static void header_pickup_detail_r22_reference(char *text,int pending)
{
    text[0]=0;
    if(pending)return;
    if(seq.chain_a!=seq.chain_b && !seq.song_on)put_s(text,"CHN");
    else if(steps_view() && steps_len()>16)
        put_i(put_s(put_i(text,ui.spage*16+1),"-"),ui.spage*16+16);
}

static const char *header_save_label_r22_reference(int state)
{
    return project_storage_unsupported()==2?"!":state==SAVE_QUEUED?queued_save_label():
        state==SAVE_FAILED?"SAVE FAILED":"UNSAVED";
}

static UI_DRAW void draw_header_r22_reference(void)
{
#if BOARD_SMK37
    extern uint8_t smk_clip_mode_on(void);
#endif
    int clip_mode = 0;
#if BOARD_SMK37
    clip_mode = smk_clip_mode_on();
#endif
    char b[24];
    int32_t x;
    int ss = save_state();
    int pickup = ui.msg[0] == 'P' && ui.msg[1] == 'I' && ui.msg[2] == 'C' &&
                 ui.msg[3] == 'K' && ui.msg[4] == ' ';
    part_header_snapshot_t snapshot = {0};
    int cacheable = !seq.ext && !ui.msg_until && ui.view == V_PART && (is_drum() || is_303()) &&
        ui.overlay == O_NONE && !ui.help && !division_pad_owner() &&
        seq_cue_of(&seq, ui.part) < 0
#if BOARD_SMK37
        && !smk_clip_mode_on()
#endif
        ;
    cacheable = 0; /* independent uncached regression reference only */
    if (cacheable) {
        snapshot.name = part_display(ui.part);
        if (is_303()) snapshot.name = engine_bass_bank(bidx()) == ORIGINAL_SOUND_JUNO ? (bidx()?"JUNO B":"JUNO A") : engine_bass_bank(bidx()) == ORIGINAL_SOUND_DRONE ?
            (bidx() ? "DRONE B" : "DRONE A") : engine_bass_bank(bidx()) == ORIGINAL_SOUND_MIAMI ?
            (bidx() ? "MIAMI B" : "MIAMI A") : (bidx() ? "CHICAGO B" : "CHICAGO A");
        memcpy(snapshot.colours, pal, sizeof pal);
        snapshot.col = matrix_col(); snapshot.light = ui_theme_light;
        snapshot.part = ui.part; snapshot.sel = is_drum()?(uint8_t)ui.sel[ui.part]:0; snapshot.page = (uint8_t)cur_page(); snapshot.pages = pg.n;
        snapshot.phase = seq.metro_view; snapshot.rec = ui.rec; snapshot.save = (uint8_t)ss;
        snapshot.playing=seq.playing;snapshot.song_on=seq.song_on;
        snapshot.pattern=(uint16_t)(seq.song_on?(seq.playing?seq.song_pos:ui.song_sel):seq.ppat[ui.part]);
        snapshot.tempo=seq_tempo(&seq);snapshot.battery=(uint8_t)plat_battery();
        snapshot.night = (uint8_t)(matrix_three_rows() || X0X_ORIGINAL_SYNTH);
        if (r22_reference_snapshot_valid && blit_hash[0] && cv_tile_valid[0] &&
            !memcmp(&snapshot, &r22_reference_snapshot, sizeof snapshot)) return;
    }
    r22_reference_snapshot_valid = 0;
    cv_begin(240, HDR_H, C_BLACK);
    night_panel(0,0,240,HDR_H,0);
    if (ui.msg_until && (int32_t)(ui.msg_until - plat_ms()) > 0 && !pickup) {   /* what a button just did */
        box(0, 0, 240, HDR_H - 1, part_col());
        if (!memcmp(ui.msg, "NEXT: PAT ", 10)) {
            int pattern=0, n=10;
            while (n<(int)sizeof ui.msg-1 && ui.msg[n]>='0' && ui.msg[n]<='9' && pattern<=NPAT)
                pattern=pattern*10+ui.msg[n++]-'0';
            put_s(put_s(b, "> "), ui.msg+10);
            x=(240-tw(&FONT_B,"NEXT")-6-12-6-tw(&FONT_B,b))/2;
            x=cv_text(x,vc(&FONT_B,0,HDR_H-1),&FONT_B,"NEXT",C_BLACK)+6;
            frame(x,1,12,17,C_BLACK);
            if (!ui.msg[n] && pattern>0 && pattern<=NPAT) {
                for (int pos=0;pos<NPARTS;pos++) {
                    uint8_t strength[8];
                    home_pattern_strength8(&proj.pat[pattern-1],proj.arr.lane,pattern-1,part_at(pos),
                                           proj.set.accent_q7,T_VEL,strength);
                    pat_glyph_row(x+2,2+pos*3,strength,C_BLACK,0);
                }
            }
            cv_text(x+18,vc(&FONT_B,0,HDR_H-1),&FONT_B,b,C_BLACK);
        } else {   /* #800 NEXT pattern glyph; other messages keep #806's long-text font fallback */
            const felucca_font_t *font = tw(&FONT_B, ui.msg) <= 232 ? &FONT_B : &FONT_XS;
            text_c(120, vc(font, 0, HDR_H - 1), font, ui.msg, C_BLACK);
        }
        reference_header_commit();
        return;
    }
    if (!pickup || (int32_t)(ui.msg_until - plat_ms()) <= 0)
        ui.msg_until = pickup = 0;
    { /* All status headers use R22; retain pickup and clip identities below. */
        const char *name=ui.view==V_PART?part_display(ui.part):
            ui.view==V_GLO?list_title:VIEW_N[ui.view];
        char drum_title[24],context_title[32];
        if(ui.view==V_PART && is_303()) {
            name=engine_bass_bank(bidx())==ORIGINAL_SOUND_JUNO?(bidx()?"JUNO B":"JUNO A"):engine_bass_bank(bidx())==ORIGINAL_SOUND_DRONE?(bidx()?"DRONE B":"DRONE A"):
                engine_bass_bank(bidx())==ORIGINAL_SOUND_MIAMI?(bidx()?"MIAMI B":"MIAMI A"):
                (bidx()?"CHICAGO B":"CHICAGO A");
        }
        if(ui.view==V_PART && is_drum()) {
            dv_full_title_r22_reference(drum_title, ui.part, ui.sel[ui.part]);
            name = drum_title;
        }
        if(ui.view==V_SEQ || ui.view==V_GEN) {
            put_s(put_s(put_s(context_title,part_display(ui.part))," "),VIEW_N[ui.view]);
            name=context_title;
        }
        if(clip_mode)name="LP";
        if(pickup)name=ui.msg;
        sound_slab(0,0,240,HDR_H,0);
        /* R22 r20 header: engine + voice in the part colour, not a filled badge. */
        cv_text_clip(4,2,pickup?&FONT_S:&FONT_B,name,pickup?C_AMB:part_view()?part_col():SOUND_WHITE,4,125);
        if((pickup || clip_mode) && ss!=SAVE_SAVED) {
            const char *label=header_save_label_r22_reference(ss);
            const felucca_font_t *font=pickup?&FONT_S:&FONT_B;
            int left=4+tw(font,name)+6,width=tw(&FONT_XS,label)+10;
            uint16_t ink=ss==SAVE_FAILED?RGB(255,70,70):C_AMB;
            if(left+width<=130) {
                box(left,3,width,13,ink);
                text_c(left+width/2,vc(&FONT_XS,3,13),&FONT_XS,label,C_BLACK);
            } else if(left+11<=130) {
                box(left,3,11,13,ink);
                text_c(left+5,vc(&FONT_XS,3,13),&FONT_XS,ss==SAVE_FAILED?"!":ss==SAVE_QUEUED?"Q":"*",C_BLACK);
            }
        }
        if (division_pad_owner() && !pickup && !clip_mode) {
            int dx = 4 + tw(&FONT_B, name) + 8;
            dx = cv_text(dx, 6, &FONT_T, "PADS ", C_AMB);
            cv_text(dx, 6, &FONT_T, division_pad_label() + 11, C_AMB);
        }
#if BOARD_SMK37
        if(clip_mode) {
            extern uint8_t smk_ck_ext(void);
            cv_text(132,4,&FONT_T,smk_ck_ext()?"EXT":"INT",smk_ck_ext()?C_AMB:SOUND_GRAY);
        } else
#endif
        if(pickup || !division_pad_owner()) {
            char pattern[16];
            int header_part=part_view()?ui.part:PART_909;
            int cue=seq_cue_of(&seq,header_part);
            int pending=cue>=0 && cue!=seq.ppat[header_part] && !seq.song_on;
            char *end=put_i(put_s(pattern,seq.song_on?"S":"P"),
                (seq.song_on?(seq.playing?seq.song_pos:ui.song_sel):seq.ppat[header_part])+1);
            if(pending)put_i(put_s(end,">"),cue+1);
            int context_end=cv_text(132,4,&FONT_T,pattern,pending?C_AMB:C_WHITE);
            if(pickup) {
                char detail[12];header_pickup_detail_r22_reference(detail,pending);
                cv_text(context_end+3,4,&FONT_T,detail,SOUND_GRAY);
            }
        }
        {   /* tempo, same readout as the shared header */
            char b[8];
            static float avg; static int shown;
            float tmp = seq_tempo(&seq);
            if (!seq.ext || avg < 1.0f) avg = tmp;
            avg += (tmp - avg) * 0.03f;
            if (!seq.ext || fm_fabsf(avg - (float)shown) > 0.6f)
                shown = (int)(avg + 0.5f);
            put_i(b, shown);
            text_r(pickup?215:200, 3, &FONT_T, b, seq.ext ? C_AMB : C_HI);
        }
        if(clip_mode || pickup) {
            uint8_t phase=seq.metro_view;
            play_icon(pickup?177:168,4,!phase?C_DIM:phase<=4u?C_WHITE:C_HI);
        } else if(seq.playing) play_icon(168,4,C_WHITE);
        else cv_rect(170,7,5,5,SOUND_GRAY);
        if(ui.rec) dot(pickup?190:206,9,3,RGB(255,50,50));
        dv_header_battery_r22_reference(219, 5);
        if(ss!=SAVE_SAVED) dot(pickup?237:211,3,1,ss==SAVE_FAILED?RGB(255,70,70):C_AMB);
        reference_header_commit();
        if (cacheable) { r22_reference_snapshot = snapshot; r22_reference_snapshot_valid = 1; }
        return;
    }
}
