/* SPDX-License-Identifier: GPL-3.0-only */
/* R22 drum voice / SOUND page. Generated paths from
 * from-dot/R22-gallery/views/909-voice-00..10.svg and 808-voice-00..10.svg.
 * Included from ui.c after drum_strip_glyph. */

static const char *const DV_NAME_909[NDRUM] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP", "CLOSED HAT", "OPEN HAT", "CRASH", "RIDE"};
static const char *const DV_NAME_808[NDRUM] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP", "COWBELL", "CYMBAL", "OPEN HAT", "CLOSED HAT"};

static const char *dv_voice_name(int t, int v)
{
    if (v < 0 || v >= NDRUM) return "KIT";
    return t == T_808 ? DV_NAME_808[v] : DV_NAME_909[v];
}
static void dv_full_title(char *t, int part, int v)
{
    put_s(put_s(t, part == PART_808 ? "808 " : "909 "), dv_voice_name(part == PART_808 ? T_808 : T_909, v));
}

/* Lossless coordinate stream: exact multiples of 100 in 800..13500
 * use one byte; every other authored 15-bit coordinate uses two bytes.
 * This is an exact representation, not geometric quantisation. */
static const uint8_t DV_XY_PACKED[] = {
    17, 48, 142, 141, 148, 194, 146, 242, 147, 142, 150, 243, 146, 69, 60, 144,
    229, 157, 201, 143, 112, 160, 158, 141, 228, 163, 15, 140, 67, 87, 19, 92,
    137, 90, 97, 136, 127, 102, 135, 252, 107, 12, 112, 135, 252, 117, 136, 127,
    122, 137, 90, 127, 19, 182, 223, 140, 67, 185, 96, 141, 228, 188, 62, 143,
    112, 191, 122, 144, 229, 195, 20, 146, 69, 199, 12, 147, 142, 203, 97, 148,
    194, 208, 20, 48, 199, 74, 55, 190, 128, 60, 181, 182, 63, 107, 64, 164,
    34, 63, 62, 60, 146, 142, 55, 17, 48, 17, 48, 188, 40, 46, 187, 146,
    153, 139, 185, 219, 68, 183, 31, 161, 63, 124, 163, 251, 175, 107, 165, 178,
    102, 90, 166, 133, 165, 178, 80, 163, 251, 158, 209, 161, 63, 156, 21, 68,
    154, 94, 153, 139, 58, 46, 154, 94, 144, 165, 156, 21, 24, 158, 209, 136,
    241, 80, 134, 53, 166, 133, 132, 126, 102, 2, 175, 107, 132, 126, 124, 134,
    53, 183, 31, 136, 241, 185, 219, 24, 187, 146, 144, 165, 188, 40, 46, 190,
    128, 16, 189, 231, 138, 254, 188, 37, 24, 185, 90, 141, 203, 181, 182, 142,
    202, 177, 121, 143, 105, 107, 32, 168, 95, 143, 105, 164, 34, 142, 202, 160,
    126, 141, 203, 157, 179, 24, 155, 241, 138, 254, 62, 16, 155, 241, 135, 194,
    157, 179, 8, 160, 126, 132, 245, 164, 34, 131, 246, 168, 95, 131, 87, 107,
    0, 177, 121, 131, 87, 181, 182, 131, 246, 185, 90, 132, 245, 188, 37, 8,
    189, 231, 135, 194, 190, 128, 16, 193, 160, 16, 192, 235, 138, 254, 190, 218,
    24, 187, 144, 141, 203, 183, 70, 142, 202, 178, 72, 143, 105, 107, 32, 167,
    144, 143, 105, 162, 146, 142, 202, 158, 72, 141, 203, 154, 254, 24, 152, 237,
    138, 254, 54, 16, 152, 237, 135, 194, 154, 254, 8, 158, 72, 132, 245, 162,
    146, 131, 246, 167, 144, 131, 87, 107, 0, 178, 72, 131, 87, 183, 70, 131,
    246, 187, 144, 132, 245, 190, 218, 8, 192, 235, 135, 194, 193, 160, 16, 197,
    136, 16, 196, 177, 138, 254, 194, 60, 24, 190, 83, 141, 203, 185, 58, 142,
    202, 179, 75, 143, 105, 107, 32, 166, 141, 143, 105, 160, 158, 142, 202, 155,
    133, 141, 203, 151, 156, 24, 149, 39, 138, 254, 44, 16, 149, 39, 135, 194,
    151, 156, 8, 155, 133, 132, 245, 160, 158, 131, 246, 166, 141, 131, 87, 107,
    0, 179, 75, 131, 87, 185, 58, 131, 246, 190, 83, 132, 245, 194, 60, 8,
    196, 177, 135, 194, 197, 136, 16, 198, 80, 62, 197, 97, 157, 120, 194, 166,
    159, 114, 190, 78, 161, 37, 184, 164, 162, 115, 178, 12, 163, 68, 102, 83,
    163, 228, 163, 68, 67, 162, 115, 151, 162, 161, 37, 147, 74, 159, 114, 144,
    143, 157, 120, 32, 62, 144, 143, 153, 56, 147, 74, 151, 62, 151, 162, 149,
    139, 67, 148, 61, 163, 228, 147, 108, 102, 41, 178, 12, 147, 108, 184, 164,
    148, 61, 190, 78, 149, 139, 194, 166, 151, 62, 197, 97, 153, 56, 198, 80,
    62, 200, 68, 22, 199, 85, 141, 99, 196, 154, 142, 241, 192, 66, 144, 71,
    186, 152, 145, 77, 180, 0, 145, 242, 107, 146, 42, 165, 216, 145, 242, 72,
    145, 77, 153, 150, 144, 71, 149, 62, 142, 241, 146, 131, 141, 99, 37, 22,
    146, 131, 138, 13, 149, 62, 136, 127, 153, 150, 135, 41, 72, 134, 35, 165,
    216, 133, 126, 107, 133, 70, 180, 0, 133, 126, 186, 152, 134, 35, 192, 66,
    135, 41, 196, 154, 136, 127, 199, 85, 138, 13, 200, 68, 22, 12, 44, 82,
    25, 87, 138, 229, 92, 137, 102, 97, 136, 104, 102, 135, 233, 107, 135, 235,
    112, 136, 108, 117, 137, 110, 122, 20, 210, 8, 29, 202, 118, 145, 181, 194,
    104, 148, 61, 185, 220, 150, 13, 117, 151, 37, 167, 78, 151, 132, 67, 151,
    43, 146, 204, 150, 26, 12, 44, 12, 44, 27, 28, 47, 138, 165, 67, 13,
    87, 134, 189, 107, 8, 127, 134, 189, 188, 140, 13, 196, 92, 138, 165, 204,
    44, 28, 196, 92, 144, 95, 188, 140, 146, 4, 127, 147, 2, 107, 147, 86,
    87, 147, 2, 67, 146, 4, 47, 144, 95, 27, 28, 27, 28, 27, 46, 47,
    145, 151, 67, 143, 23, 87, 141, 150, 107, 141, 22, 127, 141, 150, 188, 140,
    143, 23, 196, 92, 145, 151, 204, 44, 46, 196, 92, 151, 81, 188, 140, 152,
    231, 127, 153, 219, 107, 59, 87, 153, 219, 67, 152, 231, 47, 151, 81, 27,
    46, 27, 46, 27, 75, 47, 158, 29, 67, 156, 119, 87, 155, 122, 107, 155,
    38, 127, 155, 122, 188, 140, 156, 119, 196, 92, 158, 29, 204, 44, 75, 196,
    92, 162, 252, 188, 140, 164, 209, 127, 165, 234, 107, 90, 87, 165, 234, 67,
    164, 209, 47, 162, 252, 27, 75, 27, 75, 76, 2, 184, 164, 2, 200, 68,
    85, 193, 110, 166, 163, 186, 152, 168, 72, 179, 194, 169, 70, 107, 169, 154,
    166, 22, 169, 70, 72, 168, 72, 152, 106, 166, 163, 37, 85, 76, 2, 32,
    62, 32, 82, 150, 118, 165, 75, 67, 166, 210, 164, 34, 167, 188, 102, 168,
    10, 177, 206, 167, 188, 184, 164, 166, 210, 191, 122, 165, 75, 198, 80, 82,
    198, 80, 62, 37, 22, 37, 69, 152, 106, 160, 230, 72, 162, 234, 166, 22,
    164, 31, 107, 164, 134, 179, 194, 164, 31, 186, 152, 162, 234, 193, 110, 160,
    230, 200, 68, 69, 200, 68, 22, 44, 16, 44, 69, 154, 119, 160, 77, 160,
    158, 161, 227, 166, 197, 162, 215, 107, 82, 179, 19, 162, 215, 185, 58, 161,
    227, 191, 97, 160, 77, 197, 136, 69, 197, 136, 16, 54, 16, 54, 56, 157,
    101, 155, 57, 162, 146, 156, 207, 167, 191, 157, 195, 107, 69, 178, 25, 157,
    195, 183, 70, 156, 207, 188, 115, 155, 57, 193, 160, 56, 193, 160, 16, 62,
    16, 62, 50, 159, 189, 152, 225, 164, 34, 154, 119, 168, 135, 155, 107, 107,
    63, 177, 81, 155, 107, 181, 182, 154, 119, 186, 27, 152, 225, 190, 128, 50,
    190, 128, 16, 22, 82, 69, 36, 86, 54, 62, 79, 112, 74, 189, 184, 26,
    198, 80, 44, 183, 120, 92, 84, 102, 22, 82, 30, 57, 150, 93, 155, 135,
    157, 226, 157, 14, 165, 103, 157, 248, 107, 158, 70, 180, 113, 157, 248, 187,
    246, 157, 14, 195, 123, 155, 135, 203, 0, 57, 42, 59, 153, 225, 156, 188,
    160, 58, 158, 145, 166, 147, 159, 170, 107, 74, 179, 69, 159, 170, 185, 158,
    158, 145, 191, 247, 156, 188, 198, 80, 59, 46, 85, 155, 13, 162, 93, 161,
    2, 160, 246, 166, 247, 160, 30, 107, 159, 214, 178, 225, 160, 30, 184, 214,
    160, 246, 190, 203, 162, 93, 196, 192, 85, 83, 30, 105, 4, 122, 20, 102,
    47, 83, 30, 150, 218, 32, 150, 218, 55, 55, 72, 47, 100, 152, 206, 32,
    152, 206, 42, 154, 94, 32, 154, 94, 36, 62, 39, 62, 67, 67, 79, 48,
    97, 68, 17, 186, 252, 17, 76, 99, 184, 164, 99, 77, 85, 87, 71, 77,
    99, 184, 164, 99, 92, 100, 112, 100, 95, 73, 119, 73, 95, 92, 119, 92,
    97, 3, 117, 3, 97, 17, 117, 17, 102, 89, 112, 75, 102, 90, 102, 102,
    107, 128, 100, 107, 1, 107, 128, 200, 107, 16, 107, 13, 107, 99, 107, 36,
    107, 97, 107, 43, 107, 59, 107, 64, 107, 95, 107, 74, 107, 108, 107, 93,
    107, 109, 127, 85, 184, 164, 71, 184, 164, 79, 192, 16, 97, 188, 140, 0,
    95, 61, 190, 128, 39, 190, 128, 67, 191, 122, 32, 191, 122, 36, 193, 10,
    32, 193, 10, 42, 193, 60, 72, 196, 92, 100, 193, 160, 6, 108, 67, 194,
    254, 32, 194, 254, 55, 198, 80, 130, 88, 203, 0, 9, 200, 68, 5, 208,
    20, 129, 44, 206, 32, 32, 213, 240, 36,
};
static unsigned dv_coordinate_next(unsigned *offset)
{
    unsigned a=DV_XY_PACKED[(*offset)++];
    return a<128 ? (a+8)*100 : ((a&127)<<8)|DV_XY_PACKED[(*offset)++];
}
static const uint8_t DV_POLY_PACKED[] = {
    107, 200, 154, 41, 156, 84, 224, 167, 64, 78, 33, 68, 217, 232, 22, 53,
    38, 201, 20, 100, 42, 108, 83, 192, 167, 32, 82, 225, 45, 217, 241, 150,
    89, 10, 79, 21, 114, 41, 16, 85, 184, 72, 182, 190, 165, 151, 130, 79,
    5, 139, 10, 191, 200, 30, 184, 12, 83, 192, 169, 96, 81, 225, 56, 217,
    223, 150, 150, 10, 74, 21, 102, 168, 164, 178, 224, 170, 48, 86, 161, 93,
    146, 18, 147, 130, 10, 244, 20, 208, 41, 104, 84, 200, 149, 52, 50, 9,
    160, 2, 60, 5, 137, 10, 236, 20, 60, 42, 244, 137, 114, 168, 96, 77,
    161, 170, 2, 0, 68, 133, 10, 223, 20, 214, 144, 53, 83, 56, 169, 192,
    79, 129, 156, 66, 136, 178, 209, 45, 106, 76, 146, 41, 200, 84, 216, 166,
    128, 79, 65, 164, 194, 91, 178, 227, 45, 179, 20, 158, 42, 228, 82, 32,
    170, 112, 145, 108, 125, 75, 47, 5, 159, 10, 22, 21, 126, 145, 61, 112,
    25, 166, 128, 83, 193, 162, 194, 113, 178, 191, 45, 45, 21, 148, 42, 204,
    80, 73, 101, 193, 85, 97, 172, 66, 214, 152, 104, 10, 137, 76, 250, 68,
    57, 84, 176, 166, 80, 85, 33, 87, 210, 200, 36, 128, 10, 240, 20, 36,
    42, 176, 83, 240, 168, 208, 46, 73, 137, 73, 65, 5, 122, 10, 232, 20,
    52, 42, 0,
};
static unsigned dv_path(unsigned i)
{
    unsigned bit=i*17,at=bit>>3;
    unsigned word=(unsigned)DV_POLY_PACKED[at] | ((unsigned)DV_POLY_PACKED[at+1]<<8) | ((unsigned)DV_POLY_PACKED[at+2]<<16);
    return (word>>(bit&7))&131071u;
}
typedef struct { uint8_t first, npoly; } dv_body_t;
static const dv_body_t DV_BODY[2][NDRUM] = {
    {
        {0, 5},
        {5, 8},
        {13, 6},
        {19, 5},
        {24, 5},
        {29, 4},
        {33, 4},
        {37, 6},
        {43, 7},
        {50, 4},
        {54, 3},
    },
    {
        {57, 5},
        {62, 8},
        {70, 6},
        {76, 5},
        {81, 5},
        {86, 4},
        {90, 4},
        {94, 3},
        {97, 4},
        {101, 7},
        {108, 6},
    },
};

