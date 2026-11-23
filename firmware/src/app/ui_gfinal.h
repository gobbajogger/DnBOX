/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef UI_GFINAL_H
#define UI_GFINAL_H
/* G-FINAL shared glyph rasterizer: C port of tools/gfinal/gfinal.py (pixel parity:
 * tests/host/gfinal_parity_test.py).
 * 26x19 cell, 4x4 supersampled coverage, integer Q4 geometry. */
typedef float gf;
#define GF_W 26
#define GF_H 19
enum { GF_TRACK, GF_GRAY, GF_INK, GF_WHITE };
typedef struct {
    gf amp, env, filter_env, phase, progress, mod, tape_phase, clock_phase, cutoff, resonance;
    uint8_t active, tape_active, playing, has_cutoff, has_resonance;
} gf_live;
/* One role per pass (track, gray, ink, white): 4x4 sample bits per pixel. */
static uint16_t *gf_m; static int gf_pass;  /* caller-owned GF_W*GF_H mask (UI stack) */
/* sin: quarter wave of round(sin(2*pi*i/1024)*16384), i=0..256 (exact by symmetry) */
static const uint16_t GF_SINQ[257]={0,101,201,302,402,503,603,704,804,904,1005,1105,1205,1306,1406,1506,1606,1706,1806,1906,2006,2105,2205,2305,2404,2503,2603,2702,2801,2900,2999,3098,3196,3295,3393,3492,3590,3688,3786,3883,3981,4078,4176,4273,4370,4467,4563,4660,4756,4852,4948,5044,5139,5235,5330,5425,5520,5614,5708,5803,5897,5990,6084,6177,6270,6363,6455,6547,6639,6731,6823,6914,7005,7096,7186,7276,7366,7456,7545,7635,7723,7812,7900,7988,8076,8163,8250,8337,8423,8509,8595,8680,8765,8850,8935,9019,9102,9186,9269,9352,9434,9516,9598,9679,9760,9841,9921,10001,10080,10159,10238,10316,10394,10471,10549,10625,10702,10778,10853,10928,11003,11077,11151,11224,11297,11370,11442,11514,11585,11656,11727,11797,11866,11935,12004,12072,12140,12207,12274,12340,12406,12472,12537,12601,12665,12729,12792,12854,12916,12978,13039,13100,13160,13219,13279,13337,13395,13453,13510,13567,13623,13678,13733,13788,13842,13896,13949,14001,14053,14104,14155,14206,14256,14305,14354,14402,14449,14497,14543,14589,14635,14680,14724,14768,14811,14854,14896,14937,14978,15019,15059,15098,15137,15175,15213,15250,15286,15322,15357,15392,15426,15460,15493,15525,15557,15588,15619,15649,15679,15707,15736,15763,15791,15817,15843,15868,15893,15917,15941,15964,15986,16008,16029,16049,16069,16088,16107,16125,16143,16160,16176,16192,16207,16221,16235,16248,16261,16273,16284,16295,16305,16315,16324,16332,16340,16347,16353,16359,16364,16369,16373,16376,16379,16381,16383,16384,16384};
static int gf_sin_t(int i) { int h = i >= 512; i -= h*512; int v = GF_SINQ[i > 256 ? 512 - i : i]; return h ? -v : v; }
/* exp: round(exp(-i/64)*2^20) == round(A[i%64]*B[i/64]*2^20) in binary32, except 5 entries */
static const float GF_EXPA[64]={1.0f,0.9844964146614075f,0.9692332148551941f,0.954206645488739f,0.9394130706787109f,0.9248487949371338f,0.9105103611946106f,0.8963941931724548f,0.8824968934059143f,0.8688150644302368f,0.8553453087806702f,0.8420844078063965f,0.8290291428565979f,0.8161762356758118f,0.8035225868225098f,0.7910650968551636f,0.7788007855415344f,0.7667266130447388f,0.7548395991325378f,0.7431368827819824f,0.7316156029701233f,0.720272958278656f,0.7091062068939209f,0.6981124877929688f,0.687289297580719f,0.6766338348388672f,0.6661435961723328f,0.6558160185813904f,0.6456485390663147f,0.6356386542320251f,0.6257839798927307f,0.6160821318626404f,0.6065306663513184f,0.5971272587776184f,0.5878696441650391f,0.5787556171417236f,0.5697828531265259f,0.5609491467475891f,0.552252471446991f,0.54369056224823f,0.5352614521980286f,0.5269629955291748f,0.5187931656837463f,0.5107499957084656f,0.5028315782546997f,0.49503588676452637f,0.4873610734939575f,0.47980523109436035f,0.4723665416240692f,0.46504318714141846f,0.45783334970474243f,0.45073530077934265f,0.44374731183052063f,0.4368676543235779f,0.4300946295261383f,0.42342662811279297f,0.41686201095581055f,0.4103991687297821f,0.4040365219116211f,0.39777252078056335f,0.3916056156158447f,0.3855343461036682f,0.37955719232559204f,0.3736726939678192f};
static const float GF_EXPB[17]={1.0f,0.3678794503211975f,0.1353352814912796f,0.049787066876888275f,0.018315639346837997f,0.0067379469983279705f,0.0024787522852420807f,0.0009118819725699723f,0.000335462624207139f,0.00012340980174485594f,4.539993096841499e-05f,1.6701700587873347e-05f,6.1442124206223525e-06f,2.260329438286135e-06f,8.315287232107949e-07f,3.0590231858695915e-07f,1.1253517584464134e-07f};
static const uint16_t GF_EXPFIX_I[5]={14,32,78,117,164};
static const uint32_t GF_EXPFIX_V[5]={842554,635993,309958,168521,80857};
static int32_t gf_exp_t(int i)
{
    for (int k = 0; k < 5; ++k) if (GF_EXPFIX_I[k] == i) return (int32_t)GF_EXPFIX_V[k];
    return (int32_t)fm_floorf(GF_EXPA[i & 63] * GF_EXPB[i >> 6] * 1048576.f + .5f);
}
static int gf_rnd(gf x) { return (int)fm_floorf(x + (gf)0.5); }
static gf gf_cl(gf x, gf a, gf b) { return x < a ? a : x > b ? b : x; }
static gf gf_sinc(gf t)
{
    t = t - fm_floorf(t); gf x = t * (gf)1024.0; int i = (int)fm_floorf(x); gf f = x - (gf)i;
    return ((gf)gf_sin_t(i) + (gf)(gf_sin_t(i+1) - gf_sin_t(i)) * f) / (gf)16384.0;
}
static gf gf_exp(gf x)
{
    if (x > 0) return (gf)1.0 / gf_exp(-x);
    gf y = -x * (gf)64.0;
    if (y >= (gf)1024.0) return (gf)gf_exp_t(1024) / (gf)1048576.0;
    int i = (int)fm_floorf(y); gf f = y - (gf)i;
    return ((gf)gf_exp_t(i) + (gf)(gf_exp_t(i+1) - gf_exp_t(i)) * f) / (gf)1048576.0;
}
static gf gf_tanh(gf x)
{
    gf e = gf_exp((gf)-2.0 * (x < 0 ? -x : x)), v = ((gf)1.0 - e) / ((gf)1.0 + e);
    return x >= 0 ? v : -v;
}
static int gf_q(gf v) { return gf_rnd(v * (gf)16); }
/* sample centre of subsample s (0..3) of pixel p, in Q4 */
#define GF_S(p, s) ((p) * 16 - 6 + (s) * 4)
/* pixel range whose samples may lie in [v, ...] / [..., v] */
static int gf_lo(int v) { int p = (v + 6 - 12) >> 4; return p < 0 ? 0 : p; }
static int gf_hi(int v) { int p = (v + 6) >> 4; return p > GF_W - 1 ? GF_W - 1 : p; }
static void gf_seg(int role, int ax, int ay, int bx, int by, int hw)
{
    if (role != gf_pass) return;
    int x0 = (ax < bx ? ax : bx) - hw, x1 = (ax > bx ? ax : bx) + hw;
    int y0 = (ay < by ? ay : by) - hw, y1 = (ay > by ? ay : by) + hw;
    int64_t dx = bx - ax, dy = by - ay, l2 = dx*dx + dy*dy, h2 = (int64_t)hw*hw;
    int pya = gf_lo(y0), pyb = gf_hi(y1) > GF_H - 1 ? GF_H - 1 : gf_hi(y1), pxa = gf_lo(x0), pxb = gf_hi(x1);
    for (int py = pya; py <= pyb; ++py) for (int sy = 0; sy < 4; ++sy) {
        int Y = GF_S(py, sy); if (Y < y0 || Y > y1) continue;
        for (int px = pxa; px <= pxb; ++px) for (int sx = 0; sx < 4; ++sx) {
            int X = GF_S(px, sx); if (X < x0 || X > x1) continue;
            int64_t qx = X - ax, qy = Y - ay; int in;
            if (!l2) in = qx*qx + qy*qy <= h2;
            else {
                int64_t t = qx*dx + qy*dy, cr = qx*dy - qy*dx;
                if (t <= 0) in = qx*qx + qy*qy <= h2;
                else if (t >= l2) in = (int64_t)(X-bx)*(X-bx) + (int64_t)(Y-by)*(Y-by) <= h2;
                else in = cr*cr <= h2*l2;
            }
            if (in) gf_m[py*GF_W+px] |= (uint16_t)(1u << (sy*4+sx));
        }
    }
}
/* Streamed polyline: gf_begin(role), GP(x,y)..., gf_poly(role); a lone point is a dot. */
static int gf_n, gf_lx, gf_ly, gf_role;
static void gf_begin(int role) { gf_n = 0; gf_role = role; }
static void gf_to(gf x, gf y)
{
    int qx = gf_q(x), qy = gf_q(y);
    if (gf_n) gf_seg(gf_role, gf_lx, gf_ly, qx, qy, 8);
    gf_lx = qx; gf_ly = qy; ++gf_n;
}
#define GP(x, y) gf_to((gf)(x), (gf)(y))
static void gf_poly(int role) { (void)role; if (gf_n == 1) gf_seg(gf_role, gf_lx, gf_ly, gf_lx, gf_ly, 8); gf_n = 0; }
/* constant polyline in half-pixel units */
static void gf_pl(const int8_t *p, int n, int role)
{
    if (!gf_n) gf_begin(role);
    for (int i = 0; i < n; i += 2) GP((gf)p[i] * .5f, (gf)p[i+1] * .5f);
    gf_poly(role);
}
static void gf_line(gf ax, gf ay, gf bx, gf by, int role) { gf_begin(role); GP(ax, ay); GP(bx, by); gf_poly(role); }
static void gf_rect(gf x, gf y, gf w, gf h, int role)
{
    if (role != gf_pass) return;
    int x0 = gf_rnd(x), y0 = gf_rnd(y), x1 = gf_rnd(x + w), y1 = gf_rnd(y + h);
    if (x1 <= x0 || y1 <= y0) return;
    if (x0 < 0) x0 = 0; if (y0 < 0) y0 = 0; if (x1 > GF_W) x1 = GF_W; if (y1 > GF_H) y1 = GF_H;
    for (int yy = y0; yy < y1; ++yy) for (int xx = x0; xx < x1; ++xx) gf_m[yy*GF_W+xx] = 0xffff;
}
static void gf_erase(int x, int y, int w, int h)
{
    for (int yy = y; yy < y + h; ++yy) for (int xx = x; xx < x + w; ++xx)
        if (xx >= 0 && yy >= 0 && xx < GF_W && yy < GF_H) gf_m[yy*GF_W+xx] = 0;
}
static void gf_circle(gf x, gf y, gf r, int role, int fill)
{
    if (role != gf_pass) return;
    int cx = gf_q(x), cy = gf_q(y), rq = gf_q(r);
    int64_t o = (int64_t)(rq + 8) * (rq + 8), in0 = rq - 8 > 0 ? (int64_t)(rq - 8) * (rq - 8) : 0;
    int e = rq + 8;
    for (int py = gf_lo(cy - e); py <= gf_hi(cy + e) && py < GF_H; ++py) for (int sy = 0; sy < 4; ++sy)
        for (int px = gf_lo(cx - e); px <= gf_hi(cx + e); ++px) for (int sx = 0; sx < 4; ++sx) {
            int64_t X = GF_S(px, sx) - cx, Y = GF_S(py, sy) - cy, d2 = X*X + Y*Y;
            if (d2 <= o && (fill || d2 >= in0)) gf_m[py*GF_W+px] |= (uint16_t)(1u << (sy*4+sx));
        }
}
typedef gf (*gf_fn)(gf t);
static void gf_wave(gf_fn fn, gf amp, gf mid, int role, gf x0, gf x1, int n)
{
    gf_begin(role); for (int i = 0; i <= n; ++i) {
        gf x = x0 + (gf)i * (x1 - x0) / (gf)n;
        GP(x, mid - amp * gf_cl(fn((x - x0) / (x1 - x0)), -1, 1));
    }
    gf_poly(role);
}
static void gf_arrow(gf x0, gf y0, gf x1, gf y1, int role)
{
    gf_line(x0, y0, x1, y1, role);
    gf dx = x1 - x0, dy = y1 - y0, l = fm_sqrtf(dx*dx + dy*dy), ux = dx / l, uy = dy / l;
    const gf C = (gf)0.8253356149096783, S = (gf)0.5646424733950354;
    gf_begin(role); GP(x1 - 3*(ux*C - uy*S), y1 - 3*(uy*C + ux*S)); GP(x1, y1); GP(x1 - 3*(ux*C + uy*S), y1 - 3*(uy*C - ux*S));
    gf_poly(role);
}

