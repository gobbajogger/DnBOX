/* SPDX-License-Identifier: GPL-3.0-only */
/* Complete production fm1_main, with RAM-only flash and panel input.
 * Capture after the real ui_frame() completes each requested main-loop state. */
#define SMC_UI_MAIN retained_smc_ui_main
/* Observe storage enumeration by the separately linked engine, delegating to
 * the existing host slot implementation for the actual image/zone count. */
#define plat_slot_zones startup_host_slot_zones
#include "../host/smc_ui_test.c"
#undef plat_slot_zones
#include <setjmp.h>

static uint8_t flash_memory[0x100000];
static unsigned erases, reads, prog_count, branch, captions, frame_count, owner_kind, irq_ready, clear_calls, cancel_calls, ota_erases, sample_writes;
enum { SAMPLE_EMPTY, SAMPLE_RESTORE, SAMPLE_KEEP_LIVE };
static int sample_case;
static unsigned live_erases, shadow_erases, recovered_zone_scans;

int plat_slot_zones(int k)
{
    int zones = startup_host_slot_zones(k);
    if (sample_case != SAMPLE_EMPTY && k == 0 && seq.pat && zones == 1)
        ++recovered_zone_scans;
    return zones;
}
static int flash_ok, safe_mode, ed_install_pending;
static uint32_t ed_reboot_at, batt_raw, master_q12;
static struct {uint32_t magic,boots,max_us,ui_frames,crash_seen;} x0x_dbg;
static struct {uint32_t magic,count,pc,core;} fm1_crash;
static struct {uint32_t failed,pending;} bootguard;
static struct {uint32_t ota_req,uboot_req;} usb;
static struct {uint32_t buttons;} fm1_in;
static struct {uint8_t btn[NB];} panel;
static jmp_buf completed;
enum { NORMAL, OWNED, HOLD_TO_END, LAST_TICK_RELEASE };
#define fm1_ms now_ms
#define X0X_BLE 1
#define X0X_TRS 1
#define FL_FAR(f) (f)
#define ST_IDLE() ((void)0)
#define FELUCCA_DATA_FAMILY "X0X"
#define FELUCCA_DATA_UNTAGGED_OK 0
#include "normal_constants.inc"

static uint8_t expected_sector[ST_SECTOR];