/* R22 layout (main-area y, origin at the first row under the 20 px header). */
#define DV_MAIN(y_screen) ((y_screen) - HDR_H)
static int dv_band_h(int band) { return band == 2 ? KNOB_H : BAND_H; }

static void dv_header_battery(int x, int y)
{
    int lvl = plat_battery();
    int bars, i;
    uint16_t ink = SOUND_GRAY;
    bars = lvl < 0 ? 0 : lvl > 3 ? 3 : lvl;
    if (bars < 0) bars = 0;
    /* r20 battery: 14 x 8 outline, three cells. */
    rbox(x, y, 14, 8, 1, SOUND_FACE);
    cv_rect(x, y, 14, 1, ink);
    cv_rect(x, y+7, 14, 1, ink);
    cv_rect(x, y, 1, 8, ink);
    cv_rect(x+13, y, 1, 8, ink);
    cv_rect(x+14, y+2, 2, 4, ink);
    for (i = 0; i < 3; i++) {
        uint16_t c = (i < bars) ? ink : SOUND_LINE;
        cv_rect(x+2+i*4, y+2, 3, 4, c);
    }
}

static UI_DRAW void dv_body_pictogram(int t, int v, int x, int y, float scale, float env, uint16_t col)
{
    const dv_body_t *b;
    float s, cx = x + 11.0f * 3.0f, cy = y + 6.0f * 3.0f;
    int p, i, n;
    uint16_t ink;
    if (t != T_808) t = T_909;
    if (v < 0 || v >= NDRUM) return;
    b = &DV_BODY[t == T_808][v];
    s = 3.0f * scale * (1.0f + 0.06f * env);
    ink = env > 0.02f ? blend(col, C_WHITE, (int)(env * 6.0f + 0.5f)) : col;
    for (p = 0; p < b->npoly; p++) {
        unsigned path = dv_path(b->first + p);
        unsigned offset=path&2047u;
        float xy[128];
        n = path>>11;
        if (n > 64) n = 64;
        for (i = 0; i < n; i++) {
            float px = (float)dv_coordinate_next(&offset) / 1000.0f;
            float py = (float)dv_coordinate_next(&offset) / 1000.0f;
            xy[2*i]     = (float)x + (px - 11.0f) * s + 11.0f * 3.0f;
            xy[2*i + 1] = (float)y + (py - 5.4f) * s + 5.4f * 3.0f;
            (void)cx; (void)cy;
        }
        for(int j=1;j<n;j++) cv_stroke_aa(xy[2*j-2],xy[2*j-1],xy[2*j],xy[2*j+1],3.6f*scale,ink);
    }
}

