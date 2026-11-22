/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef X0X_DNR2_SCALAR_H
#define X0X_DNR2_SCALAR_H
#include <stdint.h>
#include <string.h>

/* One stored, independent U32 parameter. No held input or float exposure. */
enum {
    DNR2_HELLO, DNR2_OPEN, DNR2_READ, DNR2_SET, DNR2_DOWN, DNR2_UP,
    DNR2_KEEPALIVE, DNR2_CLOSE
};
enum {
    DNR2_OK, DNR2_QUEUED, DNR2_BAD, DNR2_BUSY, DNR2_STALE, DNR2_CONFLICT,
    DNR2_UNSUPPORTED, DNR2_FULL, DNR2_EXHAUSTED
};
enum {
    DN_OP = 4, DN_UNIT = 5, DN_CLIENT = 9, DN_EPOCH = 13, DN_REQUEST = 17,
    DN_REVISION = 21, DN_CONTROL = 25, DN_TOKEN = 27, DN_VALUE = 29, DN_UNIT_HIGH = 34,
    DN_FRAME = 49, DN_REPLY = 51, DN_LIMIT = 0x0fffffff, DN_LEASE_MS = 2000
};
typedef struct {
    /* The caller supplies encoded, nonzero, boot-incarnation identity. The
     * identity correlates replies. The local MIDI link supplies no authentication. */
    uint8_t unit[4], unit_high[15];
    uint16_t control;
    uint32_t catalog_crc, revision, epoch, client, deadline, last_request;
    uint8_t cached_input[DN_FRAME], cached_reply[DN_REPLY];
    uint8_t cache_valid;
} dnr2_scalar;

static uint32_t dnr2_get28(const uint8_t *p)
{
    return p[0] | (uint32_t)p[1] << 7 | (uint32_t)p[2] << 14 | (uint32_t)p[3] << 21;
}

static void dnr2_put(uint8_t *p, unsigned n, uint32_t value)
{
    while (n--) {
        *p++ = (uint8_t)(value & 127u);
        value >>= 7;
    }
}

static int dnr2_zero(const uint8_t *p, unsigned n)
{
    while (n--)
        if (*p++)
            return 0;
    return 1;
}

static void dnr2_scalar_changed(dnr2_scalar *s)
{
    if (s->revision < DN_LIMIT)
        ++s->revision;
}

/* try_store returns 1 only after the authoritative stored value is updated and
 * the corresponding audio command is accepted. On failure both remain intact.
 * External changes use dnr2_scalar_changed(); this callback does not. */