static uint8_t *fm1_xip_ptr(uint32_t off)
{
    CHECK(off < sizeof flash_memory);
    return flash_memory+off;
}
static int st_read(uint32_t off, void *dst, uint32_t n)
{
    CHECK(off<=sizeof flash_memory && n<=sizeof flash_memory-off);
    memcpy(dst,flash_memory+off,n); ++reads; return 0;
}
static int st_erase(uint32_t off)
{
    CHECK(off>=ST_SMK_BASE && off<=ST_OWNER_SECTOR && off%ST_SECTOR==0);
    memset(flash_memory+off,0xff,ST_SECTOR); ++erases; return 0;
}
static int st_prog(uint32_t off,const void *src,uint32_t n)
{
    CHECK(off==ST_OWNER_SECTOR && n==16);
    memcpy(flash_memory+off,src,n); ++prog_count; return 0;
}
static uint32_t fl_jedec_ram(void) { return 0xeb6014; }
static uint32_t irq_save(void) { return 1; }
static void irq_restore(uint32_t saved) { CHECK(saved==1); }
static void fl_plain_window_init(void) { CHECK(flash_ok); }
static int ota_fread(uint32_t off,void *dst,uint32_t n) { return st_read(off,dst,n); }
static void ota_erase(uint32_t off) {
    CHECK(off>=OTA_AREA && off<OTA_AREA+OTA_AREA_LEN && !(off%ST_SECTOR));
    memset(flash_memory+off,0xff,ST_SECTOR); ++ota_erases;
}
static uint8_t ed_smp_buf[256], smp_sec0_erased[SMP_USER_SLOTS], smp_importing[SMP_USER_SLOTS];
static void fm1_wdt_feed(void);
static int ed_silence_erase4k(uint32_t off,uint32_t *duration) {
    (void)duration;
    CHECK(off==SMP_USER_OFF(0) || off==SMP_SEC0_BACKUP_OFF(0));
    if (off == SMP_USER_OFF(0)) {
        ++live_erases;
        memset(slot_img[0], 0xff, ST_SECTOR);
    } else {
        ++shadow_erases;
        memset(flash_memory+off, 0xff, ST_SECTOR);
    }
    return 0;
}
static int fl_write(uint32_t off,const void *src,uint32_t n) {
    CHECK(off>=SMP_USER_OFF(0) && off+n<=SMP_USER_OFF(0)+ST_SECTOR && n==256);
    CHECK(off == SMP_USER_OFF(0) + sample_writes * n);
    memcpy(slot_img[0]+off-SMP_USER_OFF(0),src,n); ++sample_writes; return 0;
}
static void ed_smp_inval(uint32_t k) { CHECK(k==0); }
static void lcd_init(void)
{
    if (sample_case != SAMPLE_EMPTY) {
        CHECK(!seq.pat && usr_nz[0] == 1 && plat_slot_zones(0) == 1);
        CHECK(!memcmp(slot_img[0], expected_sector, ST_SECTOR));
        CHECK(shadow_erases == 1);
        for (unsigned i = 0; i < ST_SECTOR; ++i)
            CHECK(flash_memory[SMP_SEC0_BACKUP_OFF(0) + i] == 0xff);
        CHECK(live_erases == (sample_case == SAMPLE_RESTORE ? 1u : 0u));
        CHECK(sample_writes == (sample_case == SAMPLE_RESTORE ? ST_SECTOR / sizeof ed_smp_buf : 0u));
    }
    lcd_fill(0,0,240,240,0);
}
static void safe_main(void) { abort(); }
static void fm1_input_init(void) {
    CHECK(flash_ok && (boot_pick(flash_memory+BOOT_SEC0,flash_memory+BOOT_SEC1)>=0)==(branch==OWNED));
    if (sample_case == SAMPLE_RESTORE) shot("startup-recovered-splash");
    else shot(branch!=OWNED?"boot-default":owner_kind?"boot-custom-picture":"boot-custom-text");
}
static void fm1_adc_init(void) {}
static void smk_trim_load(const void *ref) { CHECK(ref==flash_memory+SMK_TRIM_FLASH); }
static void panel_init(void) { panel.btn[B_OCTDN]=FM1_BTN_OCT_DOWN; panel.btn[B_OCTUP]=FM1_BTN_OCT_UP; }
static void panel_setup(void) { abort(); }
static void led_pos_init(void) {}
static void audio_init(void) {}
static void usb_start(void) {}
static void isr_timer5(void) {}
static void fm1_timer5_start(void (*f)(void),unsigned p) { CHECK(f==isr_timer5 && p==4); }
static void fm1_guard_lock_top(void) {}
static void fm1_irq_enable_all(void) { irq_ready=1; }
static void ble_prepare(int on)
{
    CHECK(on==1);
    if (sample_case != SAMPLE_EMPTY) {
        /* The normal startup refresh has run, but the first main-loop refresh
         * and the UI's lazy catalog access have not. Both bank revisions
         * start at zero in breaks_init and the revision getter never scans. */
        CHECK(recovered_zone_scans > 0);
        CHECK(engine_brk_outline_revision(0) && engine_brk_outline_revision(1));
    }
}
static void ble_start(void) {}
static void ble_attach(void) {}
static void uart_midi_init(void) {}
static void fm1_delay_ms(uint32_t ms) { now_ms+=ms; }
static void usb_retry(uint32_t t) { CHECK(t==now_ms); }
static int capture_maintenance_poll(void) { return 0; }
static int32_t fm1_adc_read(unsigned id) { (void)id; return -1; }
static void ed_install_forget(void) {}
static int ed_service(void) { return 0; }
static void usb_detach(void) { abort(); }
static void fm1_reboot(void) { abort(); }
static void ota_service(void) { abort(); }
static void ota_session(void) { abort(); }
static void smk_clip_mode_poll(void) {}
static void smk_ck_poll(void) {}
static void smk_wheels(void) {}
static void smk_grid(void) {}
static void enter_uboot(const char *why) { CHECK(branch==HOLD_TO_END && !strcmp(why,"UBOOT")); longjmp(completed,2); }
static void fm1_wdt_feed(void)
{
    if (!irq_ready) return;
    now_ms = now_ms<400 ? 400 : now_ms+1;
    CHECK(now_ms<8500);
    uint32_t release = branch==HOLD_TO_END?8500:branch==LAST_TICK_RELEASE?6753:6000;
    int held = now_ms>1752 && now_ms<release;
    fm1_in.buttons=held?FM1_OCT_BOTH:0;
    held_btn=held?(1u<<B_OCTDN | 1u<<B_OCTUP):0;
}
static void caption(const char *expected,const char *name)
{
    if (strcmp(ui.msg,expected)) fprintf(stderr, "time %u expected [%s], actual [%s]\n", now_ms, expected, ui.msg);
    CHECK(!strcmp(ui.msg,expected) && (int32_t)(ui.msg_until-now_ms)>0);
    shot(name);
    /* Render an independent font mask at the centered header location. Every
     * ink sample, including its antialiasing, must survive in the final frame. */
    const felucca_font_t *f=&FONT_B;
    CHECK(tw(f,expected)<=206);
    uint16_t base=part_col();
    cv_begin(240,HDR_H,base);
    text_c(107,vc(f,0,HDR_H-1),f,expected,C_BLACK);
    unsigned ink=0;
    for(int y=0;y<HDR_H;y++) for(int x=0;x<240;x++) {
        unsigned i=(unsigned)(240*y+x);
        if(cv_px[i]==swap16(base)) continue;
        CHECK(x>=3 && x<214 && y>=0 && y<HDR_H-1 && fb[i]==cv_px[i]);
        ++ink;
    }
    CHECK(ink>0);
    printf("CAPTION %s %u\n",name,ink);
    ++captions;
}
static void smk_frame(void)
{
    ++frame_count;
    switch(now_ms){
    case 400:
        if (branch == OWNED) {
            CHECK(!st_owner_cleared && !ui.msg_until);
            if (sample_case != SAMPLE_EMPTY) {
                int loop_count = engine_brk_nslots();
                CHECK(seq.pat == proj.pat && loop_count > 0);
                CHECK(!strcmp(engine_brk_slot_names()[loop_count-1], "BR1"));
                shot(sample_case == SAMPLE_RESTORE ? "startup-sample-recovered" : "startup-sample-already-live");
            } else {
                shot("startup-store-owned");
            }
            longjmp(completed,1);
        }
        CHECK(st_owner_cleared && clear_calls==1);
        caption("OLD DATA CLEARED","startup-store-cleared");
        break;
    case 1728: CHECK(!ui.msg_until); shot("startup-notice-expired"); break;
    case 3760: caption("UPDATE MODE IN 3..","update-hold-3"); break;
    case 4752: caption("UPDATE MODE IN 2..","update-hold-2"); break;
    case 5760: caption("UPDATE MODE IN 1..","update-hold-1"); break;
    case 6000:
        if(branch==NORMAL) { CHECK(cancel_calls==1); caption("STEPS 17-32","update-hold-cancelled"); }
        break;
    case 6752:
        if(branch!=NORMAL) caption("UPDATE MODE IN 0..","update-hold-0");
        break;
    case 6768:
        if(branch==LAST_TICK_RELEASE) { CHECK(!cancel_calls); caption("STEPS 17-32","update-last-tick-release"); }
        break;
    case 8160:
        CHECK(!ui.msg_until); shot("update-cancel-expired"); longjmp(completed,1);
    }
}
static void observe_firmware_say(const char *a,const char *b) {
    if(!strcmp(a,"OLD DATA CLEARED")) ++clear_calls;
    if(!strcmp(a,"UPDATE MODE CANCELLED")) ++cancel_calls;
    ui_say(a,b);
}
#define ui_say observe_firmware_say
#include "normal_bodies.inc"
#undef ui_say

