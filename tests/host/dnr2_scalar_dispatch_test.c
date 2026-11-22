/* SPDX-License-Identifier: GPL-3.0-only */
/* No physical transport. Step the production editor and OTA dispatchers across
 * the existing host frame boundary. The scalar engine callback is injected. */
#include <stdlib.h>
#ifdef DNR2_TEST_CANDIDATE
#include "../../firmware/src/app/dnr2_scalar.h"
static dnr2_scalar remote = {
    .unit = {13, 7, 2, 1}, .unit_high = {31, 22, 17},
    .control = 1, .catalog_crc = 0x4bdfe371u, .revision = 1
};
static uint8_t scalar = 75;
static int writes, full, maintenance, refuse;
static uint8_t read_scalar(void) { return scalar; }
static int write_scalar(uint8_t value)
{
    if (full) return -1;
    scalar = value;
    writes++;
    return 1;
}
#define ED_DNR2_STATE remote
#define ED_DNR2_READ read_scalar
#define ED_DNR2_WRITE write_scalar
#define ED_MAINTENANCE_HELD() maintenance
#define ED_MAINTENANCE_ADMIT() (!refuse)
#endif
#define main project_protocol_main
#include "project_protocol_test.c"
#undef main
#define FELUCCA_ID "FM-1_900"
#include "../../firmware/src/ota.c"

static uint32_t ota_now_ms(void) { abort(); }
static void ota_idle(void) { abort(); }
static int ota_erase(uint32_t off) { (void)off; abort(); }
static int ota_prog(uint32_t off, const void *p, uint32_t n) { (void)off; (void)p; (void)n; abort(); }
static int ota_fread(uint32_t off, void *p, uint32_t n) { (void)off; (void)p; (void)n; abort(); }
static void ota_show(uint32_t step, int32_t code) { (void)step; (void)code; abort(); }
static void ota_commit(const uint8_t *parm) { (void)parm; abort(); }

static void dispatch(const uint8_t *input, uint32_t n)
{
    memcpy(frame, input, n);
    frame_n = n;
    reply_n = 0;
    if (ed_service()) ota_service();
    assert(!frame_n && !flash_writes && !flash_erases && !usb.ota_req && !usb.uboot_req);
}

#ifdef DNR2_TEST_CANDIDATE
#include "dnr2_scalar_wire_fixture.h"

static void external_edit(uint8_t value)
{
    scalar = value;
    dnr2_scalar_changed(&remote);
}

