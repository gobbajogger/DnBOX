int main(void)
{
    unsigned n=0, cases=0;
    static ui_original_control_view text[22];
    static uint8_t text_ok[22],wave_ok[22];
    static ui_original_wave_cache wave[22];
    boot();
    for(unsigned theme=0;theme<NPALETTES;theme++)for(unsigned v=0;v<NBASS;v++){
        palette_set(theme);
        ui.view=V_PART;ui.part=v+PART_303A;ui.overlay=O_NONE;ui.help=0;
        describe_pages(&pg,0,ui.view,ui.part,ui.mixsel,ui.sel);
        for(unsigned page=0;page<(unsigned)pg.n;page++){
            ui.page[ui.view][ui.part]=page;
            cv_legacy_semantic s={0};
            if(!cv_capture_legacy_semantic(&s,v))return 2;
            const unsigned count=original_sound_count((unsigned)s.bank);
            if(count>22)return 3;
            for(unsigned id=0;id<count;id++){
                const pref_t r=PR(R_ENG,T_303,v,id);
                const int value=original_display_value(r);
                if(s.shown[id]!=value)return 4;
                const ui_original_control_view *t=original_text_view(r,value);
                if((text_ok[id]=t!=0))text[id]=*t;
                const ui_original_wave_cache *w=original_wave_view(r,value);
                if((wave_ok[id]=w!=0))wave[id]=*w;
            }
            ui_t before=ui;
            palette_set((theme+2)%NPALETTES);ui.part=PART_909;ui.view=V_MIX;ui.overlay=O_ASK;
            memset(proj.sound.v[T_303][v],0xED,sizeof s.sound); /* owner cell no longer readable as former bank */
            for(unsigned id=0;id<count;id++){
                uint8_t preview[22];
                ui_original_control_cache t={0};
                ui_original_wave_cache w={0};
                uint32_t m=cv_legacy_preview(&s,id,s.shown[id],preview,0);
                int ok=ui_original_controls_update(&t,v,s.sound,preview,m);
                if(ok!=text_ok[id] || (ok && memcmp(&t.view,&text[id],sizeof t.view)))return 5;
                m=cv_legacy_preview(&s,id,s.shown[id],preview,1);
                ok=ui_original_wave_update(&w,v,s.sound,preview,m);
                const size_t rows=s.bank==ORIGINAL_SOUND_CHICAGO ? 1u : 3u;
                if(ok!=wave_ok[id] || (ok&&(memcmp(w.wave,wave[id].wave,rows*sizeof w.wave[0])||memcmp(w.zero,wave[id].zero,rows)))) {printf("wave mismatch v=%u bank=%u page=%u id=%u val=%d mask=%u expectedmask=%u ok=%u/%u\n",v,s.bank,page,id,s.shown[id],m,wave[id].preview_mask,ok,wave_ok[id]);return 6;}
                ++n;
            }
            ui=before;palette_set(theme);
            memcpy(proj.sound.v[T_303][v],s.sound,sizeof s.sound);
            ++cases;
        }
    }
    printf("PASS %u exact Chicago/Miami text/wave results after scene changes across %u layouts; frozen record %zu host bytes\n",n,cases,sizeof(cv_legacy_semantic));
    return 0;
}