/* TUNE fork (R22 path M9 0V6Q9 11 16 11Q23 11 23 6V0M16 11V17) that bends with tune. */
static UI_DRAW void dv_tune_fork(float ox, float oy, float sx, float sy, float tune01, uint16_t col)
{
    float mid = 16.0f, half = 7.0f * (0.62f + 0.76f * tune01); /* 14-wide at default ~0.27 */
    float drop = 11.0f + 2.0f * (tune01 - 0.27f);
    float tip = 17.0f + 1.5f * (tune01 - 0.27f);
    float L = mid - half, R = mid + half;
    float xy[40];
    int n = 0;
    #define P(X,Y) do { xy[2*n]=(ox)+(X)*sx; xy[2*n+1]=(oy)+(Y)*sy; n++; } while(0)
    P(L,0);P(L,6);
    for(int i=1;i<=8;i++) {
        float t=(float)i/8.f,mt=1.f-t;
        P(mt*mt*L+2*mt*t*L+t*t*mid,mt*mt*6+2*mt*t*drop+t*t*drop);
    }
    for(int i=1;i<=8;i++) {
        float t=(float)i/8.f,mt=1.f-t;
        P(mt*mt*mid+2*mt*t*R+t*t*R,mt*mt*drop+2*mt*t*drop+t*t*6);
    }
    P(R,0);
    cv_poly_aa(xy, n, col);
    n = 0;
    P(mid, drop); P(mid, tip);
    cv_poly_aa(xy, n, col);
    #undef P
}

/* One projection for the small kick preview and its actual sounding marker.
 * Keep its 40..400 Hz domain distinct from the full-row 20..640 Hz graph. */
static UI_DRAW float dv_sweep_level(float hz)
{
    float level=fm_log2f(hz/40.f)/3.32f;
    if(level<0)level=0;if(level>1)level=1;
    return level;
}