static void seed_splash(int picture) {
    uint8_t *record=flash_memory+BOOT_SEC0,*pay=record+BOOT_PAY_OFF;
    int length=picture?BOOT_REC_MAX:BOOT_PAY_FIXED;
    memset(record,0,BOOT_PAY_OFF+length);
    pay[0]=BOOT_KIND_CUSTOM;pay[2]=pay[3]=5;memcpy(pay+4,"DnBOX",5);memcpy(pay+16,"STILL",5);
    if(picture) { pay[1]=BOOT_F_PIC;boot_p16(pay+40,240);boot_p16(pay+42,96);pay[BOOT_PAY_FIXED]=128; }
    boot_p32(record,BOOT_MAGIC);boot_p16(record+4,BOOT_VERSION);boot_p32(record+8,1);
    boot_p32(record+12,length);boot_p32(record+16,boot_crc(pay,length));boot_p32(record+20,boot_crc(record,20));
    CHECK(boot_image_state(record,NULL));
}
/* Build a complete last-good sector with a CRC-checked one-zone sample.
 * Distinct fill bytes distinguish a restored shadow from the stale backup
 * that must be discarded when the live sector is already valid. */
static void sample_sector(uint8_t *sector, uint8_t fill)
{
    smp_user_hdr_t h = {
        .magic = SMP_USER_MAGIC, .version = 1, .nz = 1, .name = "RECOVER",
        .data_len = 128, .zone = {{ .n = 256, .rate = 1u << 16, .hi = 127 }}
    };
    memset(sector, fill, ST_SECTOR);
    h.crc = st_crc32(sector + SMP_USER_DATA, h.data_len);
    memcpy(sector, &h, sizeof h);
}

