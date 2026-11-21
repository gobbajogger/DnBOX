/* SPDX-License-Identifier: GPL-3.0-only */
/* Focused exact differential for the engine's cleared stereo 808 render. */
#include "drum808.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { COUNT = 24000, CAP = D8_MAX_BLOCK + 8 };
static drum808_t ref, alt;
static float bus[2][4][CAP];
static uint32_t seed = 71, total[8], sizes[D8_MAX_BLOCK + 1], masks[2048];
static uint32_t random_word(void)
{
    seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
    return seed;
}
static void set_pan(int tr, int v)
{
    drum808_set(&ref, tr, drum808_nparams(tr) - 1, v);
}
static void all_center(int settle)
{
    for (int tr = 0; tr < D8_NUM; tr++) set_pan(tr, 64);
    if (settle) drum808_pan_settle(&ref);
}
static void prepare(int k)
{
    if (k % 33 == 0)
        for (int tr = 0; tr < D8_NUM; tr++) drum808_trigger(&ref, tr, .77f);
    if (k % 4 == 0)
        drum808_trigger(&ref, D8_OH, .65f);
    if (k % 4 == 1)
        drum808_trigger(&ref, D8_CH, .9f);
    if (k % 5 == 0) {
        int tr = random_word() % D8_NUM;
        drum808_trigger(&ref, tr, (random_word() % 128) / 127.0f);
    }
    if (k % 367 == 0) drum808_all_off(&ref);
    if (k % 2 == 0) {
        int tr = random_word() % D8_NUM;
        int param = random_word() % (drum808_nparams(tr) - 1);
        int max = drum808_param(tr, param)->max;
        drum808_set(&ref, tr, param, random_word() % (max + 1));
    }
    if (k % 16 == 0) {
        for (int tr = 0; tr < D8_NUM; ++tr) {
            int tail = drum808_nparams(tr) - 5;
            drum808_set(&ref, tr, tail, k % 32 ? random_word() % 128 : 0);
            drum808_set(&ref, tr, tail + 1, ((k / 16) + tr) % 7);
            drum808_set(&ref, tr, tail + 2, k % 48 ? random_word() % 128 : 0);
            drum808_set(&ref, tr, tail + 3, k % 32 ? random_word() % 128 : 0);
        }
    }
    if (k % 11 == 0) {
        drum808_set(&ref, D8_KIT, D8_KIT_DIST, (k / 11) % 8);
        drum808_set(&ref, D8_KIT, D8_KIT_DRIVE, k % 22 ? 127 : 1);
        drum808_set(&ref, D8_KIT, D8_KIT_GLUE, k % 33 ? 23 + random_word() % 105 : 0);
        drum808_set(&ref, D8_KIT, 0, (k / 11) % 128);
    }
    int phase = k % 96;
    if (phase == 0 || phase == 95)
        all_center(1);
    if (phase >= 57 && phase < 67) {
        int tr = random_word() % D8_NUM;
        static const int steps[] = { 0, 127, 63, 65, 1, 126 };
        set_pan(tr, steps[random_word() % 6]);
    }
    if (phase == 67) drum808_pan_settle(&ref);
    if (phase == 74 || phase == 87) all_center(0);
    if (phase == 85) set_pan(D8_CH, 127);
}
static int mode(void)
{
    int off = 0, moving = 0, stale = 0;
    for (int tr = 0; tr < D8_NUM; ++tr) {
        off |= ref.pan_l[tr] != 1.0f || ref.pan_r[tr] != 1.0f;
        moving |= ref.pan_lc[tr] != ref.pan_l[tr] || ref.pan_rc[tr] != ref.pan_r[tr];
        stale |= ref.pan_dl[tr] != 0.0f || ref.pan_dr[tr] != 0.0f;
    }
    return moving ? (off ? 3 : 4) : (off ? 2 : (stale ? 1 : 0));
}
static int check(int k, int mode, int n, uint16_t mask, const char *stage)
{
    for (int b = 0; b < 4; ++b)
        for (int i = 0; i < CAP; ++i)
            if (memcmp(&bus[0][b][i], &bus[1][b][i], sizeof(float))) {
                printf("%s mismatch block %d n %d mode %d mask %03x bus %d index %d %a vs %a\n",
                       stage, k, n, mode, mask, b, i, bus[0][b][i], bus[1][b][i]);
                return 0;
            }
    if (memcmp(&ref, &alt, sizeof ref)) {
        for (size_t i=0; i<sizeof ref; i++) if (((uint8_t*)&ref)[i] != ((uint8_t*)&alt)[i]) {
            printf("%s mismatch block %d n %d mode %d mask %03x state byte %zu %02x vs %02x\n",
                   stage, k, n, mode, mask, i, ((uint8_t*)&ref)[i], ((uint8_t*)&alt)[i]);
            break;
        }
        return 0;
    }
    return 1;
}
int main(void)
{
    drum808_init(&ref);
    for (int k = 0; k < COUNT; ++k) {
        prepare(k);
        int offset = random_word() % 5;
        static const int edges[] = { 0,1,2,15,16,17,31,32,33,63,64,65,87,88,89,127,128,129,195,228,240,255,256 };
        int n = k % 3 ? edges[k % (sizeof edges / sizeof edges[0])] : (int)(random_word() % 257);
        uint16_t mask = (uint16_t)(random_word() & 0x7ff);
        if (k < 4096) mask = k & 2047;
        else if (k % 5 == 0) mask = 0x7ff;
        else if (k % 5 == 1) mask = 0;
        else if (k % 5 == 2) mask = 1u << (random_word() % D8_NUM);
        if (!(k % 4)) mask |= 0xf800;
        for (int b = 0; b < 4; ++b)
            for (int i = 0; i < CAP; ++i) {
                float init = ((int)(random_word() % 2049) - 1024) * .001953125f;
                if (i >= offset && i < offset + n && (b < 2 || k % 4 == 0)) init = 0.0f;
                bus[0][b][i] = init;
            }
        memcpy(bus[1], bus[0], sizeof bus[0]);
        memcpy(&alt, &ref, sizeof alt);
        int m = mode();
        ++total[m]; ++sizes[n]; ++masks[mask & 0x7ff];
        for (int i = 0; i < 13; ++i)
            if (ref.lane[i].cstep < 0 && ref.lane[i].cg > 0) { ++total[5]; break; }
        if (ref.master_on) ++total[6];
        if (ref.kpot[D8_KIT_GLUE]) ++total[7];
        drum808_render_st_masked(&ref, bus[0][0]+offset, bus[0][1]+offset,
                                 bus[0][2]+offset, bus[0][3]+offset, n, mask);
        drum808_render_st_cleared(&alt, bus[1][0]+offset, bus[1][1]+offset,
                                  bus[1][2]+offset, bus[1][3]+offset, n, mask);
        if (!check(k,m,n,mask,"render")) return 1;
        drum808_kit_process(&ref, bus[0][0]+offset, bus[0][1]+offset, bus[0][2]+offset, bus[0][3]+offset, n);
        drum808_kit_process(&alt, bus[1][0]+offset, bus[1][1]+offset, bus[1][2]+offset, bus[1][3]+offset, n);
        if (!check(k,m,n,mask,"kit")) return 1;
    }
    unsigned nlen = 0, nmask = 0;
    for (int i=0;i<=256;++i) nlen += sizes[i] != 0;
    for (int i=0;i<2048;++i) nmask += masks[i] != 0;
    printf("%d blocks, %u lengths, %u masks. Four complete guarded buses and all %zu state bytes match"
           " before and after kit processing.\n",COUNT,nlen,nmask,sizeof ref);
    printf("center %u + %u stale slopes, off-center settled %u, moving away/sideways %u, moving to center %u, chokes %u, master drive %u, glue %u\n",
            total[0],total[1],total[2],total[3],total[4],total[5],total[6],total[7]);
    return nlen != 257 || nmask != 2048 || !total[1];
}