/* Live AA glyph in an R22 slot. Box origin is the path origin (unit 0..32, 0..17). */
static UI_DRAW void dv_glyph(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col)
{
    int role = drum_strip_role(r);
    int val = (int)(ev + (ev >= 0 ? 0.5f : -0.5f));
    const x0x_param_t *d = pref_desc(r);
    float q = d && d->max ? ev / (float)d->max : 0;
    float e = 0;
    uint32_t age = 0xffffffffu;
    int live = drum_live(r, &age, &e);
    int i, n;
    float xy[80];
    if (q < 0) q = 0;
    if (q > 1) q = 1;
    if (d && name_is(d,"Pitch")) {
        /* Pitch is the 909 BD multiplier, not Tune's sweep time constant.
         * Show the actual pitched carrier's cycles over a fixed 40ms window. */
        float tau=dg_si(r,0,matrix_value(PR(R_ENG,T_909,0,0)));
        float m=.01f*dg_si(r,4,matrix_value(PR(R_ENG,T_909,0,4)));
        float pm=dg_si(r,5,val),hz=engine_bd909_hz(tau,m,pm,100.f);
        n=0;
        for(i=0;i<=32;i++) {
            float t=(float)i/32.f;
            xy[2*n]=ox+(1.f+30.f*t)*sx;
            xy[2*n+1]=oy+(6.f-5.f*fm_sinf(6.28318531f*hz*.04f*t))*sy;n++;
        }
        cv_poly_aa(xy,n,col);return;
    }
    if (role == DG_TUNE || (role == DG_NONE && d && name_is(d, "Tune"))) {
        dv_tune_fork(ox, oy, sx, sy, q, col);
        return;
    }
    if (role == DG_ATTACK) {
        /* Normalized rising-edge symbol: x encodes attack AMOUNT, not time.
         * The parameter and its percentage retain the engine's click amount.
         * Zero is vertical; larger amounts lengthen the visible rising ramp. */
        float end=1.f+30.f*q;
        n=0;
        xy[2*n]=ox+sx;xy[2*n+1]=oy+12.f*sy;n++;
        xy[2*n]=ox+end*sx;xy[2*n+1]=oy;n++;
        xy[2*n]=ox+31.f*sx;xy[2*n+1]=oy;n++;
        cv_poly_aa(xy, n, live && age < 30u ? C_WHITE : col);
        return;
    }
    if (role == DG_DECAY || role == DG_NTAIL) {
        float ms = 0, reach, hold=role==DG_NTAIL?24.f:0.f;
        int uu = engine_param_si(r.a, r.b, r.c, val, &ms);
        if (uu == X0X_U_S) ms *= 1000.0f;
        if (uu != X0X_U_MS && uu != X0X_U_S) ms = q * 4000.0f;
        /* ms is the physical fall time; hold is separate. */
        if (ms < 0) ms = 0;
        if (ms > 4000) ms = 4000;
        reach = 8.0f + 23.0f * fm_sqrtf((ms+hold) / 4000.0f); /* 8..31 */
        n = 0;
        for (i = 0; i <= 16; i++) {
            float t = (float)i / 16.0f;
            float px,py;
            if(r.a==T_909 && (uu==X0X_U_MS || uu==X0X_U_S)) {
                /* The production 909 exponential reaches 1e-5 at Decay ms.
                 * A common 2s axis makes changing decay visibly meaningful. */
                px=1.f+30.f*t;
                float elapsed=fm_maxf(0.f,2000.f*t-hold),fall=ms;
                py=12.f*(1.f-fm_exp2f(-16.60964047f*elapsed/(fall>0?fall:1.f)));
            } else {
                /* Untimed 808 controls remain raw controls, not invented ms. */
                px=1.f+t*reach;
                py=12.f*(1.f-fm_exp2f(-t*(2.2f+3.f*(1.f-q))));
                if(t*reach>reach-.01f)py=12.f;
            }
            xy[2*n] = ox + px * sx; xy[2*n+1] = oy + py * sy; n++;
        }
        float back=dg_live_back(r,live);
        uint16_t idle_col=!live && back<1.f?blend(C_LINE,col,(int)(16.f*back)):col;
        cv_poly_aa(xy, n, live?C_LINE:idle_col);
        if(live) {
            /* Same -60dB envelope/remaining-tail law as dg_tail, in the R22
             * viewbox. The approved idle curve stays as a dim ghost. */
            float level=fm_clampf(e,.001f,1.f);
            int noisy=role==DG_NTAIL;
            int xe=(noisy || uu==X0X_U_MS || uu==X0X_U_S)?dg_tail_px(ms+hold):2+(int)(q*23.f+.5f);
            float shift=(float)xe*fm_log2f(level)/-9.966f;
            uint32_t seed=noisy?age/40u:0;
            n=0;
            /* Preserve dg_tail's integer envelope/noise law and age-seeded
             * fill strokes, affinely scaled into this R22 viewbox. */
            for(i=0;(float)i<=(float)xe-shift;i++) {
                float amplitude=fm_exp2f(-9.966f*((float)i+shift)/(float)xe);
                int height=(int)(amplitude*(OG_H-2)+.5f);
                if(noisy)height=height*(40+dg_rnd((i+(int)shift)*7+(int)seed))/140;
                if(height>0) {
                    float px=ox+(1.f+(reach-1.f)*(float)i/(float)xe)*sx;
                    float py=oy+12.f*(1.f-(float)height/(OG_H-2))*sy;
                    xy[2*n]=px;xy[2*n+1]=py;n++;
                    if(i==0 || (noisy && i%5==0))
                        cv_line_aa(px,py,px,oy+12.f*sy,i?col:C_WHITE);
                }
            }
            if(n>1)cv_poly_aa(xy,n,col);
        }
        return;
    }
    if (role == DG_BDSWEEP) { /* P.DPTH / PITCH: kick pitch drop, 0..100 ms */
        int c0 = 0, c4 = 4, c5 = 5;
        float tau = dg_si(r, c0, r.c == c0 ? val : matrix_value(PR(R_ENG, T_909, 0, c0)));
        float m = .01f * dg_si(r, c4, r.c == c4 ? val : matrix_value(PR(R_ENG, T_909, 0, c4)));
        float pm = dg_si(r, c5, r.c == c5 ? val : matrix_value(PR(R_ENG, T_909, 0, c5)));
        n = 0;
        for (i = 0; i <= 24; i++) {
            float tms = (float)i * 100.0f / 24.0f;
            float hz = engine_bd909_hz(tau, m, pm, tms);
            float yv = dv_sweep_level(hz);
            xy[2*n] = ox + (31.0f * (float)i / 24.0f) * sx;
            xy[2*n+1] = oy + (12.0f * (1.0f - yv)) * sy;
            n++;
        }
        cv_poly_aa(xy, n, col);
        /* The sounding kick latches its own sweep; a next-note control edit
         * must not move this marker onto the newly requested preview. */
        float ms,hz;
        if(engine_bd909_pitch_view(&ms,&hz) && ms>=0 && ms<=100) {
            float level=dv_sweep_level(hz);
            int mx=(int)(ox+31.f*ms/100.f*sx+.5f);
            int my=(int)(oy+12.f*(1.f-level)*sy+.5f);
            cv_rect(mx-1,my-1,3,3,C_WHITE);
        }
        return;
    }
    if (role == DG_LPF) {
        /* Actual one-pole 808 BD tone response; logarithmic frequency axis.
         * bd_trigger sets c=1-exp(-2*pi*fc/sr), fc=300*20^(pot/127). */
        float hz=300.f*fm_exp2f(q*4.32192809f);
        float c=1.f-fm_exp2f(-6.28318531f*hz/44100.f*1.44269504f),a=1.f-c;
        n=0;
        for(i=0;i<=24;i++) {
            float t=(float)i/24.f,freq=100.f*fm_exp2f(t*7.78463485f);
            float den=1.f+a*a-2.f*a*fm_cosf(6.28318531f*freq/44100.f);
            float mag=den>0?c/fm_sqrtf(den):1.f;
            if(mag>1)mag=1;
            if(mag<0)mag=0;
            xy[2*n]=ox+31.f*t*sx;xy[2*n+1]=oy+(1.f+11.f*(1.f-mag))*sy;n++;
        }
        cv_poly_aa(xy, n, col);
        return;
    }
    if (role == DG_ACCENT) {
        /* four ticks, height follows accent */
        float hmax = 4.0f + 8.0f * q;
        for (i = 0; i < 4; i++) {
            float hx = 2.0f + i * 8.0f;
            float hh = hmax * (0.25f + 0.25f * (float)i);
            cv_line_aa(ox + hx * sx, oy + 12 * sy, ox + hx * sx, oy + (12.0f - hh) * sy, col);
        }
        return;
    }
    if (role == DG_SNAPPY || role == DG_SHELLS) {
        /* a live burst / two shells, scaled into the slot */
        float k = live ? (0.35f + 0.65f * e) : 1.0f;
        if (role == DG_SHELLS) {
            float tp = ev, ang = (tp <= 64 ? ev / 128.0f : 0.5f + (ev - 64.0f) * (0.5f / 63.0f)) * 1.5708f;
            float g1 = fm_cosf(ang) * 12.0f * k, g2 = fm_sinf(ang) * 12.0f * k;
            cv_line_aa(ox + 7 * sx, oy + 12 * sy, ox + 11 * sx, oy + (12 - g1) * sy, col);
            cv_line_aa(ox + 11 * sx, oy + (12 - g1) * sy, ox + 15 * sx, oy + 12 * sy, col);
            cv_line_aa(ox + 17 * sx, oy + 12 * sy, ox + 21 * sx, oy + (12 - g2) * sy, col);
            cv_line_aa(ox + 21 * sx, oy + (12 - g2) * sy, ox + 25 * sx, oy + 12 * sy, col);
        } else {
            n = 0;
            for (i = 0; i <= 16; i++) {
                float t = (float)i / 16.0f;
                float envp = fm_exp2f(-t * 3.5f);
                float h = q * k * envp * 12.0f;
                xy[2*n] = ox + (1 + t * 30) * sx;
                xy[2*n+1] = oy + (12.0f - h) * sy;
                n++;
            }
            cv_poly_aa(xy, n, col);
        }
        return;
    }
    /* fallback: a value-scaled arc */
    n = 0;
    for (i = 0; i <= 12; i++) {
        float t = (float)i / 12.0f;
        xy[2*n] = ox + (1 + t * 30) * sx;
        xy[2*n+1] = oy + (12.0f - 12.0f * q * fm_sinf(t * 3.1416f)) * sy;
        n++;
    }
    cv_poly_aa(xy, n, col);
}