static void seed_pending_sample(void)
{
    uint8_t *shadow = flash_memory + SMP_SEC0_BACKUP_OFF(0);
    sample_sector(expected_sector, 0x36);
    slot_is_img[0] = 1;
    if (sample_case == SAMPLE_RESTORE) {
        memcpy(shadow, expected_sector, ST_SECTOR);
        CHECK(!smp_header_valid(smp_hdr(slot_img[0]), 0));
    } else {
        memcpy(slot_img[0], expected_sector, ST_SECTOR);
        sample_sector(shadow, 0x59);
        CHECK(smp_header_valid(smp_hdr(slot_img[0]), 0));
    }
    CHECK(ed_smp_backup_present(0));
}

int main(int argc,char **argv)
{
    (void)argc;(void)argv;
    const char *mode=getenv("STARTUP_CASE");
    owner_kind=!strcmp(mode,"picture");
    sample_case = !strcmp(mode, "recover") ? SAMPLE_RESTORE :
                  !strcmp(mode, "live-shadow") ? SAMPLE_KEEP_LIVE : SAMPLE_EMPTY;
    branch=(sample_case != SAMPLE_EMPTY || owner_kind || !strcmp(mode,"owned"))?OWNED:!strcmp(mode,"hold")?HOLD_TO_END:!strcmp(mode,"last-release")?LAST_TICK_RELEASE:NORMAL;
    memset(flash_memory,0xff,sizeof flash_memory);
    memcpy(flash_memory+ST_OWNER_SECTOR,branch==OWNED?"SMK37DATAX0X":"SMK37DATAOLD",12);
    memset(flash_memory+ST_OWNER_SECTOR+12,0,4);
    memset(slot_img,0xff,sizeof slot_img);
    if (sample_case != SAMPLE_EMPTY) seed_pending_sample();
    seed_splash(owner_kind);
    if(branch==NORMAL) flash_memory[OTA_AREA]=0;
    int exit_code=setjmp(completed);
    if(!exit_code) { fm1_main(); abort(); }
    CHECK(flash_ok && reads>=3 && !memcmp(flash_memory+ST_OWNER_SECTOR,"SMK37DATAX0X",12));
    if(branch==OWNED) CHECK(exit_code==1 && !captions && !erases && !prog_count && frame_count==1);
    else {
        CHECK(erases==(ST_CLEAR_HI-ST_SMK_BASE)/ST_SECTOR+1 && prog_count==1);
        CHECK(branch==HOLD_TO_END ? exit_code==2 && captions==5 :
              exit_code==1 && captions==(branch==NORMAL?5u:6u));
    }
    if(branch==NORMAL) {
        CHECK(!sample_writes && !ed_smp_backup_present(0));
        CHECK(ota_erases==5 && flash_memory[OTA_AREA]==255);
    } else {
        CHECK(!ota_erases);
        if (sample_case == SAMPLE_EMPTY) CHECK(!sample_writes);
    }
    puts("PASS complete fm1_main host path");
    return 0;
}