enum {GF_LEVEL,GF_PAN,GF_STEREO,GF_WAVE,GF_WAVE_SHAPE,GF_SUB,GF_NOISE,GF_TUNE,GF_GLIDE,GF_PWM,GF_FM,GF_DRIVE,GF_FOLD,GF_BITS,GF_SAMPLE_RATE,GF_DISTORTION,GF_CUTOFF,GF_HIGHPASS,GF_RESONANCE,GF_FILTER_MODE,GF_BANDWIDTH,GF_ENV_AMOUNT,GF_ATTACK,GF_DECAY,GF_SUSTAIN,GF_RELEASE,GF_GATE_MODE,GF_TRANSIENT,GF_PITCH_DECAY,GF_PITCH_DEPTH,GF_PITCH_BASE,GF_SNAPPY,GF_LFO_RATE,GF_LFO_DEPTH,GF_LFO_DELAY,GF_CHORUS,GF_PHASER,GF_RINGMOD,GF_DELAY_TIME,GF_FEEDBACK,GF_DELAY_SEND,GF_REVERB_SEND,GF_TONE,GF_TAPE,GF_WEAR,GF_DELAY_TYPE,GF_PINGPONG,GF_THRESHOLD,GF_RATIO,GF_MAKEUP,GF_BLEND,GF_COMPRESSOR,GF_LIMITER,GF_PUMP,GF_SOURCE,GF_TEMPO,GF_SWING,GF_LENGTH,GF_REPEAT,GF_RATE,GF_DIRECTION,GF_SLOT,GF_NOTE,GF_GATE,GF_CHANCE,GF_SLIDE,GF_LINK,GF_OCTAVE,GF_KEY_RANGE,GF_TRACKING,GF_PEDAL,GF_ENABLE,GF_DESTINATION,GF_SCALE,GF_ROOT,GF_UNISON,GF_MOD_BASE,GF_MOD_MODE,GF_LEGATO,GF_LOOP,GF_DENSITY,GF_MUTATE,GF_COUNT};
#ifdef X0X_HOST
static const char *const gf_names[]={"LEVEL","PAN","STEREO","WAVE","WAVE_SHAPE","SUB","NOISE","TUNE","GLIDE","PWM","FM","DRIVE","FOLD","BITS","SAMPLE_RATE","DISTORTION","CUTOFF","HIGHPASS","RESONANCE","FILTER_MODE","BANDWIDTH","ENV_AMOUNT","ATTACK","DECAY","SUSTAIN","RELEASE","GATE_MODE","TRANSIENT","PITCH_DECAY","PITCH_DEPTH","PITCH_BASE","SNAPPY","LFO_RATE","LFO_DEPTH","LFO_DELAY","CHORUS","PHASER","RINGMOD","DELAY_TIME","FEEDBACK","DELAY_SEND","REVERB_SEND","TONE","TAPE","WEAR","DELAY_TYPE","PINGPONG","THRESHOLD","RATIO","MAKEUP","BLEND","COMPRESSOR","LIMITER","PUMP","SOURCE","TEMPO","SWING","LENGTH","REPEAT","RATE","DIRECTION","SLOT","NOTE","GATE","CHANCE","SLIDE","LINK","OCTAVE","KEY_RANGE","TRACKING","PEDAL","ENABLE","DESTINATION","SCALE","ROOT","UNISON","MOD_BASE","MOD_MODE","LEGATO","LOOP","DENSITY","MUTATE"};
#endif
static const unsigned char gf_states[]={0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,8,0,0,0,4,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,2,0,0,0,0,0,2,0,3,0,0,0,0,0,4,0,0,0,0,0,2,4,0,0,2,2,3,0,0,0,0,3,2,2,0,0};
static gf G_q, G_amp, G_p, G_ph, G_g, G_cyc; static int G_active, G_id, G_j;
static void gf_base(void) { gf_line(1, 17, 24, 17, GF_TRACK); }
static void gf_dot(gf x, gf y) { gf_rect(gf_cl(x, 2, 23) - 1, gf_cl(y, 2, 16) - 1, 3, 3, GF_WHITE); }
static void gf_pips(void) { if (G_active) for (int i = 0; i < gf_rnd(G_amp * 3); ++i) gf_rect(2 + i*4, 16, 2, 1, GF_WHITE); }
static gf gf_sine(gf t, gf f) { return gf_sinc(t * f); }
static void gf_filter(int mode, gf cq, gf rq, int ink) /* mode 0 LP, 1 HP, 2 BP */
{
    gf x = 5 + 14*cq, top = 7, peak = top - 5*rq;
    gf_begin(ink); if (mode == 0) { GP(1, top); GP(x-2, top); GP(x, peak); GP(x+4, 16); GP(24, 16); }
    else if (mode == 1) { GP(1, 16); GP(x-4, 16); GP(x, peak); GP(x+2, top); GP(24, top); }
    else { GP(1, 16); GP(x-4, 16); GP(x, peak); GP(x+4, 16); GP(24, 16); }
    gf_poly(ink);
}
static void gf_envshape(int which)
{
    gf a = (gf).2, d = (gf).3, su = (gf).6, r = (gf).3, q = G_q;
    int hl = -1;
    if (which == GF_ATTACK) { a = (gf).02 + (gf).88*q; hl = 0; }
    else if (which == GF_SUSTAIN) { su = q; hl = 2; }
    else if (which == GF_RELEASE) { r = (gf).02 + (gf).88*q; hl = 3; }
    gf x1 = 2 + 9*a, x2 = x1 + 3 + 4*d; if (x2 > 15) x2 = 15;
    gf x3 = 17, x4 = 18 + 6*r, sy = (gf)gf_rnd(16 - 13*su);
    gf X[6] = {1, x1, x2, x3, x4, 24}, Y[6] = {16, 3, sy, sy, 16, 16};
    for (int i = 0; i < 5; ++i) gf_line(X[i], Y[i], X[i+1], Y[i+1], i == hl ? GF_INK : GF_TRACK);
    if (G_active) {
        gf x = 1 + 23*G_p;
        for (int i = 0; i < 5; ++i) if (X[i] <= x && x <= X[i+1]) {
            gf dd = X[i+1] - X[i]; if (dd < (gf).001) dd = (gf).001;
            gf_dot(x, Y[i] + (Y[i+1] - Y[i]) * (x - X[i]) / dd); break;
        }
    }
}
static void gf_tapset(int feedback)
{
    gf_base(); gf fb = (gf).08 + (gf).9*G_q, pw = 1;
    for (int j = 0; j < 4; ++j) {
        gf h = feedback ? 14*pw : (14 - 3*j) * G_q; pw *= fb;
        if (h >= (gf).8) gf_rect(2 + j*7, 16 - h, 2, h, GF_INK);
        gf dj = G_p - (gf)j/4; if (G_active && (dj < 0 ? -dj : dj) < (gf).13) gf_rect(2 + j*7, 15, 2, 2, GF_WHITE);
    }
}
/* wave callbacks */
static gf w_sine2(gf t) { return gf_sine(t, 2); }
static gf w_sine15(gf t) { return gf_sine(t, (gf)1.5); }
static gf w_tune(gf t) { return gf_sine(t, G_cyc); }
static gf w_fm(gf t) { return gf_sinc((gf)1.5*t + G_q*(gf)0.25464790894703254*gf_sinc((gf)1.5*t)); }
static gf w_drive(gf t) { return gf_tanh(G_g * gf_sine(t, (gf)1.5)) / gf_tanh(G_g); }
static gf w_fold(gf t) { gf v = gf_sine(t, (gf)1.5) * (1 + (gf)1.6*G_q); while (v > 1 || v < -1) v = v > 1 ? 2 - v : -2 - v; return v; }
static gf w_ring(gf t) { return gf_sinc(3*t) * ((1 - G_q) + G_q*gf_sinc(t)); }
static gf w_lfo(gf t) { return gf_sinc(G_cyc*t + G_ph); }
static gf w_lfodelay(gf t) { gf x = 1 + 23*t, d = G_g; if (x <= d) return 0; gf k = (x - d) / 6; if (k > 1) k = 1; return k * gf_sinc((x - d) / (gf)8.0); }
static gf w_chorus(gf t) { return gf_sine(t + G_g, (gf)1.5); }
static gf w_unison(gf t) { return gf_sine(t + (gf)G_j*(gf).04*G_q, (gf)1.5); }
static gf gf_dist(int s, gf x)
{
    switch (s) {
    case 0: return x;
    case 1: return gf_tanh(x*2) / gf_tanh(2);
    case 2: return gf_cl(x*3, -1, 1);
    case 3: return (x < 0 ? -x : x)*2 - 1;
    case 4: return gf_sinc(x*(gf)0.8);
    case 5: return (gf)gf_rnd(x*2) / 2;
    case 6: return x < 0 ? -x : x;
    default: return gf_tanh(x*4 + (gf).5) * (gf).75;
    }
}
static gf G_th, G_ra, G_mg, G_t0, G_r; static int G_lim;
static gf gf_comp(gf t)
{
    if (G_id == GF_LIMITER) return G_lim ? (t < (gf).45 ? t : (gf).45) : t;
    return gf_cl((t < G_t0 ? t : G_t0 + (t - G_t0) / G_r) + G_mg, 0, 1);
}
static gf G_base, G_depth, G_tau;
static gf gf_pitch(gf t) { return 16 - 13*gf_cl(G_base + G_depth*gf_exp(-t / G_tau), 0, 1); }
static gf gf_decay(gf t) { return 16 - 13*gf_exp(-t / G_tau); }