static int dnr2_scalar_frame(dnr2_scalar *s, const uint8_t *p, uint32_t n,
                            uint32_t now, uint8_t current, int (*try_store)(uint8_t),
                            uint8_t out[DN_REPLY])
{
    if (n != DN_FRAME || p[DN_OP] > DNR2_CLOSE || !s->revision || !s->control ||
        !s->catalog_crc || !dnr2_get28(s->unit))
        return 0;
    for (unsigned i = DN_UNIT; i < DN_FRAME; ++i)
        if (p[i] > 127u)
            return 0;
    if (p[DN_UNIT_HIGH - 1] > 15u || p[38] > 15u || p[43] > 15u || p[48] > 15u)
        return 0;

    unsigned op = p[DN_OP], status = DNR2_BAD, kind = 0;
    uint32_t client = dnr2_get28(p + DN_CLIENT), request = dnr2_get28(p + DN_REQUEST);
    uint32_t epoch = dnr2_get28(p + DN_EPOCH), value = 0;
    unsigned control = p[DN_CONTROL] | (unsigned)p[DN_CONTROL + 1] << 7;
    int cache = 0, empty = dnr2_zero(p + DN_CONTROL, DN_UNIT_HIGH - DN_CONTROL);
    if (s->client && (int32_t)(now - s->deadline) >= 0)
        s->client = 0;

    if (!client || !request)
        return 0;
    if (op == DNR2_HELLO) {
        if (dnr2_zero(p + DN_UNIT, 4) && dnr2_zero(p + DN_UNIT_HIGH, 15) &&
            !epoch && dnr2_zero(p + DN_REVISION, 4) && empty) {
            status = DNR2_OK;
            control = 1;
            value = s->catalog_crc;
        }
    } else if (memcmp(p + DN_UNIT, s->unit, 4) ||
               memcmp(p + DN_UNIT_HIGH, s->unit_high, 15)) {
        return 0;
    } else if (s->cache_valid && !memcmp(p, s->cached_input, DN_FRAME)) {
        if (s->client || op == DNR2_CLOSE) {
            memcpy(out, s->cached_reply, DN_REPLY);
            return 1;
        }
        status = DNR2_STALE;
    } else if (op == DNR2_OPEN) {
        if (!epoch && dnr2_zero(p + DN_REVISION, 4) && empty) {
            if (s->client)
                status = DNR2_BUSY;
            else if (s->epoch == DN_LIMIT)
                status = DNR2_EXHAUSTED;
            else {
                epoch = ++s->epoch;
                s->client = client;
                s->last_request = request;
                s->deadline = now + DN_LEASE_MS;
                status = DNR2_OK;
                cache = 1;
            }
        }
    } else if (op == DNR2_READ) {
        if (dnr2_zero(p + DN_TOKEN, DN_UNIT_HIGH - DN_TOKEN)) {
            status = DNR2_UNSUPPORTED;
            if (control == s->control) {
                status = DNR2_OK;
                kind = 1;
                value = current;
            }
        }
    } else if (client != s->client || epoch != s->epoch || request <= s->last_request) {
        status = DNR2_STALE;
    } else {
        s->last_request = request;
        cache = 1;
        if ((op == DNR2_KEEPALIVE || op == DNR2_CLOSE) && empty) {
            if (op == DNR2_CLOSE)
                s->client = 0;
            else
                s->deadline = now + DN_LEASE_MS;
            status = DNR2_OK;
        } else if (op == DNR2_DOWN || op == DNR2_UP) {
            status = DNR2_UNSUPPORTED;
        } else if (op == DNR2_SET) {
            if (dnr2_get28(p + DN_REVISION) != s->revision)
                status = DNR2_CONFLICT;
            else if (s->revision == DN_LIMIT)
                status = DNR2_EXHAUSTED;
            else if (control != s->control)
                status = DNR2_UNSUPPORTED;
            else if (dnr2_zero(p + DN_TOKEN, 2) && dnr2_zero(p + DN_VALUE + 1, 4)) {
                if (try_store(p[DN_VALUE]) == 1) {
                    dnr2_scalar_changed(s);
                    s->deadline = now + DN_LEASE_MS;
                    status = DNR2_QUEUED;
                    kind = 1;
                    value = p[DN_VALUE];
                } else {
                    status = DNR2_FULL;
                }
            }
        }
    }

    out[0] = 0xf0;
    memcpy(out + 1, p, 4);
    out[5] = (uint8_t)(op | 64u);
    out[6] = (uint8_t)status;
    out[7] = (uint8_t)kind;
    memcpy(out + 8, s->unit, 4);
    memcpy(out + 12, p + DN_CLIENT, 4);
    dnr2_put(out + 16, 4, epoch);
    memcpy(out + 20, p + DN_REQUEST, 4);
    dnr2_put(out + 24, 4, s->revision);
    dnr2_put(out + 28, 2, control);
    dnr2_put(out + 30, 5, value);
    memcpy(out + 35, s->unit_high, 15);
    out[50] = 0xf7;
    if (cache) {
        memcpy(s->cached_input, p, DN_FRAME);
        memcpy(s->cached_reply, out, DN_REPLY);
        s->cache_valid = 1;
    }
    return 1;
}
#endif
