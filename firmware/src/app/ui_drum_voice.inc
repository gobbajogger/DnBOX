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

/* Exact authored millipixel coordinates. Identical point runs share an offset;
 * DV_POLY preserves the original boundaries, direction and painting order. */
/* Lossless 15-bit coordinates; trailing zero octets bound the final decode. */
static const uint8_t DV_XY_PACKED[] = {
    196, 9, 240, 74, 163, 67, 152, 34, 47, 113, 156, 204, 91, 138, 36, 144,
    154, 114, 72, 114, 7, 238, 225, 9, 34, 111, 60, 140, 134, 24, 28, 37,
    70, 5, 196, 73, 43, 65, 144, 250, 67, 224, 171, 248, 15, 236, 44, 232,
    3, 184, 139, 255, 64, 13, 251, 67, 32, 203, 180, 18, 188, 52, 70, 197,
    183, 109, 136, 1, 150, 35, 111, 248, 240, 224, 30, 122, 191, 114, 8, 197,
    176, 72, 194, 112, 116, 156, 132, 45, 133, 41, 20, 80, 240, 138, 210, 145,
    19, 3, 232, 131, 212, 216, 214, 120, 55, 236, 44, 16, 142, 8, 137, 119,
    131, 181, 129, 212, 56, 74, 56, 49, 196, 9, 240, 10, 113, 2, 188, 130,
    194, 195, 168, 72, 238, 22, 51, 219, 57, 216, 206, 199, 237, 39, 4, 57,
    219, 31, 173, 189, 100, 75, 248, 42, 36, 83, 161, 73, 182, 4, 38, 218,
    31, 69, 123, 126, 66, 21, 28, 216, 142, 151, 102, 49, 131, 156, 193, 168,
    120, 105, 74, 33, 21, 28, 64, 70, 180, 39, 30, 1, 38, 170, 49, 20,
    154, 252, 8, 248, 42, 244, 193, 218, 203, 143, 0, 57, 171, 49, 124, 220,
    226, 17, 219, 57, 64, 134, 228, 174, 20, 130, 194, 195, 168, 0, 250, 192,
    18, 231, 61, 127, 69, 9, 15, 144, 161, 149, 91, 110, 216, 214, 148, 29,
    121, 177, 180, 7, 59, 11, 244, 241, 133, 74, 123, 136, 144, 148, 29, 126,
    160, 229, 198, 108, 7, 144, 17, 191, 241, 87, 96, 109, 192, 18, 241, 27,
    225, 195, 108, 7, 200, 224, 7, 170, 39, 136, 144, 236, 7, 95, 168, 171,
    1, 59, 11, 100, 144, 23, 187, 26, 216, 214, 236, 7, 90, 185, 122, 66,
    9, 15, 200, 112, 222, 19, 62, 0, 250, 192, 18, 160, 65, 176, 196, 58,
    208, 95, 161, 237, 3, 100, 64, 238, 150, 27, 70, 55, 101, 7, 146, 44,
    237, 193, 206, 2, 125, 64, 158, 210, 30, 146, 34, 101, 7, 146, 103, 185,
    225, 175, 1, 100, 180, 99, 252, 21, 56, 24, 176, 68, 59, 70, 248, 224,
    175, 1, 50, 32, 121, 234, 9, 146, 34, 251, 1, 228, 233, 106, 192, 206,
    2, 25, 32, 201, 174, 6, 70, 55, 251, 1, 228, 174, 158, 160, 237, 3,
    50, 172, 3, 133, 15, 160, 65, 176, 4, 98, 17, 44, 17, 75, 244, 87,
    240, 8, 1, 25, 83, 190, 229, 134, 78, 78, 217, 177, 52, 75, 123, 176,
    179, 64, 31, 141, 166, 180, 135, 39, 72, 217, 81, 184, 89, 110, 112, 94,
    0, 25, 39, 21, 127, 5, 20, 5, 44, 113, 82, 17, 62, 112, 94, 128,
    12, 133, 155, 122, 130, 39, 200, 126, 208, 104, 186, 26, 176, 179, 64, 6,
    75, 179, 171, 129, 78, 206, 126, 48, 229, 171, 39, 240, 8, 129, 12, 177,
    68, 225, 3, 98, 17, 44, 1, 101, 196, 218, 132, 21, 241, 58, 166, 66,
    185, 143, 147, 175, 36, 68, 138, 155, 19, 49, 200, 136, 70, 248, 42, 198,
    17, 249, 136, 104, 196, 212, 153, 19, 137, 94, 74, 66, 74, 19, 185, 207,
    35, 4, 175, 3, 250, 192, 218, 60, 66, 112, 50, 74, 19, 159, 139, 232,
    101, 177, 194, 212, 233, 161, 144, 143, 216, 38, 248, 42, 146, 9, 131, 140,
    109, 66, 138, 235, 161, 56, 249, 22, 43, 166, 66, 159, 75, 88, 17, 39,
    3, 101, 196, 218, 16, 33, 113, 23, 85, 199, 177, 134, 38, 49, 222, 33,
    4, 60, 130, 96, 234, 154, 34, 0, 52, 249, 8, 59, 75, 69, 130, 93,
    146, 143, 0, 125, 154, 34, 150, 153, 35, 136, 79, 37, 222, 49, 40, 25,
    107, 80, 70, 112, 23, 131, 146, 6, 133, 79, 229, 15, 97, 153, 73, 57,
    0, 125, 70, 12, 216, 37, 191, 2, 59, 203, 168, 0, 64, 243, 43, 96,
    234, 70, 12, 66, 192, 148, 131, 38, 241, 15, 81, 117, 108, 80, 16, 33,
    113, 23, 208, 7, 40, 10, 202, 136, 156, 193, 81, 42, 87, 64, 156, 204,
    18, 4, 41, 52, 4, 190, 42, 253, 192, 206, 90, 63, 128, 187, 216, 16,
    212, 48, 183, 4, 178, 12, 94, 129, 32, 165, 115, 216, 41, 107, 35, 104,
    194, 30, 10, 119, 174, 193, 66, 13, 43, 185, 56, 157, 8, 47, 76, 157,
    149, 11, 179, 68, 195, 2, 125, 128, 162, 64, 31, 160, 40, 172, 13, 8,
    7, 95, 165, 84, 193, 212, 161, 65, 112, 148, 122, 13, 236, 44, 32, 3,
    47, 173, 215, 192, 200, 163, 65, 112, 17, 75, 21, 44, 76, 8, 7, 23,
    241, 11, 194, 200, 35, 144, 240, 210, 4, 38, 236, 44, 171, 9, 71, 73,
    96, 194, 212, 33, 144, 240, 85, 190, 32, 172, 13, 8, 7, 107, 3, 194,
    193, 218, 192, 168, 240, 85, 46, 35, 76, 157, 139, 7, 71, 201, 178, 193,
    206, 178, 104, 240, 210, 44, 27, 140, 188, 139, 7, 23, 241, 50, 194, 194,
    196, 168, 112, 17, 163, 46, 140, 188, 115, 12, 47, 109, 59, 195, 206, 98,
    209, 112, 148, 182, 51, 76, 157, 115, 12, 95, 37, 234, 194, 218, 192, 168,
    176, 54, 48, 42, 172, 13, 54, 16, 95, 165, 195, 195, 212, 185, 227, 112,
    148, 244, 54, 236, 44, 147, 13, 47, 77, 111, 195, 200, 187, 227, 112, 17,
    59, 60, 44, 76, 54, 16, 23, 145, 95, 196, 200, 139, 38, 241, 210, 212,
    75, 236, 44, 36, 19, 71, 73, 189, 196, 212, 137, 38, 241, 85, 248, 69,
    172, 13, 54, 16, 107, 131, 13, 4, 13, 66, 31, 144, 226, 208, 7, 68,
    72, 42, 146, 91, 112, 212, 132, 169, 67, 66, 9, 207, 140, 82, 236, 44,
    205, 148, 133, 201, 40, 5, 244, 65, 66, 169, 97, 70, 77, 148, 17, 42,
    18, 52, 8, 125, 0, 250, 192, 218, 128, 62, 80, 70, 118, 150, 165, 18,
    83, 71, 218, 36, 66, 226, 61, 225, 171, 20, 80, 206, 49, 222, 19, 41,
    78, 218, 164, 247, 91, 42, 65, 25, 81, 70, 80, 70, 172, 13, 101, 4,
    119, 65, 25, 161, 240, 168, 97, 204, 65, 64, 31, 117, 145, 133, 233, 131,
    196, 206, 50, 36, 9, 207, 62, 72, 152, 58, 117, 145, 91, 208, 28, 68,
    132, 164, 240, 16, 33, 113, 23, 80, 20, 176, 4, 20, 133, 194, 115, 167,
    105, 2, 121, 130, 198, 67, 197, 166, 107, 17, 59, 11, 101, 52, 49, 187,
    22, 233, 228, 198, 67, 97, 191, 38, 16, 98, 145, 194, 131, 88, 4, 75,
    224, 96, 192, 18, 56, 24, 128, 76, 89, 39, 103, 35, 41, 122, 230, 252,
    158, 134, 59, 236, 44, 10, 79, 134, 108, 184, 99, 116, 123, 230, 204, 241,
    114, 54, 160, 65, 128, 12, 104, 16, 44, 129, 181, 1, 75, 96, 109, 80,
    45, 189, 159, 112, 140, 8, 233, 78, 115, 136, 90, 219, 176, 179, 120, 55,
    81, 177, 181, 141, 109, 237, 78, 179, 161, 11, 199, 0, 250, 80, 45, 128,
    62, 176, 4, 238, 2, 101, 68, 225, 129, 137, 224, 146, 112, 48, 88, 27,
    254, 16, 184, 11, 1, 132, 219, 67, 106, 64, 25, 161, 40, 120, 55, 136,
    19, 252, 8, 95, 133, 187, 64, 25, 97, 59, 200, 50, 93, 150, 195, 141,
    120, 199, 161, 115, 86, 194, 239, 176, 179, 140, 60, 113, 52, 252, 142, 253,
    206, 161, 179, 55, 60, 220, 0, 44, 201, 50, 136, 19, 22, 77, 120, 134,
    151, 163, 3, 138, 244, 76, 154, 84, 63, 236, 44, 4, 80, 209, 76, 245,
    227, 153, 139, 244, 220, 255, 120, 57, 80, 70, 22, 13, 70, 133, 138, 212,
    176, 233, 18, 9, 132, 236, 65, 247, 38, 15, 16, 59, 203, 250, 19, 46,
    243, 0, 89, 227, 236, 65, 203, 190, 46, 17, 48, 145, 138, 196, 56, 194,
    118, 144, 176, 96, 9, 200, 50, 120, 5, 190, 138, 175, 194, 56, 194, 118,
    104, 91, 64, 31, 218, 22, 78, 12, 39, 6, 232, 195, 87, 129, 81, 57,
    99, 64, 31, 206, 24, 196, 137, 151, 6, 244, 225, 165, 129, 137, 96, 109,
    184, 36, 88, 27, 166, 14, 83, 135, 63, 4, 94, 33, 72, 193, 118, 136,
    19, 252, 58, 226, 4, 52, 136, 57, 69, 138, 99, 78, 209, 132, 168, 72,
    28, 37, 110, 15, 77, 136, 57, 69, 138, 99, 78, 65, 156, 96, 84, 224,
    46, 24, 21, 15, 138, 244, 195, 25, 35, 253, 240, 160, 32, 78, 156, 49,
    136, 19, 65, 138, 137, 64, 13, 99, 34, 16, 164, 136, 19, 212, 48, 226,
    4, 190, 138, 188, 4, 238, 98, 3, 225, 171, 144, 76, 248, 42, 124, 21,
    59, 139, 12, 192, 206, 34, 28, 176, 179, 144, 1, 236, 44, 176, 4, 59,
    139, 6, 193, 206, 98, 78, 177, 179, 96, 34, 236, 44, 130, 20, 59, 139,
    125, 194, 206, 98, 209, 176, 179, 64, 56, 236, 44, 30, 20, 59, 11, 1,
    196, 206, 130, 106, 177, 179, 232, 78, 236, 44, 218, 22, 47, 141, 138, 68,
    138, 227, 246, 144, 226, 248, 67, 16, 64, 130, 20, 35, 15, 100, 192, 131,
    162, 215, 0, 250, 184, 36, 128, 62, 166, 142, 222, 15, 244, 161, 247, 131,
    137, 40, 4, 65, 31, 10, 65, 196, 9, 79, 16, 232, 195, 69, 132, 81,
    129, 6, 241, 10, 80, 45, 166, 142, 191, 16, 244, 225, 47, 228, 196, 64,
    25, 177, 4, 0, 75, 82, 3, 17, 146, 162, 64, 1, 101, 9, 128, 56,
    65, 31, 240, 85, 152, 8, 0, 0,
};
static unsigned dv_coordinate(unsigned i)
{
    unsigned bit=i*15,at=bit>>3;
    unsigned word=(unsigned)DV_XY_PACKED[at] | ((unsigned)DV_XY_PACKED[at+1]<<8) | ((unsigned)DV_XY_PACKED[at+2]<<16);
    return (word>>(bit&7))&32767u;
}
/* Lossless 9-bit coordinate offset + 6-bit point count; no geometry quantisation. */
static const uint16_t DV_POLY[] = {
    12834, 1401, 1443, 1423, 1411, 12959, 5913, 4952,
    1399, 1447, 1407, 1421, 1441, 12909, 5924, 1391,
    1457, 1393, 1453, 12884, 5935, 1395, 1451, 1435,
    12859, 5946, 1397, 1449, 1435, 12934, 5902, 1445,
    1455, 5445, 2922, 1461, 1463, 9438, 4943, 1427,
    1419, 1413, 1437, 9420, 9456, 1425, 1417, 1433,
    1415, 1439, 10424, 1431, 1405, 1459, 17408, 1429,
    1409, 12834, 1401, 1443, 1423, 1411, 12959, 5913,
    4952, 1399, 1447, 1407, 1421, 1441, 12909, 5924,
    1391, 1457, 1393, 1453, 12884, 5935, 1395, 1451,
    1435, 12859, 5946, 1397, 1449, 1435, 12934, 5902,
    1445, 1455, 5445, 2922, 1461, 1463, 6402, 1403,
    4961, 10424, 1431, 1405, 1459, 9420, 9456, 1425,
    1417, 1433, 1415, 1439, 9438, 4943, 1427, 1419,
    1413, 1437,
};
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
        uint16_t path = DV_POLY[b->first + p];
        unsigned offset=path&511u;
        float xy[128];
        n = path>>9;
        if (n > 64) n = 64;
        for (i = 0; i < n; i++) {
            float px = (float)dv_coordinate(2 * (offset + i)) / 1000.0f;
            float py = (float)dv_coordinate(2 * (offset + i) + 1) / 1000.0f;
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
    if (role == DG_TUNE || (d && name_is(d, "Tune"))) {
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
        float ms = 0, reach;
        int uu = engine_param_si(r.a, r.b, r.c, val, &ms);
        if (uu == X0X_U_S) ms *= 1000.0f;
        if (uu != X0X_U_MS && uu != X0X_U_S) ms = q * 4000.0f;
        if (role == DG_NTAIL) ms = 24.0f + 0.7f * ms;
        if (ms < 0) ms = 0;
        if (ms > 4000) ms = 4000;
        reach = 8.0f + 23.0f * fm_sqrtf(ms / 4000.0f); /* 8..31 */
        n = 0;
        for (i = 0; i <= 16; i++) {
            float t = (float)i / 16.0f;
            float px,py;
            if(r.a==T_909 && (uu==X0X_U_MS || uu==X0X_U_S)) {
                /* The production 909 exponential reaches 1e-5 at Decay ms.
                 * A common 2s axis makes changing decay visibly meaningful. */
                px=1.f+30.f*t;
                py=12.f*(1.f-fm_exp2f(-16.60964047f*(2000.f*t)/(ms>0?ms:1.f)));
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
            int xe=(noisy || uu==X0X_U_MS || uu==X0X_U_S)?dg_tail_px(ms):2+(int)(q*23.f+.5f);
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
            float yv = fm_log2f(hz / 40.0f) / 3.32f;
            if (yv < 0) yv = 0;
            if (yv > 1) yv = 1;
            xy[2*n] = ox + (31.0f * (float)i / 24.0f) * sx;
            xy[2*n+1] = oy + (12.0f * (1.0f - yv)) * sy;
            n++;
        }
        cv_poly_aa(xy, n, col);
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
static int dv_kick_pitch_y(float hz,int top,int bottom)
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
    if(voice==DR_SD){curve->hold=24.f;curve->window=24.f+.7f*horizon;}
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
    const dpart_t *dp;
    int len, s;
    int body_y, yb;
    cv_rect(0, 0, 240, bh, SOUND_BASE);

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
                    {
                        char caption[16];const x0x_param_t *d=pref_desc(r);int j=0;
                        if(d&&d->name)for(;d->name[j]&&j<14;j++)caption[j]=d->name[j]>='a'&&d->name[j]<='z'?d->name[j]-32:d->name[j];
                        caption[j]=0;
                        if(d&&name_is(d,"P.Dpth"))put_s(caption,"DEPTH");
                        text_c(vx,y+14-off,&FONT_T,caption,gray);
                    }
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
                    const x0x_param_t *d=pref_desc(r);char caption[16];int j=0;
                    if(d&&d->name)for(;d->name[j]&&j<14;j++)caption[j]=d->name[j]>='a'&&d->name[j]<='z'?d->name[j]-32:d->name[j];
                    caption[j]=0;
                    if(d&&name_is(d,"P.Dpth"))put_s(caption,"DEPTH");
                    text_c(gx+22,y+14-off,&FONT_T,caption,gray);
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
        int pat_i, slen;
        if (ph < 40) ph = 40;
        if (py + ph > 0 && py < bh) {
            rbox(3, py, 234, ph, UI_PANEL_RADIUS, SOUND_FACE);
            /* Real voice identity and actual16 steps share the remaining
             * context surface. Rows reflow; no activity or data is invented. */
            /* One continuous16-step window, matching the white-key input.
             * Keep the voice identity above it, never over the step lane. */
            int left=9, pitch=14, step_top=body_y+ph-25;
            unsigned first=(unsigned)ui.spage*16u;
            if(first>=NSTEPS)first=0;
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
            char identity[32];
            put_i(put_s(put_s(identity,dv_voice_name(t,v)),t==T_808?" 808/":" 909/"),v+1);
            cv_text(11,body_y+3-off,&FONT_T,identity,col);
            pat_i = seq.ppat[ui.part];
            slen = cur_pat()->drum[ui.part].len;
            if (slen < 1) slen = 16;
            {
                char rhs[16];
                put_s(put_i(put_s(put_i(put_s(rhs, "P"), pat_i + 1), " / "), slen), "");
                text_r(226,body_y+3-off,&FONT_T,rhs,gray);
            }
            dp = &cur_pat()->drum[ui.part];
            len = dp->len;
            for (s = 0; s < 16; s++) {
                int sx = left+s*pitch;
                int sy=step_top-off;
                int absolute=(int)first+s;
                int hit = absolute < NSTEPS && absolute < len && sm_get(dp->hit[v], absolute);
                int curstep = seq.playing && (eng_step[TRK_DRUM + ui.part] % (len ? len : 16)) == absolute;
                uint16_t sc = hit ? col : SOUND_LINE;
                if (curstep) sc = C_WHITE;
                cv_rect(sx, sy, 11, 4, sc);
                {
                    char nn[4];put_i(nn, absolute + 1);
                    text_c(sx+5,sy+6,&FONT_T,nn,gray);
                }
            }
        }
    }
}


