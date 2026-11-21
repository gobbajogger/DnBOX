/* SPDX-License-Identifier: GPL-3.0-only */
/* The runner supplies unchanged production recovery function bodies. All
 * board-facing interfaces below terminate at a host RAM input or output. */
#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "app/plat.h"
#include "route_types.inc"

static uint8_t pixels[240][240][2];
static unsigned cmd, x0, y0, x1, y1, position, hw_init, display_on, baud_set;
static unsigned word(const uint8_t *p) { return (unsigned)p[0] * 256 + p[1]; }
static void fm1_lcd_send_cmd(uint8_t c)
{
    if (cmd == 0x2c) assert(position == (x1-x0+1)*(y1-y0+1));
    cmd = c;
    if (c == 0x2c) position = 0;
    if (c == 0x29) ++display_on;
}
static void fm1_lcd_send_data(const void *data, unsigned size)
{
    const uint8_t *p = data;
    if (cmd == 0x2a || cmd == 0x2b) {
        assert(size == 4 && word(p) <= word(p+2) && word(p+2) < 240);
        if (cmd == 0x2a) { x0 = word(p); x1 = word(p+2); }
        else { y0 = word(p); y1 = word(p+2); }
    } else if (cmd == 0x2c) {
        unsigned width = x1-x0+1;
        assert(size % 2 == 0 && position + size/2 <= width*(y1-y0+1));
        for (unsigned k = 0; k < size; k += 2, ++position)
            memcpy(pixels[y0+position/width][x0+position%width], p+k, 2);
    }
}
static void fm1_lcd_hw_init(void) { ++hw_init; }
static void fm1_lcd_baud(unsigned n) { assert(n == 4); ++baud_set; }
static void fm1_lcd_wait(void) {}
static void fm1_lcd_deselect(void) {}
static void fm1_padled_send(const void *p, unsigned n) { (void)p; (void)n; abort(); }
static void fm1_padled_done(void) { abort(); }
#define SMK_BIG
#include "lcd.c"
#include "gfx.c"

static void capture(const char *name)
{
    char path[128];
    assert(snprintf(path, sizeof path, "%s.rgb565", name) < (int)sizeof path);
    FILE *f = fopen(path, "rb");
    if (f) {
        uint8_t row[sizeof pixels[0]];
        for (unsigned y = 0; y < 240; ++y) {
            assert(fread(row, 1, sizeof row, f) == sizeof row);
            assert(!memcmp(row, pixels[y], sizeof row));
        }
        assert(fgetc(f) == EOF && fclose(f) == 0);
        return;
    }
    f = fopen(path, "wb");
    assert(f && fwrite(pixels, 1, sizeof pixels, f) == sizeof pixels && fclose(f) == 0);
}

static uint32_t fm1_ms, ticks, edges;
static unsigned input_id, knob_id, edge_reads, feeds, saved, detaches, stops, retries;
static int32_t encoder_steps[FM1_NENC];
static struct { uint32_t buttons; } fm1_in;
static struct { int uboot_req, ota_req; } usb;
static struct { uint32_t magic, failed, pending; } bootguard;
static fm1_crash_t fm1_crash;
static int flash_ok;
static jmp_buf reset_destination;
enum scenario { CAL_SUCCESS, CAL_BUTTON_TIMEOUT, CAL_KNOB_TIMEOUT, CAL_HELD_TIMEOUT,
                FAULT, UBOOT, SAFE_PLAY, SAFE_USB, START_NORMAL };
static enum scenario scenario;
static char entry_frame[64], exit_frame[64];

static uint32_t fm1_ticks(void) { ticks += FM1_TICKS_PER_US * 1000000u; return ticks; }
static unsigned fm1_input_edges(int unused)
{
    (void)unused;
    uint32_t result = edges;
    edges = 0;
    ++edge_reads;
    return result;
}
static int fm1_enc_take(unsigned id)
{
    assert(id < FM1_NENC);
    int result = encoder_steps[id];
    encoder_steps[id] = 0;
    return result;
}
static void fm1_delay_ms(uint32_t ms) { fm1_ms += ms; }
static void fm1_wdt_feed(void);
#include "app/panel.c"
static panel_t saved_panel;
int plat_store_load(uint32_t obj, void *dst, uint32_t size)
{
    assert(obj == OBJ_PANEL && size == sizeof saved_panel);
    if (!saved) return -1;
    memcpy(dst, &saved_panel, size);
    return (int)size;
}
int plat_store_save(uint32_t obj, const void *src, uint32_t size)
{
    assert(obj == OBJ_PANEL && size == sizeof saved_panel);
    memcpy(&saved_panel, src, size);
    ++saved;
    return 0;
}

