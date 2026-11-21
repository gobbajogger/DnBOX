/* SPDX-License-Identifier: GPL-3.0-only */
/* Linker regions are ordinary bounded host arrays. The main prefix selects
 * the production RAM-only startup path when no flash chip is present. */
static uint32_t ram_zero[3][4], ram_copy[3][4];
static uint32_t rom[3][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12}};
#define _bss_start ram_zero[0]
#define _bss_end (_bss_start+4)
#define _pool_start ram_zero[1]
#define _pool_end (_pool_start+4)
#define _xpool_start ram_zero[2]
#define _xpool_end (_xpool_start+4)
#define _data_start ram_copy[0]
#define _data_end (_data_start+4)
#define _data_load rom[0]
#define _rt_start ram_copy[1]
#define _rt_end (_rt_start+4)
#define _rt_load rom[1]
#define _c1_start ram_copy[2]
#define _c1_end (_c1_start+4)
#define _c1_load rom[2]
static struct { uint8_t p3_rst, wdt_con; uint32_t rst_src; } fm1_boot;
static uint8_t safe_mode;
static struct { uint32_t magic, crash_seen, boots; } x0x_dbg;
static unsigned guard_call, watchdog_armed, irq_enabled;
static void fm1_cpu1_halt(void) {}
static void fm1_time_init(void) {}
static void fm1_reset_reason(void) { fm1_boot.p3_rst=7; fm1_boot.rst_src=123; fm1_boot.wdt_con=11; }
static void fm1_wdt_arm(unsigned n) { assert(n == 0x0d); ++watchdog_armed; }
static void fm1_irq_init(void) { ++irq_enabled; }
static void irq_stack_mark(void) {}
static void fm1_mailbox_clear(void) {}
static void fm1_guard_enable(unsigned flags) { assert(flags == 15); ++guard_call; }
static unsigned irq_save(void) { return 1; }
static void irq_restore(unsigned saved_state) { assert(saved_state == 1); }
#define FL_FAR(fn) (fn)
static uint32_t fl_jedec_ram(void) { return 0; }
static void fl_plain_window_init(void) { abort(); }
static void st_owner_check(void) { abort(); }
static void smp_user_scan(uint32_t slot) { (void)slot; abort(); }
static void ed_smp_recover(void) { abort(); }
static void ota_boot_cleanup(void) { abort(); }
static const uint8_t *boot_shown(void) { return NULL; }
static void stop_before_audio_startup(void)
{
    capture(exit_frame);
    longjmp(reset_destination, 3);
}
#include "startup_bodies.inc"

static void startup(unsigned previous_failures, int core, const char *last)
{
    watchdog_armed = guard_call = irq_enabled = hw_init = feeds = fm1_ms = 0;
    fm1_in.buttons = usb.uboot_req = 0;
    scenario = previous_failures == 1 ? SAFE_PLAY : START_NORMAL;
    memset(ram_zero, 0xa5, sizeof ram_zero);
    memset(ram_copy, 0x5a, sizeof ram_copy);
    memset(&x0x_dbg, 0, sizeof x0x_dbg);
    bootguard.magic = BOOTGUARD_MAGIC; bootguard.failed = previous_failures;
    bootguard.pending = 1;
    snprintf(entry_frame, sizeof entry_frame, "%s", "boot-safe-mode");
    snprintf(exit_frame, sizeof exit_frame, "%s", last);
    fm1_crash = (fm1_crash_t){.magic=core<0 ? 0 : FM1_CRASH_MAGIC,
                              .core=core==1, .pc=0x12345678, .count=1};
    int endpoint = setjmp(reset_destination);
    if (!endpoint) { fm1_cstart(); abort(); }
    assert(watchdog_armed == 1);
    if (previous_failures == 3) {
        assert(endpoint == 2 && !hw_init && !irq_enabled && !guard_call && !bootguard.pending);
        return;
    }
    uint32_t zero[3][4] = {{0}};
    assert(!memcmp(ram_zero, zero, sizeof zero) && !memcmp(ram_copy, rom, sizeof rom));
    assert(irq_enabled==1 && guard_call==1 && hw_init==1 && fm1_boot.p3_rst==7 && !flash_ok);
    assert(fm1_boot.rst_src==123 && fm1_boot.wdt_con==11);
    if (previous_failures == 1) assert(endpoint == 1 && safe_mode && !bootguard.pending && !bootguard.failed);
    else assert(endpoint == 3 && !safe_mode && (core<0 || x0x_dbg.crash_seen==1));
}