static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large)
{
    char num[16], unit[8], t[24];
    const felucca_font_t *f = large ? &FONT_S : &FONT_T;
    pref_value_of(r, ev_i, num, unit);
    t[0] = 0;
    put_s(put_s(t, num), unit);
    text_c(cx, y, f, t, col);
}

/* Format helpers already exist; text_c used above. */

/* Fixed 20..640 Hz and 160 ms axes cover the whole 909 kick sweep. The
 * line previews the current controls. The marker follows the sounding
 * oscillator, including after controls change between two hits. */
static UI_DRAW int dv_kick_pitch_y(float hz,int top,int bottom)
{
    float octaves=fm_clampf(fm_log2f(hz/20.f),0.f,5.f);
    return bottom-(int)((bottom-top)*octaves*.2f+.5f);
}

static UI_DRAW int dv_909_curve(int voice,drum_curve_t *curve)
{
    engine_decay_view_t v;
    if(!engine_drum909_decay_view(voice,&v))return 0;
    int pot=voice<=DR_HT?1:voice==DR_CH || voice==DR_OH?0:2;
    float horizon;
    if(!drum909_param_si(voice,pot,127,&horizon))return 0;
    *curve=(drum_curve_t){.peak=v.peak,.window=horizon,.elapsed=v.elapsed_ms,.level=v.level};
    if(voice==DR_SD){curve->hold=24.f;curve->window=24.f+horizon;}
    curve->fall=v.decay_ms/fm_logf(fm_maxf(1.001f,v.peak/.00001f));
    return 1;
}