/* Render one role (GF_TRACK..GF_WHITE) of glyph id into mask[GF_W*GF_H]:
 * each pixel holds its 16 4x4 sample bits; coverage = popcount/16. */
static void gf_render(int id, int value, int state, const gf_live *L, int role, uint16_t *mask)
{
    gf_m = mask; gf_pass = role;
    static const gf_live idle;
    if (!L) L = &idle;
    memset(gf_m, 0, GF_W*GF_H*sizeof *gf_m); gf_n = 0;
    gf q = gf_cl((gf)value / (gf)127, 0, 1);
    G_q = q; G_id = id;
    gf amp = gf_cl(L->amp, 0, 1), env = gf_cl(L->env, 0, 1);
    gf ph = L->phase - fm_floorf(L->phase);
    int active = L->active; gf p = gf_cl(L->progress, 0, 1);
    G_amp = amp; G_p = p; G_ph = ph; G_active = active;
    int n = gf_states[id], s = 0;
    if (n) { s = state >= 0 ? state : gf_rnd(q * (gf)(n - 1)); if (s < 0) s = 0; if (s > n - 1) s = n - 1; }
    switch (id) {
    case GF_LEVEL:
        gf_base();
        for (int j = 0; j < 7; ++j) { int h = 2 + j*2; gf_rect(3 + j*3, 16 - h, 2, h, q > 0 && (gf)j/6 <= q ? GF_INK : GF_TRACK); }
        if (active) for (int j = 0; j < 7; ++j) if ((gf)j/6 <= q*amp) gf_rect(3 + j*3, 15, 2, 1, GF_WHITE);
        break;
    case GF_PAN:
        gf_base(); gf_line(2, 9, 23, 9, GF_TRACK); gf_rect(12, 5, 2, 9, GF_GRAY);
        gf_line(2, 6, 2, 12, GF_GRAY); gf_line(23, 6, 23, 12, GF_GRAY);
        gf_rect(gf_rnd(3 + 18*q), 6, 2, 7, GF_INK); break;
    case GF_MOD_BASE: {
        gf_base(); gf_line(2, 9, 23, 9, GF_TRACK); int x = gf_rnd(3 + 18*q), dep = gf_rnd(6*(gf).5);
        int lo = x + 1 - dep < 2 ? 2 : x + 1 - dep, hi = x + 1 + dep > 23 ? 23 : x + 1 + dep;
        gf_line(lo, 12, hi, 12, GF_GRAY); gf_line(lo, 11, lo, 13, GF_GRAY); gf_line(hi, 11, hi, 13, GF_GRAY);
        gf_rect(x, 5, 2, 7, GF_INK);
        if (active) gf_dot(gf_cl(x + 1 + L->mod*dep, 2, 23), 4);
        break; }
    case GF_STEREO: {
        gf_rect(12, 2, 2, 15, GF_TRACK); int sp = gf_rnd(9*q);
        gf_rect(10 - sp, 5, 2, 9, GF_INK); gf_rect(14 + sp, 5, 2, 9, GF_INK);
        if (active) { int h = gf_rnd(amp*3); if (h) { gf_rect(10 - sp, 14 - h, 2, h, GF_WHITE); gf_rect(14 + sp, 14 - h, 2, h, GF_WHITE); } }
        break; }
    case GF_WAVE: case GF_WAVE_SHAPE: case GF_PWM:
        gf_base(); gf_line(2, 9, 22, 9, GF_TRACK);
        if (id == GF_PWM) { int w = gf_rnd(10 - 8*q); gf_begin(GF_INK); GP(2,15); GP(2,3); GP(2+w,3); GP(2+w,15); { static const int8_t P[] = {44,30,44,6}; gf_pl(P, sizeof P, GF_INK); } }
        else if (id == GF_WAVE_SHAPE) {
            if (s == 0) gf_wave(w_sine2, 6, 9, GF_INK, 2, 22, 92);
            else if (s == 1) { { static const int8_t P[] = {4,18,9,6,19,30,29,6,39,30,44,18}; gf_pl(P, sizeof P, GF_INK); } }
            else if (s == 2) { { static const int8_t P[] = {4,30,24,6,24,30,44,6,44,30}; gf_pl(P, sizeof P, GF_INK); } }
            else { { static const int8_t P[] = {4,30,4,6,14,6,14,30,24,30,24,6,34,6,34,30,44,30,44,6}; gf_pl(P, sizeof P, GF_INK); } }
        } else for (int x = 2; x <= 12; x += 10) {
            gf_begin(GF_INK); GP(x, 15 - 12*q); GP(x+5, 9 - 6*q); GP(x+5, 9 + 6*q); GP(x+10, 3 + 12*q); GP(x+10, 15 - 12*q); gf_poly(GF_INK);
        }
        gf_pips(); break;
    case GF_SUB: {
        { static const int8_t P[] = {4,8,4,2,14,2,14,8,24,8,24,2,34,2,34,8,44,8,44,2}; gf_pl(P, sizeof P, GF_TRACK); }
        int a = 1 + gf_rnd(4*q), y = 11;
        gf_begin(GF_INK); GP(2, y+a); GP(2, y-a); GP(12, y-a); GP(12, y+a); GP(22, y+a); GP(22, y-a); gf_poly(GF_INK);
        gf_base(); gf_pips(); break; }
    case GF_NOISE: case GF_SNAPPY: {
        gf_base(); gf a = 6*q*(active ? (gf).25 + (gf).75*amp : 1);
        int seed = active ? (int)(L->progress*25) : 0;
        gf_begin(GF_INK); for (int j = 0; j < 24; ++j) {
            gf v = (gf)((((j + seed)*73 + 19) % 127)) / (gf)63.5 - 1;
            gf k = id == GF_SNAPPY ? gf_exp(-(gf)j / (gf)9.0) : 1;
            GP(1 + j, 9 - a*k*v);
        }
        gf_poly(GF_INK);
        if (id == GF_SNAPPY) gf_line(1, 3, 1, 15, GF_TRACK);
        break; }
    case GF_TUNE: {
        gf_base(); gf_line(12, 14, 12, 17, GF_GRAY); gf_rect(1 + gf_rnd(22*q), 15, 2, 2, GF_INK);
        G_cyc = 1 + (gf)2.5*q; gf am = active ? 2 + 4*amp : 5;
        gf_wave(w_tune, am, 8, GF_INK, 1, 24, 92);
        if (active) gf_dot(1 + 23*p, 8 - am*gf_sine(p, G_cyc));
        break; }
    case GF_GLIDE: case GF_SLIDE: {
        gf_base(); int width = gf_rnd(18*q); int x0 = gf_rnd((gf)(25 - width) / 2);
        gf X[4] = {1, (gf)x0, (gf)(x0 + width), 24}, Y[4] = {15, 15, 3, 3};
        if (id == GF_SLIDE) { X[0] = 4; X[3] = 21; gf_rect(1, 14, 3, 3, GF_INK); gf_rect(22, 2, 3, 3, GF_INK); }
        if (active && p < 1) {
            gf fx = x0 + p*width, fy = 15 - 12*p;
            gf_begin(GF_INK); GP(X[0], Y[0]); GP(X[1], Y[1]); GP(fx, fy); gf_poly(GF_INK);
            gf_begin(GF_TRACK); GP(fx, fy); GP(X[2], Y[2]); GP(X[3], Y[3]); gf_poly(GF_TRACK); gf_dot(fx, fy);
        } else { gf_begin(GF_INK); for (int i = 0; i < 4; ++i) GP(X[i], Y[i]); gf_poly(GF_INK); }
        break; }
    case GF_FM: case GF_DRIVE: case GF_FOLD: case GF_BITS: case GF_SAMPLE_RATE: case GF_RINGMOD:
        gf_base(); gf_line(1, 9, 24, 9, GF_TRACK);
        if (id == GF_BITS) {
            int levels = gf_rnd(6 - 4*q); gf prev = 0; int have = 0;
            gf_begin(GF_INK); for (int x = 1; x < 25; ++x) {
                gf v = gf_sine((gf)(x - 1) / (gf)23.0, (gf)1.5);
                gf y = 9 - (gf)gf_rnd(v*levels)*6/(gf)levels;
                if (have && y != prev) { GP(x - (gf).5, prev); GP(x - (gf).5, y); }
                GP(x, y); prev = y; have = 1;
            }
            gf_poly(GF_INK);
        } else if (id == GF_SAMPLE_RATE) {
            int steps = gf_rnd(23 - 19*q); if (steps < 3) steps = 3;
            int xs[32]; for (int j = 0; j <= steps; ++j) xs[j] = 1 + gf_rnd(23*(gf)j/(gf)steps);
            gf_begin(GF_INK); for (int j = 0; j < steps; ++j) { gf y = 9 - 6*gf_sine((gf)j/(gf)steps, (gf)1.5); GP(xs[j], y); GP(xs[j+1], y); }
            gf_poly(GF_INK);
        } else {
            gf_fn fn = id == GF_FM ? w_fm : id == GF_DRIVE ? w_drive : id == GF_FOLD ? w_fold : w_ring;
            G_g = (gf).3 + 6*q;
            gf_wave(fn, 6, 9, GF_INK, 1, 24, 92);
        }
        gf_pips(); break;
    case GF_DISTORTION:
        gf_base(); gf_line(3, 15, 22, 3, GF_TRACK);
        gf_begin(GF_INK); for (int i = 0; i < 65; ++i) GP(3 + 19*(gf)i/64, 9 - 6*gf_cl(gf_dist(s, (gf)i/32 - 1), -1, 1));
        gf_poly(GF_INK); break;
    case GF_CUTOFF: case GF_HIGHPASS: case GF_RESONANCE: case GF_ENV_AMOUNT: case GF_FILTER_MODE: case GF_BANDWIDTH: {
        gf_base();
        gf cut = gf_cl(L->has_cutoff ? L->cutoff : (id == GF_CUTOFF || id == GF_HIGHPASS) ? q : (gf).5, 0, 1);
        gf res = gf_cl(L->has_resonance ? L->resonance : id == GF_RESONANCE ? q : (gf).12, 0, 1);
        if (id == GF_ENV_AMOUNT) {
            gf dep = 2*q - 1, base = (gf).5, bx = 5 + 14*base, target = gf_cl(base + dep*(gf).45, 0, 1), tx = 5 + 14*target;
            gf_line(gf_rnd(bx), 15, gf_rnd(bx), 17, GF_GRAY);
            gf dd = tx - bx; if ((dd < 0 ? -dd : dd) >= 2) gf_arrow(bx, 3, tx, 3, GF_GRAY);
            cut = gf_cl(base + dep*(gf).45*gf_cl(L->filter_env > 0 || !active ? (active ? L->filter_env : 0) : 0, 0, 1), 0, 1);
            gf_filter(0, cut, res, GF_INK);
        } else if (id == GF_FILTER_MODE) {
            if (s == 0) { gf_line(1, 7, 24, 7, GF_TRACK); gf_line(9, 3, 16, 12, GF_INK); }
            else gf_filter(s == 1 ? 0 : s == 2 ? 2 : 1, (gf).5, (gf).35, GF_INK);
        } else if (id == GF_BANDWIDTH) {
            int w = gf_rnd(2 + 6*q); gf_begin(GF_INK); GP(1,16); GP(12-w-3,16); GP(12-w,5); GP(13+w,5); GP(13+w+3,16); GP(24,16); gf_poly(GF_INK);
        } else {
            gf_filter(id == GF_HIGHPASS ? 1 : 0, cut, res, GF_INK);
            if (active && amp > (gf).05) for (int x = 2; x < 24; x += 3) {
                int passed = ((gf)x < 5 + 14*cut) == (id != GF_HIGHPASS);
                int h = gf_rnd((passed ? 8 : 2)*amp); if (h < 1) h = 1;
                gf_rect(x, 16 - h, 1, h, GF_WHITE);
            }
        }
        break; }
    case GF_ATTACK: case GF_SUSTAIN: case GF_RELEASE: gf_base(); gf_envshape(id); break;
    case GF_DECAY:
        gf_base(); G_tau = (gf).035 + (gf).38*q; gf_line(1, 16, 1, 3, GF_TRACK);
        if (active) {
            int k = gf_rnd(92*p);
            gf_begin(GF_TRACK); for (int i = 0; i <= k; ++i) GP(1 + (gf)i*23/92, gf_decay((gf)i/92)); gf_poly(GF_TRACK);
            gf_begin(GF_INK); for (int i = k; i <= 92; ++i) GP(1 + (gf)i*23/92, gf_decay((gf)i/92)); gf_poly(GF_INK);
            gf_dot(1 + 23*p, gf_decay(p));
        } else { gf_begin(GF_INK); for (int i = 0; i <= 92; ++i) GP(1 + (gf)i*23/92, gf_decay((gf)i/92)); gf_poly(GF_INK); }
        break;
    case GF_GATE_MODE:
        gf_base();
        gf_begin(GF_INK); if (s) { GP(1,16); GP(4,16); GP(4,3); GP(20,3); GP(20,16); GP(24,16); }
        else { GP(1,16); GP(5,3); GP(11,9); GP(18,9); GP(24,16); }
        gf_poly(GF_INK); break;
    case GF_TRANSIENT: {
        gf_base(); int h = gf_rnd(2 + 12*q); gf_rect(2, 16 - h, 2, h, active && p < (gf).15 ? GF_WHITE : GF_INK);
        gf body = active ? amp : 1;
        gf_begin(GF_INK); GP(5, 16 - 6*body); GP(9, 16 - 4*body); GP(15, 16 - 2*body); GP(24, 16); gf_poly(GF_INK); break; }
    case GF_PITCH_DECAY: case GF_PITCH_DEPTH: case GF_PITCH_BASE:
        gf_base(); G_tau = id == GF_PITCH_DECAY ? (gf).08 + (gf).45*q : (gf).25;
        G_depth = id == GF_PITCH_DEPTH ? (gf).05 + (gf).75*q : (gf).6; G_base = id == GF_PITCH_BASE ? (gf).08 + (gf).45*q : (gf).15;
        gf_begin(GF_INK); for (int i = 0; i <= 92; ++i) GP(1 + (gf)i*23/92, gf_pitch((gf)i/92)); gf_poly(GF_INK);
        if (id == GF_PITCH_DEPTH) gf_line(23, gf_rnd(gf_pitch(0)), 23, gf_rnd(gf_pitch(1)), GF_GRAY);
        else if (id == GF_PITCH_DECAY) gf_line(1, 17, gf_rnd(1 + 23*G_tau*(gf)1.6), 17, GF_GRAY);
        else gf_line(19, gf_rnd(gf_pitch(1)) + 2, 24, gf_rnd(gf_pitch(1)) + 2, GF_GRAY);
        if (active) gf_dot(1 + 23*p, gf_pitch(p));
        break;
    case GF_LFO_RATE: case GF_LFO_DEPTH: {
        gf_base(); gf_line(1, 9, 24, 9, GF_TRACK);
        G_cyc = id == GF_LFO_RATE ? 1 + (gf)2.5*q : 2; gf am = id == GF_LFO_RATE ? 6 : 1 + 5*q;
        gf_wave(w_lfo, am, 9, GF_INK, 1, 24, 92);
        if (active) gf_dot(24, 9 - am*gf_sinc(G_cyc + ph));
        break; }
    case GF_LFO_DELAY: {
        gf_base(); gf_line(1, 9, 24, 9, GF_TRACK); int d = 1 + gf_rnd(13*q); G_g = (gf)d;
        gf_wave(w_lfodelay, 6, 9, GF_INK, 1, 24, 92); gf_line(d, 6, d, 12, GF_GRAY);
        if (active) gf_dot(1 + 23*p, 9 - 6*w_lfodelay(p));
        break; }
    case GF_CHORUS: {
        gf_base(); int sep = gf_rnd(2 + 4*q); G_g = ((gf).06 + (gf).2*q)*gf_sinc(ph);
        int m0 = 8 - sep/2;
        gf_wave(w_sine15, 2, m0, GF_TRACK, 1, 24, 92);
        gf_wave(w_chorus, 2, m0 + sep, GF_INK, 1, 24, 92);
        if (active) gf_dot(24, m0 + sep - 2*gf_sine(1 + G_g, (gf)1.5));
        break; }
    case GF_PHASER: {
        gf_base(); gf w = 1 + 3*q; int off = active ? gf_rnd(gf_sinc(ph)*3) : 0;
        gf_begin(GF_INK); GP(1, 4);
        for (int i = 0; i < 2; ++i) { int x = (i ? 17 : 7) + off; GP(x - w, 4); GP(x, 4 + 11*((gf).4 + (gf).6*q)); GP(x + w, 4); }
        GP(24, 4); gf_poly(GF_INK); break; }
    case GF_FEEDBACK: case GF_DELAY_SEND: gf_tapset(id == GF_FEEDBACK); break;
    case GF_REVERB_SEND:
        gf_base(); gf_rect(1, 3, 2, 13, GF_GRAY);
        for (int j = 0; j < 10; ++j) { int h = gf_rnd(13*q*gf_exp(-(gf)j/(gf)5.0)); if (h >= 1) gf_rect(5 + j*2, 16 - h, 1, h, GF_INK); }
        if (active) gf_dot(5 + 18*p, 16 - gf_rnd(13*q*gf_exp(-9*p/(gf)5.0)));
        break;
    case GF_DELAY_TIME: {
        gf_base(); int gap = gf_rnd(3 + 7*q); gf_rect(1, 3, 2, 13, GF_INK);
        for (int x = 1 + gap; x < 24; x += gap) gf_rect(x, 8, 1, 8, GF_INK);
        gf_line(1, 17, 1 + gap, 17, GF_GRAY);
        if (active) gf_dot(1 + 23*p, 16);
        break; }
    case GF_TONE: {
        gf_base(); gf v = 2*q - 1; int a = gf_rnd(5*v);
        gf_begin(GF_INK); GP(1, 9 + a); GP(8, 9 + a); GP(17, 9 - a); GP(24, 9 - a); gf_poly(GF_INK);
        gf_line(12, 4, 12, 14, GF_TRACK); break; }
    case GF_TAPE: case GF_DELAY_TYPE:
        if (id == GF_DELAY_TYPE && s == 0) { gf_base(); for (int j = 0; j < 4; ++j) gf_rect(2 + j*7, 3 + j*3, 2, 13 - j*3, GF_INK); }
        else {
            gf tq = id == GF_TAPE ? q : (gf).5;
            gf a = (id == GF_TAPE && L->tape_active) ? L->tape_phase : 0;
            for (int c = 0; c < 2; ++c) {
                int cx = c ? 19 : 6;
                gf_circle(cx, 7, 5, GF_INK, 0); gf_rect(cx, 7, 1, 1, GF_INK);
                for (int k = 0; k < 3; ++k) {
                    gf u = a + (gf)k/(gf)3.0;
                    gf_line(cx + 2*gf_sinc(u + (gf).25), 7 + 2*gf_sinc(u), cx + (gf)3.5*gf_sinc(u + (gf).25), 7 + (gf)3.5*gf_sinc(u), GF_INK);
                }
            }
            { static const int8_t P[] = {2,16,2,28,48,28,48,16}; gf_pl(P, sizeof P, GF_TRACK); }
            if (id == GF_TAPE) gf_rect(2, 16, 1 + gf_rnd(21*tq), 1, GF_INK);
        }
        break;
    case GF_WEAR: {
        static const int gaps[6] = {5, 11, 17, 21, 8, 14};
        gf_line(1, 5, 24, 5, GF_TRACK); gf_line(1, 13, 24, 13, GF_TRACK); int ng = gf_rnd(6*q);
        for (int x = 1; x < 25; ++x) { int g = 0; for (int i = 0; i < ng; ++i) g |= gaps[i] == x; if (!g) gf_rect(x, 7, 1, 5, GF_INK); }
        break; }
    case GF_PINGPONG:
        gf_line(1, 5, 24, 5, GF_TRACK); gf_line(1, 13, 24, 13, GF_TRACK);
        for (int j = 0; j < 4; ++j) gf_rect(2 + 7*j, (j % 2 == 0 || s == 0) ? 3 : 11, 2, 5, GF_INK);
        if (s) { { static const int8_t P[] = {6,16,18,20,32,16,46,20}; gf_pl(P, sizeof P, GF_GRAY); } }
        break;
    case GF_THRESHOLD: case GF_RATIO: case GF_MAKEUP: case GF_COMPRESSOR: case GF_LIMITER:
        gf_base(); gf_line(2, 16, 23, 2, GF_TRACK);
        G_th = id == GF_THRESHOLD ? (gf).2 + (gf).65*q : (gf).5;
        G_ra = id == GF_RATIO ? 1 + 9*q : 4;
        G_mg = id == GF_MAKEUP ? (gf).35*q : 0; G_lim = s;
        if (id == GF_COMPRESSOR) { G_t0 = (gf).8 - (gf).55*q; G_r = 1 + 5*q; } else { G_t0 = G_th; G_r = G_ra; }
        gf_begin(GF_INK); for (int i = 0; i < 65; ++i) GP(2 + (gf)i*21/64, 16 - 14*gf_comp((gf)i/64)); gf_poly(GF_INK);
        if (id == GF_THRESHOLD) { int y = gf_rnd(16 - 14*G_th); gf_line(2, y, 23, y, GF_GRAY); }
        if (id == GF_LIMITER && s) { int y = gf_rnd(16 - 14*(gf).45); gf_line(2, y, 23, y, GF_GRAY); }
        if (id == GF_MAKEUP) { gf m = G_mg; G_mg = 0; gf_begin(GF_GRAY); for (int i = 0; i < 65; ++i) GP(2 + (gf)i*21/64, 16 - 14*gf_comp((gf)i/64)); gf_poly(GF_GRAY); G_mg = m; }
        if (id == GF_COMPRESSOR && active) gf_dot(2 + 21*amp, 16 - 14*gf_comp(amp));
        break;
    case GF_BLEND: {
        gf_base(); int hd = gf_rnd(13*(1 - q)), hw = gf_rnd(13*q);
        gf_rect(5, 16 - hd, 4, hd, GF_GRAY); gf_rect(16, 16 - hw, 4, hw, GF_INK); break; }
    case GF_PUMP: {
        gf_base(); int depth = gf_rnd(12*q);
        gf_begin(GF_INK); GP(1,3); GP(4,3); GP(5, 3 + depth);
        for (int i = 1; i < 25; ++i) { gf u = 1 - (gf)i/24; GP(5 + (gf)i*18/24, 3 + depth*(u*u)); }
        GP(24, 3); gf_poly(GF_INK);
        if (active) gf_dot(5 + 18*p, 3 + depth*((1 - p)*(1 - p)));
        break; }
    case GF_SOURCE: {
        int cnt = 3;
        for (int j = 0; j < cnt; ++j) {
            int y = gf_rnd(2 + (gf)j*14/(gf)(cnt - 1 > 1 ? cnt - 1 : 1));
            gf_rect(1, y - 1, 3, 3, j == s ? GF_INK : GF_TRACK);
            gf_begin(j == s ? GF_INK : GF_TRACK); GP(4, y); GP(16, y); GP(16, 9); gf_poly(j == s ? GF_INK : GF_TRACK);
        }
        gf_line(16, 9, 21, 9, GF_INK); gf_circle(22, 9, 1, GF_INK, 1); break; }
    case GF_TEMPO: {
        { static const int8_t P[] = {8,34,18,4,32,4,42,34,8,34}; gf_pl(P, sizeof P, GF_INK); }
        int playing = (L->playing) != 0;
        gf angle = playing ? (gf).5*gf_sinc(L->clock_phase) : 0;
        int wy = gf_rnd(5 + 7*q);
        const gf TWO_PI = (gf)6.283185307179586;
        gf x = (gf)12.5 + 12*gf_sinc(angle / TWO_PI)*1, y = 15 - 12*gf_sinc(angle / TWO_PI + (gf).25);
        gf_line((gf)12.5, 15, x, y, playing ? GF_WHITE : GF_GRAY);
        gf wx = (gf)12.5 + (15 - wy)*gf_sinc(angle / TWO_PI)/(gf)1.0*0 + (x - (gf)12.5)*(15 - wy)/12;
        gf wyy = 15 - (15 - y)*(15 - wy)/12;
        gf_rect(wx - (gf)1.5, wyy - 1, 3, 2, GF_INK); break; }
    case GF_SWING:
        gf_base(); gf_rect(2, 3, 1, 13, GF_INK); gf_rect(gf_rnd(12 + 8*q), 7, 1, 9, GF_INK); gf_rect(23, 3, 1, 13, GF_INK); break;
    case GF_LENGTH: {
        int k = 1 + gf_rnd(q*7);
        for (int j = 0; j < 8; ++j) gf_rect(1 + j*3, 6, 2, 7, j < k ? GF_INK : GF_TRACK);
        gf_line(k*3, 3, k*3, 16, GF_GRAY); gf_base(); break; }
    case GF_DENSITY: {
        int k = gf_rnd(q*8);
        for (int j = 0; j < 8; ++j) { if ((j*k)/8 != ((j + 1)*k)/8) gf_rect(1 + j*3, 5, 2, 11, GF_INK); else gf_rect(1 + j*3, 15, 2, 1, GF_TRACK); }
        gf_base(); break; }
    case GF_SLOT: { int k = gf_rnd(q*7); for (int j = 0; j < 8; ++j) gf_rect(2 + (j % 4)*6, 3 + (j / 4)*7, 4, 5, j == k ? GF_INK : GF_TRACK); break; }
    case GF_CHANCE: {
        static const int8_t px[] = {12, 9,15, 9,12,15, 9,15,9,15, 9,15,12,9,15, 9,15,9,15,9,15};
        static const int8_t py[] = {9, 6,12, 6,9,12, 6,6,12,12, 6,6,9,12,12, 6,6,9,9,12,12};
        static const int8_t start[] = {0, 0, 1, 3, 6, 10, 15, 21};
        { static const int8_t P[] = {12,6,36,6,36,30,12,30,12,6}; gf_pl(P, sizeof P, GF_INK); }
        int k = gf_rnd(q*6);
        for (int i = start[k]; i < start[k + 1]; ++i) gf_rect(px[i] - 1, py[i] - 1, 2, 2, GF_INK);
        break; }
    case GF_MUTATE: {
        static const int base[6] = {11, 7, 9, 4, 12, 8}, rank[6] = {4, 0, 2, 5, 1, 3}; /* order.index(j) */
        int k = gf_rnd(q*6);
        for (int j = 0; j < 6; ++j) {
            int x = 1 + j*4, y = base[j]; gf_rect(x, y, 3, 2, GF_TRACK);
            if (rank[j] < k) gf_rect(x, y + (y > 8 ? -3 : 4), 3, 2, GF_INK);
        }
        gf_base(); break; }
    case GF_REPEAT: case GF_RATE: {
        gf_base(); int count = id == GF_REPEAT ? 1 + gf_rnd(q*7) : 2 + gf_rnd(q*6);
        for (int i = 0; i < count; ++i) {
            int x = 2 + gf_rnd((gf)i*21/(gf)(count - 1 > 1 ? count - 1 : 1));
            gf_rect(x, id == GF_RATE ? 5 : 4 + i, 1, id == GF_RATE ? 11 : 12 - i, GF_INK);
        }
        if (id == GF_RATE && active) gf_dot(2 + 21*p, 16);
        break; }
    case GF_DIRECTION:
        gf_base();
        if (s == 0) gf_arrow(2, 9, 23, 9, GF_INK);
        else if (s == 1) gf_arrow(23, 9, 2, 9, GF_INK);
        else if (s == 2) { gf_arrow(4, 9, 23, 9, GF_INK); gf_arrow(21, 9, 2, 9, GF_INK); }
        else { { static const int8_t P[] = {4,28,16,8,28,28}; gf_pl(P, sizeof P, GF_INK); } gf_arrow(14, 14, 22, 5, GF_INK); }
        break;
    case GF_ROOT: case GF_SCALE: case GF_KEY_RANGE: {
        static const int white[7] = {0, 2, 4, 5, 7, 9, 11}, bpc[5] = {1, 3, 6, 8, 10}, bk[5] = {0, 1, 3, 4, 5};
        static const uint16_t scales[5] = {0xfff, 0xab5, 0x5ad, 0x6ad, 0x295};
        unsigned mask;
        if (id == GF_ROOT) mask = 1u << gf_rnd(q*11);
        else if (id == GF_SCALE) mask = scales[gf_rnd(q*4)];
        else { int hi = gf_rnd(q*11); mask = (2u << hi) - 1; }
        for (int i = 0; i < 7; ++i) gf_rect(2 + i*3, 2, 2, 14, mask >> white[i] & 1 ? GF_INK : GF_TRACK);
        for (int i = 0; i < 5; ++i) { int g = 4 + 3*bk[i]; gf_erase(g - 2, 2, 5, 8); gf_rect(g - 1, 2, 3, 7, mask >> bpc[i] & 1 ? GF_INK : GF_GRAY); }
        if (id == GF_KEY_RANGE) gf_line(1, 17, 24, 17, GF_TRACK);
        break; }
    case GF_NOTE: {
        for (int y = 4; y <= 14; y += 5) gf_line(2, y, 23, y, GF_TRACK);
        gf yy = 15 - 12*q; gf_circle(11, yy, 2, GF_INK, 1);
        if (yy > 8) gf_line(13, yy, 13, yy - 6, GF_INK); else gf_line(9, yy, 9, yy + 6, GF_INK);
        if (active && p < (gf).5) gf_dot(11, yy);
        break; }
    case GF_GATE: {
        gf_base(); int end = gf_rnd(3 + 20*q); gf_begin(GF_INK); GP(1,15); GP(2,15); GP(2,4); GP(end,4); GP(end,15); GP(24,15); gf_poly(GF_INK);
        if (active) gf_dot(2 + 21*p, 2 + 21*p <= end ? 4 : 15);
        break; }
    case GF_LINK: { int dx = s ? 0 : 2; gf_circle(9 - dx, 9, 4, GF_INK, 0); gf_circle(16 + dx, 9, 4, GF_INK, 0); break; }
    case GF_OCTAVE: {
        gf_base(); int steps = 1 + s, x = 1; gf_begin(GF_INK); GP(1, 15);
        for (int j = 0; j < steps; ++j) { x += 5; GP(x, 15 - j*3); GP(x, 12 - j*3); }
        GP(24, 12 - (steps - 1)*3); gf_poly(GF_INK); break; }
    case GF_TRACKING:
        gf_base(); gf_line(2, 16, 23, 16 - 13*q, GF_INK);
        for (int x = 2; x < 24; x += 3) gf_line(x, 15, x, 17, GF_GRAY);
        if (active) gf_dot(2 + 21*p, 16 - 13*q*p);
        break;
    case GF_PEDAL: {
        gf qq = (gf)s; gf_rect(2, 3, 7, 2, GF_INK); int tail = gf_rnd(15*qq);
        if (tail) gf_rect(9, 3, tail, 2, GF_GRAY);
        gf_line(3, 16, 23, 16, GF_TRACK); gf_line(4, gf_rnd(15 - 7*(1 - qq)), 21, 15, GF_INK); gf_circle(21, 15, 1, GF_INK, 1);
        if (active) gf_line(2, 7, 2 + gf_rnd(21*env), 7, GF_WHITE);
        break; }
    case GF_ENABLE:
        gf_line(1, 10, 8, 10, GF_INK); gf_line(17, 10, 24, 10, GF_INK);
        gf_circle(8, 10, 1, GF_INK, 1); gf_circle(17, 10, 1, GF_INK, 1);
        if (s) gf_line(9, 10, 16, 10, GF_INK); else gf_line(9, 10, 15, 4, GF_INK);
        break;
    case GF_DESTINATION: {
        int y = 3 + s*6;
        for (int j = 0; j < 3; ++j) { gf_begin(GF_TRACK); GP(8,9); GP(8, 3 + j*6); GP(20, 3 + j*6); gf_poly(GF_TRACK); }
        gf_begin(GF_INK); GP(1,9); GP(8,9); GP(8,y); GP(20,y); gf_poly(GF_INK);
        for (int j = 0; j < 3; ++j) gf_rect(21, 2 + j*6, 3, 3, j == s ? GF_INK : GF_TRACK);
        break; }
    case GF_UNISON: {
        int k = 1 + gf_rnd(q*3);
        for (int j = 0; j < k; ++j) { G_j = j; gf_wave(w_unison, 1, gf_rnd(9 - (k - 1)*2 + j*4), GF_INK, 1, 24, 92); }
        break; }
    case GF_MOD_MODE:
        gf_base();
        if (s == 0) gf_wave(w_sine2, 5, 9, GF_INK, 1, 24, 92);
        else if (s == 1) { gf_line(1, 16, 1, 3, GF_GRAY); gf_wave(w_sine2, 5, 9, GF_INK, 3, 24, 92); }
        else { gf_line(1, 16, 1, 3, GF_GRAY); { static const int8_t P[] = {6,28,14,6,24,28,48,28}; gf_pl(P, sizeof P, GF_INK); } }
        break;
    case GF_LEGATO:
        gf_rect(2, 10, 8, 3, GF_INK); gf_rect(15, 6, 8, 3, GF_INK);
        if (s) { { static const int8_t P[] = {6,10,14,4,36,4,44,8}; gf_pl(P, sizeof P, GF_INK); } }
        else { gf_line(4, 15, 4, 17, GF_TRACK); gf_line(17, 11, 17, 14, GF_TRACK); }
        break;
    case GF_LOOP:
        gf_arrow(3, 4, 23, 4, GF_INK);
        if (s) { { static const int8_t P[] = {46,8,46,28,6,28,6,8}; gf_pl(P, sizeof P, GF_INK); } gf_arrow(10, 14, 3, 14, GF_INK); }
        else { { static const int8_t P[] = {6,8,6,28,36,28}; gf_pl(P, sizeof P, GF_TRACK); } }
        break;
    }
}

#endif
