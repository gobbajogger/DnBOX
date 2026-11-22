/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../firmware/src/app/dnr2_scalar.h"
static dnr2_scalar dnr2_scratch;
static void scratch_source_changed(void)
{
    if (dnr2_scratch.revision)
        dnr2_scalar_changed(&dnr2_scratch);
}
#define DNR2_SOURCE_CHANGED() scratch_source_changed()
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
/* Actual editor parser + real project/UI/runtime. Only native wire/flash
 * boundaries are substituted; no project validators/adoption are mocked. */
static uint8_t editor_flash[0x100000],editor_frame[640],reply[800];
static uint32_t frame_n,reply_n,fm1_ms,fm1_crash[2],flash_writes,flash_erases;
static struct {uint32_t failed,pending;} bootguard;
static struct {unsigned ota_req,uboot_req;} usb;
static int flash_ok=1;
static void fm1_irq_off(void){} static void fm1_irq_on(void){} static void fm1_wdt_feed(void){}
static void audio_silence(void){} static void fl_inval(uint32_t off,uint32_t n){(void)off;(void)n;}
static int fl_read(uint32_t off,void *dst,uint32_t n){if(off+n>sizeof editor_flash)return -1;memcpy(dst,editor_flash+off,n);return 0;}
static int fl_write(uint32_t off,const void *p,uint32_t n){if(off+n>sizeof editor_flash)return -1;memcpy(editor_flash+off,p,n);flash_writes++;return 0;}
static int fl_erase4k(uint32_t off,uint32_t *took){if(off+4096>sizeof editor_flash)return -1;memset(editor_flash+off,255,4096);*took=1;flash_erases++;return 0;}
static uint8_t *fm1_xip_ptr(uint32_t off){return editor_flash+off;}
static int store_stage_held(void){return 0;}static uint8_t *store_stage_lock(uint32_t *cap){static uint8_t b[64];if(cap)*cap=sizeof b;return b;}
static void store_stage_unlock(void){}
static int ota_wire_send(const uint8_t *p,uint32_t n){memcpy(reply,p,n);reply_n=n;return 0;}
static int ota_frame_get(const uint8_t **p,uint32_t *n){*p=editor_frame;*n=frame_n;return frame_n!=0;}
static void ota_frame_done(void){frame_n=0;}
#include <assert.h>
#include "../../firmware/src/app/dnr2_scalar.h"

uint32_t probe_writes(void);
uint32_t probe_pending(void);
float probe_909_gain(void);
float probe_gain_for(int value);
int probe_909_enqueue(uint8_t value);
static uint8_t read_level(void) { return proj.sound.v[T_MIX][PART_909][MX_LEVEL]; }
#define remote dnr2_scratch
#ifdef DNR2_NATIVE_HOST_FIXTURE
static unsigned entropy_calls;
static int entropy_stuck;
static unsigned entropy_boot;
static uint32_t native_test_entropy(unsigned high)
{
    static const uint32_t words[] = {0x6d123456u, 0xf04305ffu, 0x11223344u, 0x55667788u};
    unsigned i = entropy_calls++ % 4u;
    assert(high == i % 2u);
    return entropy_stuck ? 0x11111111u : words[i] ^ entropy_boot;
}
#define DNR2_NATIVE_RANDOM(high) native_test_entropy(high)
#else
#define DNR2_SCRATCH_CLOCK() 1234567u
#endif
#include "../../firmware/src/app/dnr2_scratch_binding.inc"
#include "../../firmware/src/app/editor.c"

static void dispatch(const uint8_t *in, uint32_t size)
{
    memcpy(editor_frame, in, size);
    frame_n = size; reply_n = 0;
    assert(!ed_service());
    assert(!frame_n && !flash_writes && !flash_erases && !usb.ota_req && !usb.uboot_req);
}
#include "dnr2_scalar_wire_fixture.h"

static void audio_tick(void)
{
    int32_t pcm[512];
    engine_render(pcm, 256);
}

static void panel_write(int value)
{
    pref_set(PR(R_ENG, T_MIX, PART_909, MX_LEVEL), value);
}

static int nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int serve(void)
{
    char line[104];
    boot();
    while (fgets(line, sizeof line, stdin)) {
        uint8_t in[49];
        if (strlen(line) != 103 || memcmp(line, "f07d444e02", 10) ||
            memcmp(line + 100, "f7\n", 3))
            return 2;
        for (unsigned i = 0; i < sizeof in; ++i) {
            int hi = nibble(line[2 * i + 2]), lo = nibble(line[2 * i + 3]);
            if (hi < 0 || lo < 0) return 2;
            in[i] = (uint8_t)(hi * 16 + lo);
        }
        dispatch(in, sizeof in);
        audio_tick();
        for (unsigned i = 0; i < reply_n; ++i)
            printf("%02x", reply[i]);
        puts("");
        fflush(stdout);
    }
    return ferror(stdin) ? 2 : 0;
}

