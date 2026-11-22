/* SPDX-License-Identifier: GPL-3.0-only */
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
static dnr2_scalar remote = {
    .unit = {13, 7, 2, 1}, .unit_high = {31, 22, 17},
    .control = 1, .catalog_crc = 0x4bdfe371u, .revision = 1
};
static uint8_t read_level(void) { return proj.sound.v[T_MIX][PART_909][MX_LEVEL]; }
static int write_level(uint8_t value)
{
    if (!probe_909_enqueue(value)) return 0;
    proj.sound.v[T_MIX][PART_909][MX_LEVEL] = value;
    mark_dirty();
    return 1;
}
#define ED_DNR2_STATE remote
#define ED_DNR2_READ read_level
#define ED_DNR2_WRITE write_level
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

/* The proof's one bound external writer uses the real normal UI writer. An
 * explicit change hook avoids a polling scheme missing an A-B-A edit. */
static void panel_write(int value)
{
    uint8_t before = read_level();
    pref_set(PR(R_ENG, T_MIX, PART_909, MX_LEVEL), value);
    if (read_level() != before) dnr2_scalar_changed(&remote);
}

int main(void)
{
    uint8_t in[49], apply[49];
    boot();
    for (unsigned i = 0; i < 16; ++i) audio_tick();
    assert(read_level() == 75 && !probe_pending());
    assert(probe_909_gain() == probe_gain_for(75));
    command(in, DNR2_HELLO, 42, 0, 1, 0, 0, 0);
    exchange(in, DNR2_OK);
    assert(read7(28, 2) == 1 && read7(30, 5) == 0x4bdfe371u);
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
    return 0;
}