static void fm1_wdt_feed(void)
{
    char name[80];
    assert(++feeds < 200);
    if (scenario == START_NORMAL) return;
    if (scenario == SAFE_PLAY || scenario == SAFE_USB) {
        /* Keep PLAY down across the entry guard, then cross 500 ms. USB
         * recovery instead requests the published bootloader path. */
        fm1_ms += 250;
        if (scenario == SAFE_PLAY) fm1_in.buttons = 1u << panel.btn[B_PLAY];
        else usb.uboot_req = 1;
        return;
    }
    if (!edge_reads) {
        capture("calibration-title");
        if (scenario == CAL_HELD_TIMEOUT) fm1_ms += SETUP_IDLE_MS+1;
        else fm1_in.buttons = 0;
    } else if (input_id < NB) {
        snprintf(name, sizeof name, "hardware-calibration-button-%02u", input_id);
        capture(name);
        if (scenario == CAL_BUTTON_TIMEOUT && input_id == 1) fm1_ms += SETUP_IDLE_MS+1;
        else edges = 1u << input_id++;
    } else {
        assert(knob_id < NE);
        snprintf(name, sizeof name, "hardware-calibration-encoder-%02u", knob_id);
        capture(name);
        if (scenario == CAL_KNOB_TIMEOUT && knob_id == 1) fm1_ms += SETUP_IDLE_MS+1;
        else { encoder_steps[knob_id+1] = knob_id % 2 ? -1 : 1; ++knob_id; }
    }
}
static void fm1_input_init(void) { capture(entry_frame); }
static void fm1_audio_stop(void) { ++stops; }
static void fm1_reboot(void) { capture(exit_frame); longjmp(reset_destination, 1); }
static void fm1_enter_uboot(void) { if (*exit_frame) capture(exit_frame); longjmp(reset_destination, 2); }
static void usb_detach(void) { ++detaches; }
static void usb_start(void) {}
static void isr_timer5(void) {}
static void fm1_timer5_start(void (*fn)(void), unsigned period) { assert(fn == isr_timer5 && period == 1); }
static void fm1_guard_lock_top(void) {}
static void fm1_irq_enable_all(void) {}
static void usb_retry(uint32_t now) { assert(now == fm1_ms); ++retries; }
static int capture_maintenance_poll(void) { return 0; }
static void ota_service(void) {}
static void ota_session(void) { abort(); }
static void ed_install_forget(void) {}
#include "route_bodies.inc"
#include "native_startup.inc"

static void calibration(enum scenario s, const char *last)
{
    scenario = s;
    panel = PANEL_DEFAULT;
    fm1_ms = 0; fm1_in.buttons = 1; saved = input_id = knob_id = feeds = edge_reads = edges = 0;
    memset(encoder_steps, 0, sizeof encoder_steps);
    panel_setup();
    capture(last);
    if (s == CAL_SUCCESS) {
        assert(saved == 1 && panel_valid(&saved_panel) && !memcmp(&panel,&saved_panel,sizeof panel));
        for (unsigned i = 0; i < NB; ++i) assert(panel.btn[i] == i);
        for (unsigned i = 0; i < NE; ++i) assert(panel.enc[i] == i+1 && panel.dir[i] == (i%2 ? -1 : 1));
    } else {
        assert(saved == 0 && !memcmp(&panel,&PANEL_DEFAULT,sizeof panel));
        if (s == CAL_BUTTON_TIMEOUT) assert(input_id == 1);
        if (s == CAL_KNOB_TIMEOUT) assert(knob_id == 1);
        if (s == CAL_HELD_TIMEOUT) assert(!edge_reads);
    }
}
static void recovery(enum scenario s, int core, const char *entry, const char *last)
{
    scenario = s;
    snprintf(entry_frame, sizeof entry_frame, "%s", entry);
    snprintf(exit_frame, sizeof exit_frame, "%s", last);
    fm1_ms = feeds = detaches = stops = retries = ticks = saved = usb.uboot_req = fm1_in.buttons = 0;
    panel = PANEL_DEFAULT;
    bootguard.failed = 2; bootguard.pending = 1;
    fm1_crash = (fm1_crash_t){.magic=core<0 ? 0 : FM1_CRASH_MAGIC,
                              .core=core==1, .pc=0x12345678, .vec=1, .dbg=0x1000};
    int destination = setjmp(reset_destination);
    if (!destination) {
        if (s == FAULT) fm1_fault(&fm1_crash);
        else if (s == UBOOT) enter_uboot("UBOOT");
        else safe_main();
        abort();
    }
    if (s == FAULT) assert(destination == 1 && stops == 1 && ticks >= 5*FM1_TICKS_PER_US*1000000u);
    else if (s == SAFE_PLAY) assert(destination == 1 && !stops && !bootguard.failed && !bootguard.pending &&
                                    retries == 3 && detaches == 1 && fm1_ms == 780);
    else assert(destination == 2 && stops == 1 && detaches == 1 && !bootguard.pending &&
                (s == UBOOT ? fm1_ms == 50 : retries == 1 && fm1_ms == 300));
}
int main(void)
{
    memset(pixels, 0xa5, sizeof pixels);
    lcd_init();
    assert(hw_init == 1 && display_on == 1 && baud_set == 1);
    capture("lcd-init-clear");
    palette_set(1); draw_boot_screen(NULL); capture("boot-default");
    calibration(CAL_HELD_TIMEOUT,"calibration-held-timeout-clear");
    calibration(CAL_BUTTON_TIMEOUT,"calibration-button-timeout-clear");
    calibration(CAL_KNOB_TIMEOUT,"calibration-encoder-timeout-clear");
    calibration(CAL_SUCCESS,"calibration-saved-clear");
    recovery(FAULT,1,"","boot-crash-diagnostics");
    recovery(UBOOT,-1,"","boot-uboot");
    recovery(SAFE_PLAY,-1,"boot-safe-mode","safe-retry-without-crash");
    recovery(SAFE_USB,0,"boot-safe-crash-core-0","boot-uboot-usb");
    recovery(SAFE_PLAY,1,"boot-safe-crash-core-1","boot-safe-retry");
    startup(0, -1, "boot-default");
    startup(0, 0, "boot-post-crash-core-0");
    startup(0, 1, "boot-post-crash-core-1");
    startup(1, -1, "safe-retry-without-crash");
    startup(3, -1, "");
    puts("PASS native route host endpoints");
    return 0;
}