static UI_DRAW float dv_curve_level(const drum_curve_t *v,float ms)
{
    float t=ms-v->delay;
    if(t<0)return 0;
    if(t<v->rise_end)return v->peak*(1.f-fm_expf(-t/v->rise_tau));
    float peak=v->rise_end>0?v->peak*(1.f-fm_expf(-v->rise_end/v->rise_tau)):v->peak;
    t=fm_maxf(0.f,t-v->rise_end-v->hold);
    return v->linear?fm_maxf(0.f,peak-v->peak*t/v->fall):peak*fm_expf(-t/v->fall);
}
static UI_DRAW void dv_curve_row(drum_curve_t *v,int kick,int top,int bottom,uint16_t col)
{
    if(bottom<cv_y0 || top>=cv_y1 || top>=bottom)return;
    float controls[3],elapsed=v->elapsed,level=v->level;
    if(kick) {
        static const uint8_t pots[3]={0,4,5};
        for(int i=0;i<3;i++){
            pref_t r=PR(R_ENG,T_909,DR_BD,pots[i]);
            controls[i]=dg_si(r,pots[i],matrix_value(r));
        }
        if(!engine_bd909_pitch_view(&elapsed,&level))elapsed=-1;
    }
    int left=13,width=214;
    if(bottom-top>=36) {
        /* A physical time/level scale explains natural empty plot regions.
         * The plotted window is not the selected decay control. Dense rows
         * keep their existing plot instead of squeezing in unreadable axes. */
        int end=bottom,mid;
        char time[16];
        left=33;width=194;bottom-=FONT_T.h+2;mid=(top+bottom)/2;
        cv_line(left,mid,left+width,mid,SOUND_LINE);
        cv_line(left+width/2,top,left+width/2,bottom,SOUND_LINE);
        text_r(29,top-2,&FONT_T,kick?"640Hz":"100%",SOUND_GRAY);
        text_r(29,mid-3,&FONT_T,kick?"113":"50%",SOUND_GRAY);
        text_r(29,bottom-4,&FONT_T,kick?"20":"0%",SOUND_GRAY);
        cv_text(left,end-FONT_T.h,&FONT_T,"0",SOUND_GRAY);
        put_i(time,(int)(v->window*.5f+.5f));
        text_c(left+width/2,end-FONT_T.h,&FONT_T,time,SOUND_GRAY);
        put_s(put_i(time,(int)(v->window+.5f)),"ms");
        text_r(left+width,end-FONT_T.h,&FONT_T,time,SOUND_GRAY);
    }
    float points[72];
    float onset[3]={v->delay,v->delay+v->rise_end,v->delay+v->rise_end+v->hold};
    int used=0,stage=0,i=0;
    while(i<=32) {
        float t=v->window*i/32.f;
        if(stage<3 && onset[stage]<t)t=onset[stage++];else i++;
        points[used++]=(float)left+(float)width*t/v->window;
        if(kick){
            float hz=engine_bd909_hz(controls[0],controls[1]*.01f,controls[2],t);
            points[used++]=dv_kick_pitch_y(hz,top,bottom);
        } else points[used++]=bottom-(bottom-top)*fm_minf(1.f,dv_curve_level(v,t));
    }
    cv_poly_aa(points,used/2,col);
    if(elapsed>=0 && elapsed<=v->window){
        int x=left+(int)((float)width*elapsed/v->window+.5f);
        int y=kick?dv_kick_pitch_y(level,top,bottom):
            bottom-(int)((bottom-top)*fm_clampf(level,0,1)+.5f);
        cv_rect(x-1,y-1,3,3,SOUND_INK);
    }
}

/* A bounded stage curve is a target-law preview. The separate right-hand
 * level tick is actual owner state, not reconstructed horizontal progress. */
static UI_DRAW void dv_bound_stage(drum_curve_t *v,int stage,int top,int off,uint16_t col)
{
    if(!v || !(v->peak>=0 && v->peak<1.e30f) ||
       !(v->rise_tau>0 && v->rise_tau<1.e30f) ||
       !(v->rise_end>0 && v->rise_end<1.e30f) ||
       !(v->fall>0 && v->fall<1.e30f) ||
       !(v->window>0 && v->window<1.e30f))return;
    float points[68],window=stage==1?90.f:stage==2?650.f:v->window;
    float end=stage==1?fm_minf(window,v->rise_end):window;
    int height=stage==0?48:20,bottom=top+height;
    int used=0,knot=stage!=0;
    for(int i=0;i<=32;) {
        float ms=end*i/32.f;
        if(!knot && v->rise_end<ms){ms=v->rise_end;knot=1;}else i++;
        float level=stage==1?1.f-fm_expf(-ms/v->rise_tau):
                    stage==2?dv_curve_level(v,v->delay+v->rise_end+v->hold+ms):dv_curve_level(v,ms);
        points[2*used]=13.f+214.f*ms/window;
        points[2*used+1]=bottom-off-height*fm_clampf(level,0,1);used++;
    }
    cv_poly_aa(points,used,col);
    char label[16];put_s(put_i(label,(int)window),"ms");
    cv_text(13,bottom-off,&FONT_T,"0",SOUND_GRAY);
    text_r(227,bottom-off,&FONT_T,label,SOUND_GRAY);
    if(v->elapsed>=0 && v->level>=0 && v->level<1.e30f) {
        int y=bottom-off-(int)(height*fm_clampf(v->level,0,1)+.5f);
        cv_rect(230,y,3,1,SOUND_INK);
    }
}
/* Shared identity and white-key window. Preserve selected-owner bounds and
 * the legacy generic window fallback; neither changes sequencer state. */
static UI_DRAW void dv_voice_steps(int t,int v,int identity_y,int step_y,int strict)
{
    uint16_t col=part_col();
    const dpart_t *dp=&cur_pat()->drum[ui.part];
    int len=dp->len,shown_len=len;
    unsigned first=(unsigned)ui.spage*16u;
    if(!strict){if(first>=NSTEPS)first=0;if(shown_len<1)shown_len=16;}
    char label[32];
    put_i(put_s(put_s(label,dv_voice_name(t,v)),t==T_808?" 808/":" 909/"),v+1);
    cv_text(11,identity_y,&FONT_T,label,col);
    put_i(put_s(put_i(put_s(label,"P"),seq.ppat[ui.part]+1)," / "),shown_len);
    text_r(226,identity_y,&FONT_T,label,SOUND_GRAY);
    for(int s=0;s<16;s++) {
        int absolute=(int)first+s,sx=9+s*14;
        int valid=absolute<NSTEPS && absolute<len;
        int hit=valid && sm_get(dp->hit[v],absolute);
        int live=seq.playing && eng_step[TRK_DRUM+ui.part]%(len?len:16)==absolute;
        if(strict&&!valid)live=0;
        cv_rect(sx,step_y,11,4,live?C_WHITE:hit?col:SOUND_LINE);
        put_i(label,absolute+1);text_c(sx+5,step_y+6,&FONT_T,label,SOUND_GRAY);
    }
}

/* Nominal target 99% stage duration, never an observed transition time. */
static UI_DRAW void dv_nominal_stage_time(float ms,int y)
{
    char value[32];put_s(put_d1(put_s(value,"99% "),ms,0),"ms");
    cv_text(112,y,&FONT_T,value,SOUND_GRAY);
}

