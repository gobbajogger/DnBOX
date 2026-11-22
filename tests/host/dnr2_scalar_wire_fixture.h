/* SPDX-License-Identifier: GPL-3.0-only */
/* Independent command and reply layouts from the DNR2 wire schema. */
static void put7(uint8_t *out, unsigned n, unsigned value)
{
    for (unsigned i = 0; i < n; ++i)
        out[i] = (uint8_t)((value >> (7 * i)) & 127);
}

static uint32_t read7(unsigned pos, unsigned n)
{
    uint32_t value = 0;
    for (unsigned i = 0; i < n; ++i)
        value |= (uint32_t)reply[pos + i] << (7 * i);
    return value;
}

static void command(uint8_t out[49], int op, unsigned client, unsigned epoch,
                    unsigned id, unsigned revision, unsigned control, unsigned value)
{
    memcpy(out, (uint8_t[49]){0x7d, 'D', 'N', 2}, 49);
    out[4] = (uint8_t)op;
    if (op) {
        memcpy(out + 5, remote.unit, 4);
        memcpy(out + 34, remote.unit_high, 15);
    }
    put7(out + 9, 4, client);
    put7(out + 13, 4, epoch);
    put7(out + 17, 4, id);
    put7(out + 21, 4, revision);
    put7(out + 25, 2, control);
    put7(out + 29, 5, value);
}

static void exchange(const uint8_t in[49], unsigned status)
{
    dispatch(in, 49);
    assert(reply_n == 51 && reply[0] == 0xf0 && reply[50] == 0xf7);
    assert(!memcmp(reply + 1, in, 4) && reply[5] == (in[4] | 64) && reply[6] == status);
    assert(!memcmp(reply + 8, remote.unit, 4) && !memcmp(reply + 35, remote.unit_high, 15));
    assert(!memcmp(reply + 12, in + 9, 4) && !memcmp(reply + 20, in + 17, 4));
}