static void exercise_scalar(void)
{
    uint8_t in[49], saved[51], apply[49];
    command(in, DNR2_HELLO, 42, 0, 1, 0, 0, 0);
    exchange(in, DNR2_OK);
    assert(read7(28, 2) == 1 && read7(30, 5) == 0x4bdfe371u);
    command(in, DNR2_OPEN, 42, 0, 2, 0, 0, 0);
    exchange(in, DNR2_OK);
    unsigned epoch = read7(16, 4);
    assert(epoch != 0);
    memcpy(saved, reply, sizeof saved);
    exchange(in, DNR2_OK);
    assert(!memcmp(saved, reply, sizeof saved));

    command(in, DNR2_OPEN, 43, 0, 1, 0, 0, 0);
    exchange(in, DNR2_BUSY);
    command(in, DNR2_READ, 42, epoch, 3, 1, 1, 0);
    exchange(in, DNR2_OK);
    assert(reply[7] == 1 && read7(30, 5) == 75 && read7(24, 4) == 1);

    command(apply, DNR2_SET, 42, epoch, 4, 1, 1, 76);
    exchange(apply, DNR2_QUEUED);
    assert(writes == 1 && scalar == 76 && read7(24, 4) == 2 && read7(30, 5) == 76);
    memcpy(saved, reply, sizeof saved);
    exchange(apply, DNR2_QUEUED);
    assert(writes == 1 && !memcmp(saved, reply, sizeof saved));
    command(in, DNR2_READ, 42, epoch, 5, 2, 1, 0);
    exchange(in, DNR2_OK);
    assert(read7(30, 5) == 76 && read7(24, 4) == 2);

    command(in, DNR2_SET, 43, epoch, 6, 2, 1, 90);
    exchange(in, DNR2_STALE);
    command(in, DNR2_SET, 42, epoch, 6, 2, 1, 75);
    exchange(in, DNR2_QUEUED);
    assert(writes == 2 && read7(24, 4) == 3);
    command(in, DNR2_READ, 42, epoch, 7, 3, 1, 0);
    exchange(in, DNR2_OK);
    assert(read7(30, 5) == 75 && read7(24, 4) == 3);
    puts("addressed scalar READ/Apply/READ/Undo/READ: 75 -> 76 -> 75, revisions 1/2/3");

    external_edit(71);
    external_edit(75);
    command(in, DNR2_SET, 42, epoch, 8, 3, 1, 76);
    exchange(in, DNR2_CONFLICT);
    assert(read7(24, 4) == 5 && writes == 2);
    command(in, DNR2_SET, 42, epoch, 9, 5, 1, 128);
    exchange(in, DNR2_BAD);
    command(in, DNR2_SET, 42, epoch, 10, 5, 2, 76);
    exchange(in, DNR2_UNSUPPORTED);
    command(in, DNR2_SET, 42, epoch, 11, 5, 1, 76);
    full = 1;
    exchange(in, DNR2_FULL);
    full = 0;
    assert(writes == 2 && read7(24, 4) == 5 && scalar == 75);
    /* Conflicting payload with a spent request ID may not replace it. */
    in[29] = 77;
    exchange(in, DNR2_STALE);
    exchange(apply, DNR2_STALE);

    command(in, DNR2_DOWN, 42, epoch, 12, 5, 1, 1);
    exchange(in, DNR2_UNSUPPORTED);
    command(in, DNR2_SET, 42, epoch, 13, 5, 1, 76);
    held = 1;
    dispatch(in, 49);
    assert(!reply_n && writes == 2);
    held = 0;
    maintenance = 1;
    dispatch(in, 49);
    assert(!reply_n && writes == 2);
    maintenance = 0;
    refuse = 1;
    dispatch(in, 49);
    assert(!reply_n && writes == 2);
    refuse = 0;
    in[5] ^= 1;
    dispatch(in, 49);
    assert(!reply_n);
    in[5] ^= 1;
    in[33] = 16;
    dispatch(in, 49);
    assert(!reply_n);
    in[33] = 0;
    dispatch(in, 48);
    assert(!reply_n);
    in[29] = 0x80;
    dispatch(in, 49);
    assert(!reply_n);

    command(in, DNR2_KEEPALIVE, 42, epoch, 13, 5, 0, 0);
    fm1_ms = 1999;
    exchange(in, DNR2_OK);
    fm1_ms = 3999;
    exchange(in, DNR2_STALE);
    command(in, DNR2_SET, 42, epoch, 14, 5, 1, 76);
    exchange(in, DNR2_STALE);
    command(in, DNR2_OPEN, 43, 0, 2, 0, 0, 0);
    exchange(in, DNR2_OK);
    unsigned new_epoch = read7(16, 4);
    assert(new_epoch != epoch && new_epoch);
    command(in, DNR2_CLOSE, 43, new_epoch, 3, 5, 0, 0);
    exchange(in, DNR2_OK);
    exchange(in, DNR2_OK);
    command(in, DNR2_SET, 43, new_epoch, 4, 5, 1, 76);
    exchange(in, DNR2_STALE);
    assert(writes == 2 && scalar == 75);
    request(1, NULL, 0);
    assert(reply_n == 25);
    puts("replay, foreign address, malformed input, ABA, full queue, lease expiry, close, project and maintenance guards: PASS");
}
/* A case-sensitive, line-oriented software bridge for the browser client test. */
static int nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int serve(void)
{
    char line[104];
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
        for (unsigned i = 0; i < reply_n; ++i)
            printf("%02x", reply[i]);
        puts("");
        fflush(stdout);
    }
    return ferror(stdin) ? 2 : 0;
}
#endif

int main(int argc, char **argv)
{
#ifdef DNR2_TEST_CANDIDATE
    if (argc == 2 && !strcmp(argv[1], "--serve"))
        return serve();
#else
    (void)argv;
#endif
    if (argc != 1) return 2;
    request(1, NULL, 0);
    assert(reply_n == 25);
    puts("stock INFO: 25 reply bytes");

    static const uint8_t hello[49] = {0x7D, 'D', 'N', 2, 0, 0, 0, 0, 0, 42, 0, 0, 0, 0, 0, 0, 0, 1};
    dispatch(hello, sizeof hello);
    if (reply_n != 51) {
        fprintf(stderr, "DNR2 stock dispatcher returned %u bytes; expected 51\n", reply_n);
        return 1;
    }
    puts("DNR2 HELLO: 51 reply bytes");
#ifdef DNR2_TEST_CANDIDATE
    exercise_scalar();
#endif
    return 0;
}