static UI_DRAW void dv_cp_stage_page(int band)
{
    int off=HDR_H+band*BAND_H,page=cur_page();
    uint16_t col=part_col(),ink=page==0?SOUND_INK:SOUND_GRAY;
    drum_curve_t attack={.elapsed=-1},decay={.elapsed=-1};
    int choice=matrix_value(pg.r[0][2]);
    int attack_owner=engine_drum808_param_curve_preview(D8_CP,3,choice,matrix_value(pg.r[0][1]),&attack);
    int decay_owner=engine_drum808_param_curve_preview(D8_CP,2,choice,matrix_value(pg.r[0][3]),&decay);
    int selected=decay_owner==D8S_MA+1;
    rbox(3,24-off,234,213,UI_PANEL_RADIUS,SOUND_FACE);
    cv_text(11,28-off,&FONT_T,"SOUND",ink);
    if(!key_sounds())cv_text(91,28-off,&FONT_T,"KEY SOUND OFF",SOUND_GRAY);
    char label[32];put_i(put_s(put_i(label,page+1),"/"),pg.n);
    text_r(232,28-off,&FONT_T,label,SOUND_GRAY);
    const int slots[2]={0,2};
    for(int i=0;i<2;i++) {
        pref_t r=pg.r[0][slots[i]];int center=i?177:63,value=matrix_value(r);
        text_c(center,40-off,&FONT_T,i?"K7 SOUND":"K5 TUNE",SOUND_GRAY);
        dv_glyph(r,ui_ease_float(r,value),center-22,53-off,1.375f,1.f,col);
        dv_value_text(r,value,center,72-off,SOUND_WHITE,1);
    }
    cv_text(13,88-off,&FONT_T,selected?"MARACAS AD":"MARACAS ATTACK",SOUND_GRAY);
    physical_stage_label(label,1,'A',matrix_value(pg.r[0][1]));
    if(selected)cv_text(13,100-off,&FONT_T,label,ink);
    else text_r(227,88-off,&FONT_T,label,ink);
    /* Physical nominal 99% rise readout, distinct from the fixed view window. */
    if(attack_owner==D8S_MA+1)dv_nominal_stage_time(attack.rise_end,88-off);
    if(selected) {
        /* Both editable stages belong to MA: one AD graph, never two owners. */
        physical_stage_label(label,3,'D',matrix_value(pg.r[0][3]));
        text_r(227,100-off,&FONT_T,label,ink);
        if(attack_owner==D8S_MA+1) {
            decay.rise_tau=attack.rise_tau;decay.rise_end=attack.rise_end;
            if(attack.elapsed>=0){decay.elapsed=attack.elapsed;decay.level=attack.level;}
        }
        dv_bound_stage(&decay,0,116,off,col);
    } else {
        if(attack_owner==D8S_MA+1)dv_bound_stage(&attack,1,100,off,col);
        cv_text(13,132-off,&FONT_T,"CLAP DECAY",SOUND_GRAY);
        if(decay_owner==D8S_CP+1)dv_nominal_stage_time(decay.fall*4.60517f,132-off);
        physical_stage_label(label,3,'D',matrix_value(pg.r[0][3]));
        text_r(227,132-off,&FONT_T,label,ink);
        if(decay_owner==D8S_CP+1)dv_bound_stage(&decay,2,144,off,col);
    }
    pref_t kit=pg.r[1][0];int kv=matrix_value(kit);
    cv_text(11,180-off,&FONT_T,"KIT ACCENT",page==1?SOUND_INK:SOUND_GRAY);
    dv_glyph(kit,ui_ease_float(kit,kv),105,179-off,.7f,.7f,col);
    dv_value_text(kit,kv,214,180-off,SOUND_WHITE,0);
    dv_voice_steps(T_808,D8_CP,200-off,212-off,1);
}

static UI_DRAW void dv_control_caption(pref_t r,int cx,int y)
{
    const x0x_param_t *d=pref_desc(r);char caption[16];int j=0;
    if(d&&d->name)for(;d->name[j]&&j<14;j++)caption[j]=d->name[j]>='a'&&d->name[j]<='z'?d->name[j]-32:d->name[j];
    caption[j]=0;
    if(d&&name_is(d,"P.Dpth"))put_s(caption,"DEPTH");
    text_c(cx,y,&FONT_T,caption,SOUND_GRAY);
}