int main(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "--serve"))
        return serve();
    if (argc != 1)
        return 2;
    uint8_t in[49], apply[49];
    boot();
    for (unsigned i = 0; i < 16; ++i) audio_tick();
    assert(read_level() == 75 && !probe_pending());
    assert(probe_909_gain() == probe_gain_for(75));
    command(in, DNR2_HELLO, 42, 0, 1, 0, 0, 0);
#ifdef DNR2_NATIVE_HOST_FIXTURE
    entropy_stuck = 1;
    dispatch(in, 49);
    assert(reply_n == 0 && remote.revision == 0);
    entropy_stuck = 0;
#endif
    exchange(in, DNR2_OK);
#ifdef DNR2_NATIVE_HOST_FIXTURE
    assert(read7(28, 2) == DNR2_NATIVE_CONTROL && read7(30, 5) == DNR2_NATIVE_CRC);
    assert(read7(8, 4) == 0x0d123457u && read7(35, 5) == 0xf04305ffu);
    assert(read7(40, 5) == 0x11223344u && read7(45, 5) == 0x55667788u);
    assert(entropy_calls == 8);
    exchange(in, DNR2_OK);
    assert(entropy_calls == 8);
    puts("PASS source-owned catalog and boot incarnation; stuck RNG leaves HELLO silent");
#else
    assert(read7(28, 2) == 1 && read7(30, 5) == 0x4bdfe371u);
#endif
    command(in, DNR2_OPEN, 42, 0, 1, 0, 0, 0);
    exchange(in, DNR2_OK);
    unsigned epoch = read7(16, 4);
    command(in, DNR2_READ, 42, epoch, 2, 1, 1, 0); exchange(in, DNR2_OK);
    assert(read7(30, 5) == 75);
    uint32_t w = probe_writes();
    command(apply, DNR2_SET, 42, epoch, 3, 1, 1, 76); exchange(apply, DNR2_QUEUED);
    assert(read_level() == 76 && probe_writes() == w + 1 && probe_pending() == 1);
    assert(probe_909_gain() == probe_gain_for(75));
    exchange(apply, DNR2_QUEUED);
    assert(probe_writes() == w + 1);
    audio_tick();
    assert(!probe_pending() && probe_909_gain() == probe_gain_for(76));
    command(in, DNR2_SET, 42, epoch, 4, 2, 1, 75); exchange(in, DNR2_QUEUED);
    assert(read_level() == 75 && probe_writes() == w + 2 && probe_pending() == 1);
    audio_tick();
    assert(!probe_pending() && probe_909_gain() == probe_gain_for(75));
    puts("PASS Apply and Undo each queue one real 909 MIX Level message and change audio only on consumption");

    command(in, DNR2_SET, 42, epoch, 5, 3, 1, 76); exchange(in, DNR2_QUEUED);
    audio_tick();
    panel_write(77); panel_write(76); audio_tick();
    assert(read_level() == 76 && remote.revision == 6 && probe_909_gain() == probe_gain_for(76));
    w = probe_writes();
    command(in, DNR2_SET, 42, epoch, 6, 4, 1, 75); exchange(in, DNR2_CONFLICT);
    assert(read_level() == 76 && probe_writes() == w && !probe_pending());
    puts("PASS real panel A-B-A edits invalidate the stale Undo even when the value returns");

    while (probe_909_enqueue(76)) {}
    w = probe_writes();
    command(in, DNR2_SET, 42, epoch, 7, 6, 1, 75); exchange(in, DNR2_FULL);
    assert(read_level() == 76 && probe_writes() == w && remote.revision == 6);
    audio_tick();
    assert(probe_909_gain() == probe_gain_for(76));
    puts("PASS real saturated producer queue rejects Apply before project publication or revision change");

    assert(!ui_project_reset());
    assert(read_level() == 75 && remote.revision > 6);
    command(in, DNR2_SET, 42, epoch, 8, 6, 1, 77);
    exchange(in, DNR2_CONFLICT);
    assert(read_level() == 75);
    puts("PASS project adoption invalidates the pre-reset addressed revision");

#ifdef DNR2_NATIVE_HOST_FIXTURE
    uint8_t prior[49];
    command(prior, DNR2_SET, 42, epoch, 9, remote.revision, 1, 99);
    uint8_t previous_unit[4], previous_high[15];
    memcpy(previous_unit, remote.unit, sizeof previous_unit);
    memcpy(previous_high, remote.unit_high, sizeof previous_high);

    memset(&remote, 0, sizeof remote);
    entropy_boot = 0x8cab0120u;
    command(in, DNR2_HELLO, 42, 0, 1, 0, 0, 0);
    exchange(in, DNR2_OK);
    assert(memcmp(previous_unit, remote.unit, sizeof previous_unit) != 0);
    assert(memcmp(previous_high, remote.unit_high, sizeof previous_high) != 0);
    assert(read7(30, 5) == DNR2_NATIVE_CRC && read7(24, 4) == 1);
    dispatch(prior, 49);
    assert(reply_n == 0 && remote.revision == 1 && read_level() == 75);
    puts("PASS fresh simulated boot entropy refuses the previous boot's addressed SET");
#endif
    return 0;
}