static UI_DRAW void draw_drum_voice(int band)
{
    int off = band * BAND_H, bh = dv_band_h(band);
    int t = ui.part == PART_808 ? T_808 : T_909;
    int v = ui.sel[ui.part];
    int page = cur_page();
    int n = pg.n;
    int p, y, active;
    uint16_t col = part_col();
    uint16_t ink = col, gray = SOUND_GRAY, dimc = SOUND_DIM;
    uint32_t age = 0;
    float env = 0;
    int live;
    int body_y, yb;
    cv_rect(0, 0, 240, bh, SOUND_BASE);
    if(t==T_808 && v==D8_CP){dv_cp_stage_page(band);return;}

    /* body surface + banks: R22 y=24 (screen) => main y=4, height grows with bank count. */
    yb = DV_MAIN(24);
    /* Keep the lower voice/step panel compact. Spare space belongs to the
     * active controls, not an empty box between the pictogram and steps. */
    {
        int by = yb - off;
        int body_h;
        int used = 0;
        int ah;
        for (p = 0; p < n; p++) {
            int cells = 0, k;
            for (k = 0; k < 4; k++) if (pg.r[p][k].kind != R_NONE) cells++;
            ah = (p == page) ? (cells==1?56:72) : (cells>1?42:32);
            used += ah;
        }
        body_h = used + 4;
        if (by + body_h > 0 && by < bh)
            rbox(3, by, 234, body_h, UI_PANEL_RADIUS, SOUND_FACE);
        /* banks */
        y = yb + 2;
        for (p = 0; p < n; p++) {
            int cells = 0, k, cx0;
            for (k = 0; k < 4; k++) if (pg.r[p][k].kind != R_NONE) cells++;
            if (cells < 1) cells = 1;
            active = (p == page);
            ah = active ? (cells==1?56:72) : (cells>1?42:32);
            if (active) {
                int ay = y - off;
                if (ay + ah > 0 && ay < bh)
                    rbox(6, ay, 228, ah, UI_PANEL_RADIUS, ui_theme_light?SOUND_SELECT:RGB(36,48,54));
            }
            if(adsr_row(p)) {
                cv_text(11,y-off,&FONT_T,"AMP",gray);
                draw_adsr_inline(p,active,y,ah,off);y+=ah;continue;
            }
            /* labels */
            {
                                int ly = y + 2 - off;
                char pgind[8], a[16], b[16];
                const x0x_param_t *pd0;
                a[0] = b[0] = 0;
                if (pg.r[p][0].kind == R_ENG && pg.r[p][0].b == NDRUM) {
                    put_s(a, "KIT"); put_s(b, pref_name(pg.r[p][0]));
                } else {
                    put_s(a, "SOUND");
                    pd0 = pref_desc(pg.r[p][0]);
                    /* Multi-control rows already label each cell; a second
                     * first-control summary would overprint that caption. */
                    if (cells==1 && pd0 && pd0->name) {
                        const char *nm = pd0->name;
                        if (name_is(pd0, "Pitch"))put_s(b,"PITCH");
                        else if(name_is(pd0,"P.Dpth"))put_s(b,"DEPTH");
                        else {
                            int i; char *d = b;
                            for (i = 0; nm[i] && i < 12; i++) {
                                char c = nm[i];
                                if (c >= 'a' && c <= 'z') c = (char)(c - 32);
                                *d++ = c;
                            }
                            *d = 0;
                        }
                    }
                }
                if (ly + 12 > 0 && ly < bh) {
                    cv_text(11, ly, &FONT_T, a, gray);
                    if(p==0 && !key_sounds())cv_text(91,ly,&FONT_T,"KEY SOUND OFF",gray);
                    put_s(put_i(put_s(put_i(pgind, p + 1), "/"), pg.n), "");
                    text_r(232, ly, &FONT_T, pgind, gray);
                }
                /* The second line may enter this strip after the first line
                 * has left it. Test its own bounds, not the first line's. */
                if (b[0] && ly + 24 > 0 && ly + 12 < bh)
                    cv_text(11, ly + 12, &FONT_T, b, gray);
            }
            /* glyphs + values */
            cx0 = 11;
            for (k = 0; k < 4; k++) {
                pref_t r = pg.r[p][k];
                int ev_i, vx, gy, gx;
                float ev;
                if (r.kind == R_NONE) continue;
                ev_i = matrix_value(r);
                ev = ui_ease_float(r,ev_i);
                /* Only the pictogram eases; numeric state is committed now. */
                if (active && cells >= 2) {
                    vx=6+(228*(2*k+1))/(2*cells);
                    gx=vx-22;
                    gy=y+28;
                    dv_control_caption(r,vx,y+14-off);
                    float glyph_sy=1.50f+(float)(ah-72)*.025f;
                    if (gy-off < bh && (float)(gy-off)+18.f*glyph_sy+2.f > 0)
                        dv_glyph(r, ev, (float)gx, (float)(gy-off),1.375f,glyph_sy,ink);
                    if (y + ah - 16 - off < bh && y + ah - 16 - off + 16 > 0)
                        dv_value_text(r, ev_i, vx, y + ah - 16 - off, SOUND_WHITE, 1);
                } else if (active) {
                    /* single-control active bank (e.g. rimshot Tune): centered large glyph */
                    gx = 92; gy = y + 23; /* Keep the scaled fork inside the56px active bank. */
                    if (gy - off < bh)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 1.8f, 1.8f, ink);
                    dv_value_text(r, ev_i, 214, y + 35 - off, SOUND_WHITE, 1);
                } else if(cells>1) {
                    /* Each real control retains its own labeled compact cell.
                     * A shared centre/right position would overprint siblings. */
                    gx=6+(228*(2*k+1))/(2*cells)-22;gy=y+29;
                    dv_control_caption(r,gx+22,y+14-off);
                    if(gy-off<bh&&gy+17-off>0)dv_glyph(r,ev,(float)gx,(float)(gy-off),.62f,.76f,gray);
                    dv_value_text(r,ev_i,gx+40,y+29-off,SOUND_WHITE,0);
                } else {
                    /* A single-control summary has one glyph and one value. */
                    gx = 98; gy = y + 5;
                    if (gy - off < bh && gy + 20 - off > 0)
                        dv_glyph(r, ev, (float)gx, (float)(gy - off), 1.3f, 1.5f, gray);
                    dv_value_text(r, ev_i, 214, y + 15 - off, SOUND_WHITE, 1);
                }
                (void)cx0;
            }
            y += ah;
        }
        body_y = yb + body_h + 4; /* gap before the lower panel */
    }

    /* lower panel: body pictogram, voice name, P1/16, 16-step strip */
    live = engine_drum_view(ui.part, v, &age, &env);
    if (!live) env = 0;
    {
        int py = body_y - off;
        int ph = 237 - HDR_H - body_y; /* to screen y=237 */
        if (ph < 40) ph = 40;
        if (py + ph > 0 && py < bh) {
            rbox(3, py, 234, ph, UI_PANEL_RADIUS, SOUND_FACE);
            /* Real voice identity and actual16 steps share the remaining
             * context surface. Rows reflow; no activity or data is invented. */
            /* One continuous16-step window, matching the white-key input.
             * Keep the voice identity above it, never over the step lane. */
            int step_top=body_y+ph-25;
            /* Authored body paths plus the live 6% pulse fit in 44px per
             * scale unit. Leave the identity and step lane unobstructed. */
            float body_scale=(float)(ph-44)/44.f;
            if(body_scale>3.f)body_scale=3.f;
            drum_curve_t curve={.window=160};
            int kick=t==T_909 && v==DR_BD;
            int graph=kick || (t==T_808?engine_drum808_curve(v,&curve):dv_909_curve(v,&curve));
            if(graph) {
                /* Keep the existing physical knobs, voice and all 16 steps.
                 * The editable envelope uses the full width between them. */
                dv_body_pictogram(t,v,79,body_y-9-off,.32f,env,col);
                const char *role="AMP ENV";
                if(kick)role="PITCH ENV";
                else if((t==T_909 && v==DR_SD) || (t==T_808 && v==D8_SD))role="NOISE ENV";
                else if((t==T_909 && v==DR_CP) || (t==T_808 && v==D8_CP && curve.delay>0))role="TAIL ENV";
                else if(t==T_808 && v==D8_CB)role="BODY ENV";
                else if(t==T_808 && v==D8_CY)role="SUSTAIN";
                cv_text(133,body_y+3-off,&FONT_T,role,gray);
                dv_curve_row(&curve,kick,body_y+17-off,step_top-4-off,col);
            } else if(body_scale>0.f)
                dv_body_pictogram(t,v,87,body_y+(ph-11)/2-16-off,body_scale,env,col);
            dv_voice_steps(t,v,body_y+3-off,step_top-off,0);
        }
    }
}


