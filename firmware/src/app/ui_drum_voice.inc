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

/* Exact authored millipixel coordinates: decode to float only for painting. */
/* Lossless dictionary encoding of authored millipixel coordinates. */
#define DV_XY_COUNT 1650
static const uint16_t DV_COORD[] = {
    100,200,300,600,800,855,900,1000,1014,1100,1150,1200,1269,1300,1350,1400,
    1406,1571,1589,1600,1700,1725,1833,1986,2000,2025,2027,2044,2100,2152,2156,2175,
    2289,2394,2400,2406,2414,2500,2573,2700,2725,2789,2800,2814,3000,3139,3200,3300,
    3350,3400,3427,3478,3500,3531,3556,3600,3700,3725,3786,3800,3825,3863,3945,3952,
    4000,4167,4191,4239,4261,4325,4400,4429,4500,4503,4533,4594,4612,4650,4677,4700,
    4739,4750,4812,4850,4866,4900,4938,4950,4972,5000,5006,5100,5181,5200,5314,5400,
    5415,5438,5500,5515,5600,5645,5658,5725,5750,5800,5850,5875,5925,5931,5950,5969,
    6020,6044,6050,6200,6250,6300,6350,6369,6375,6381,6400,6456,6500,6539,6550,6600,
    6619,6625,6700,6750,6775,6800,6900,6910,6925,6950,6969,7000,7019,7034,7045,7047,
    7100,7153,7189,7200,7287,7356,7375,7438,7500,7525,7544,7600,7603,7619,7625,7650,
    7672,7700,7709,7750,7752,7825,7889,7900,8000,8050,8100,8106,8125,8150,8200,8222,
    8250,8269,8300,8318,8350,8400,8422,8438,8450,8485,8500,8511,8675,8700,8797,8800,
    8819,8850,8919,8938,8956,8975,9000,9028,9100,9188,9200,9211,9247,9250,9300,9350,
    9400,9425,9500,9547,9575,9650,9688,9700,9706,9750,9800,9861,9869,9875,9891,9925,
    9938,9975,10000,10062,10100,10128,10172,10175,10250,10300,10312,10335,10375,10500,10566,10650,
    10700,10800,11000,11300,11500,11600,11700,12000,12139,12500,12625,12665,12700,12750,12812,12825,
    12872,13000,13025,13075,13125,13131,13200,13250,13312,13425,13500,13750,14047,14111,14150,14200,
    14500,14550,14650,14682,14688,14750,14811,14812,14875,15000,15100,15248,15250,15350,15397,15400,
    15422,15475,15500,15800,15847,15950,15955,16000,16075,16090,16225,16250,16375,16400,16450,16619,
    16650,16700,16750,16800,16956,17000,17062,17150,17172,17275,17500,17562,17585,17600,17761,17800,
    18000,18188,18250,18261,18500,19062,19200,19297,19500,20000,20500,21000,22000,
};
static const uint8_t DV_INDEX[] = {
    31,191,112,236,99,113,83,195,93,6,151,225,171,38,79,219,
    182,106,191,150,153,218,37,105,211,160,62,127,190,12,34,34,
    201,133,41,16,191,36,108,83,32,239,0,62,5,6,37,52,
    4,97,209,5,71,34,31,191,96,234,69,166,29,196,94,45,
    219,201,211,38,79,158,184,120,247,226,17,101,49,84,198,78,
    30,46,131,100,60,130,112,9,189,38,214,150,160,58,226,39,
    72,24,30,80,100,32,97,1,197,68,152,15,126,44,160,138,
    96,13,2,61,7,8,33,100,140,224,210,194,206,15,67,77,
    16,101,129,132,5,146,80,116,108,161,26,150,141,25,189,103,
    7,153,101,28,38,211,22,209,80,68,89,100,17,20,168,18,
    172,82,221,86,209,115,69,112,85,197,82,44,43,1,21,180,
    232,201,34,76,39,159,156,196,164,203,153,180,83,242,174,221,
    147,165,208,25,196,83,63,69,240,92,65,211,133,201,26,18,
    117,20,244,65,15,8,55,31,180,116,56,170,17,199,5,152,
    21,93,68,128,185,16,103,130,35,6,180,16,112,43,64,143,
    64,193,2,18,17,152,100,64,115,2,207,11,63,69,116,17,
    209,37,20,161,88,180,120,125,19,70,207,216,64,97,18,121,
    169,140,245,51,212,79,17,106,128,168,169,115,19,200,205,58,
    117,80,137,137,23,19,149,206,120,51,69,188,92,145,210,197,
    198,26,14,117,0,244,65,15,72,57,31,193,116,144,170,113,
    200,69,158,21,115,68,228,185,112,104,2,41,6,193,16,148,
    43,64,143,0,192,2,14,17,108,100,144,114,194,203,11,51,
    69,204,17,49,71,79,38,69,193,44,157,235,68,47,212,191,
    78,14,45,133,84,52,83,207,76,17,118,128,216,201,2,19,
    8,204,44,244,38,209,75,119,82,4,201,21,30,93,76,172,
    177,80,199,62,31,244,128,172,243,209,76,199,172,26,156,92,
    68,90,177,72,68,164,11,156,38,204,98,208,12,193,186,2,
    244,8,236,43,176,16,193,68,6,30,39,144,188,112,82,196,
    34,17,139,210,176,186,211,140,16,59,70,244,32,233,99,180,
    144,16,198,59,39,211,156,20,49,8,200,32,35,43,129,172,
    52,66,111,18,189,116,64,23,249,212,100,51,85,201,92,16,
    129,249,59,38,15,89,178,99,152,128,201,201,101,37,213,16,
    77,64,22,13,217,99,197,141,156,49,152,184,36,195,34,175,
    138,63,44,16,185,148,28,99,211,141,207,61,64,23,1,89,
    4,196,24,154,105,152,192,53,51,39,15,93,63,115,16,193,
    173,156,6,212,24,208,69,34,9,164,51,52,243,65,61,76,
    44,140,133,50,2,109,206,162,94,247,92,141,140,1,180,203,
    67,113,202,228,177,48,134,108,199,188,5,1,85,200,19,131,
    108,7,209,6,74,5,36,5,194,212,8,141,47,98,146,96,
    234,33,109,6,61,24,10,103,136,236,161,51,9,210,47,58,
    223,136,196,163,16,16,61,65,210,0,97,194,35,230,13,141,
    47,52,190,236,224,115,230,209,167,75,212,64,209,27,149,16,
    84,199,75,57,31,25,229,67,47,0,61,17,237,74,228,43,
    145,78,21,63,85,244,92,209,171,71,227,134,24,20,152,56,
    72,171,64,111,130,194,10,34,57,232,68,129,244,134,78,33,
    34,153,40,164,66,239,138,52,42,152,152,136,17,66,227,6,
    141,27,52,100,137,17,133,137,146,180,70,244,18,41,108,36,
    146,146,78,81,72,101,233,36,38,50,154,66,108,244,180,73,
    195,134,41,154,24,98,52,100,209,144,69,15,0,61,3,237,
    18,228,75,64,111,11,61,65,233,196,241,19,71,143,28,61,
    123,24,186,24,123,33,45,133,184,17,237,58,200,203,64,79,
    195,61,15,249,72,4,84,177,20,71,81,37,53,185,92,44,
    147,143,205,56,56,152,218,72,49,131,161,11,134,46,244,140,
    208,107,87,11,30,68,120,64,7,24,165,80,130,76,14,47,
    83,180,172,113,82,168,136,167,31,180,108,20,107,33,237,132,
    184,16,237,62,200,219,64,15,195,189,13,249,62,4,12,161,
    240,4,195,22,20,109,128,252,177,178,8,78,39,65,181,28,
    245,162,148,140,208,58,39,11,45,132,68,111,82,51,72,139,
    10,69,169,83,130,76,9,50,244,56,208,131,167,11,30,68,
    120,31,191,112,236,99,113,83,195,93,6,151,225,171,38,79,
    219,182,106,191,150,153,218,37,105,211,160,62,127,190,12,34,
    34,201,133,41,16,191,36,108,83,32,239,0,62,5,6,37,
    52,4,97,209,5,71,34,31,191,96,234,69,166,29,196,94,
    45,219,201,211,38,79,158,184,120,247,226,17,101,49,84,198,
    78,30,46,131,100,60,130,112,9,189,38,214,150,160,58,226,
    39,72,24,30,80,100,32,97,1,197,68,152,15,126,44,160,
    138,96,13,2,61,7,8,33,100,140,224,210,194,206,15,67,
    77,16,101,129,132,5,146,80,116,108,161,26,150,141,25,189,
    103,7,153,101,28,38,211,22,209,80,68,89,100,17,20,168,
    18,172,82,221,86,209,115,69,112,85,197,82,44,43,1,21,
    180,232,201,34,76,39,159,156,196,164,203,153,180,83,242,174,
    221,147,165,208,25,196,83,63,69,240,92,65,211,133,201,26,
    18,117,20,244,65,15,8,55,31,180,116,56,170,17,199,5,
    152,21,93,68,128,185,16,103,130,35,6,180,16,112,43,64,
    143,64,193,2,18,17,152,100,64,115,2,207,11,63,69,116,
    17,209,37,20,161,88,180,120,125,19,70,207,216,64,97,18,
    121,169,140,245,51,212,79,17,106,128,168,169,115,19,200,205,
    58,117,80,137,137,23,19,149,206,120,51,69,188,92,145,210,
    197,198,26,14,117,0,244,65,15,72,57,31,193,116,144,170,
    113,200,69,158,21,115,68,228,185,112,104,2,41,6,193,16,
    148,43,64,143,0,192,2,14,17,108,100,144,114,194,203,11,
    51,69,204,17,49,71,79,38,69,193,44,157,235,68,47,212,
    191,78,14,45,133,84,52,83,207,76,17,118,128,216,201,2,
    19,8,204,44,244,38,209,75,119,82,4,201,21,30,93,76,
    172,177,80,199,62,31,244,128,172,243,209,76,199,172,26,156,
    92,68,90,177,72,68,164,11,156,38,204,98,208,12,193,186,
    2,244,8,236,43,176,16,193,68,6,30,39,144,188,112,82,
    196,34,17,139,210,176,186,211,140,16,59,70,244,32,233,99,
    180,144,16,198,59,39,211,156,20,49,8,200,32,35,43,129,
    172,52,66,111,18,189,116,64,23,249,212,100,51,85,201,92,
    16,129,249,59,38,15,89,178,99,152,128,201,201,101,37,213,
    16,77,64,22,13,217,99,197,141,156,49,152,184,36,195,34,
    175,138,63,44,16,185,148,28,99,211,141,207,61,64,23,1,
    89,4,196,24,154,105,152,192,53,51,39,15,93,63,115,16,
    193,173,156,6,212,24,208,69,34,9,164,51,52,243,65,61,
    76,44,140,133,50,2,109,206,162,94,247,92,141,140,1,180,
    203,67,113,202,228,177,48,134,108,199,188,5,1,85,200,19,
    131,108,7,209,6,74,5,36,5,194,212,72,173,3,16,15,
    16,117,38,211,91,70,117,7,221,209,123,151,205,29,42,117,
    116,188,33,113,86,235,192,166,18,26,75,124,113,134,200,23,
    174,91,225,94,209,107,37,240,85,196,91,40,125,245,116,134,
    161,139,177,23,210,82,136,27,209,174,131,188,12,244,52,220,
    243,144,143,68,64,21,75,113,20,85,82,147,203,197,50,249,
    216,140,131,131,169,141,20,51,24,186,96,232,66,207,8,189,
    118,181,224,65,132,7,116,128,81,10,52,110,136,65,129,137,
    131,180,10,244,38,40,172,32,146,131,78,20,72,111,232,20,
    34,146,137,66,42,244,174,72,163,130,137,137,24,33,52,110,
    208,184,65,67,150,24,81,152,40,73,107,68,47,145,194,70,
    34,41,233,20,133,84,150,78,98,34,163,41,196,70,79,155,
    52,108,152,162,137,33,70,67,22,13,89,244,0,208,51,208,
    46,65,190,4,244,182,208,19,148,78,28,63,113,244,200,209,
    179,71,227,139,152,36,152,122,72,155,65,15,134,194,25,34,
    123,232,76,130,244,139,206,55,34,241,40,4,68,79,144,52,
    64,152,240,136,121,67,227,11,141,47,59,248,156,121,244,233,
    18,53,80,244,70,37,4,213,241,82,206,71,70,249,208,11,
    64,79,68,187,18,249,74,164,83,197,79,21,61,87,244,234,
    1,0,
};
static uint16_t dv_xy(unsigned i)
{
    unsigned bit=i*9u, b=bit>>3;
    unsigned word=(unsigned)DV_INDEX[b]|((unsigned)DV_INDEX[b+1]<<8);
    return DV_COORD[(word>>(bit&7u))&511u];
}
typedef struct { uint16_t off; uint8_t n; } dv_poly_t;
static const dv_poly_t DV_POLY[] = {
    {0, 25},
    {25, 2},
    {27, 2},
    {29, 2},
    {31, 2},
    {33, 25},
    {58, 11},
    {69, 9},
    {78, 2},
    {80, 2},
    {82, 2},
    {84, 2},
    {86, 2},
    {88, 25},
    {113, 11},
    {124, 2},
    {126, 2},
    {128, 2},
    {130, 2},
    {132, 25},
    {157, 11},
    {168, 2},
    {170, 2},
    {172, 2},
    {174, 25},
    {199, 11},
    {210, 2},
    {212, 2},
    {214, 2},
    {216, 25},
    {241, 11},
    {252, 2},
    {254, 2},
    {256, 10},
    {266, 5},
    {271, 2},
    {273, 2},
    {275, 18},
    {293, 9},
    {302, 2},
    {304, 2},
    {306, 2},
    {308, 2},
    {310, 18},
    {328, 18},
    {346, 2},
    {348, 2},
    {350, 2},
    {352, 2},
    {354, 2},
    {356, 20},
    {376, 2},
    {378, 2},
    {380, 2},
    {382, 34},
    {416, 2},
    {418, 2},
    {420, 25},
    {445, 2},
    {447, 2},
    {449, 2},
    {451, 2},
    {453, 25},
    {478, 11},
    {489, 9},
    {498, 2},
    {500, 2},
    {502, 2},
    {504, 2},
    {506, 2},
    {508, 25},
    {533, 11},
    {544, 2},
    {546, 2},
    {548, 2},
    {550, 2},
    {552, 25},
    {577, 11},
    {588, 2},
    {590, 2},
    {592, 2},
    {594, 25},
    {619, 11},
    {630, 2},
    {632, 2},
    {634, 2},
    {636, 25},
    {661, 11},
    {672, 2},
    {674, 2},
    {676, 10},
    {686, 5},
    {691, 2},
    {693, 2},
    {695, 12},
    {707, 2},
    {709, 9},
    {718, 20},
    {738, 2},
    {740, 2},
    {742, 2},
    {744, 18},
    {762, 18},
    {780, 2},
    {782, 2},
    {784, 2},
    {786, 2},
    {788, 2},
    {790, 18},
    {808, 9},
    {817, 2},
    {819, 2},
    {821, 2},
    {823, 2},
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
        const dv_poly_t *pl = &DV_POLY[b->first + p];
        float xy[128];
        n = pl->n;
        if (n > 64) n = 64;
        for (i = 0; i < n; i++) {
            float px = (float)dv_xy(2 * (pl->off + i)) / 1000.0f;
            float py = (float)dv_xy(2 * (pl->off + i) + 1) / 1000.0f;
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
    float back = dg_live_back(r, live);
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
            xy[2*n+1]=oy+(6.f-5.f*(live?(.3f+.7f*e):(.3f+.7f*back))*fm_sinf(6.28318531f*hz*.04f*t))*sy;n++;
        }
        cv_poly_aa(xy,n,col);return;
    }
    if (role == DG_TUNE || (d && name_is(d, "Tune"))) {
        /* The approved fork rings with the actual voice, then returns smoothly. */
        float amp=live ? e : back;
        dv_tune_fork(ox, oy, sx, sy*(0.65f+0.35f*amp), q, col);
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
        if (live && age < 100u) {
            float v=fm_log2f(engine_bd909_hz(tau,m,pm,(float)age)/40.f)/3.32f;
            v=fm_clampf(v,0.f,1.f);
            cv_rect((int)(ox+31.f*(float)age/100.f*sx),
                    (int)(oy+12.f*(1.f-v)*sy),2,2,C_WHITE);
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
        if (live) for(i=0;i<24;i+=2) {
            float t=(float)i/24.f;
            float tilt=fm_exp2f(fm_log2f(e<.001f?.001f:e)*(1.f+2.f*t));
            float h=(12.f-(xy[2*i+1]-oy)/sy)*tilt*(70+dg_rnd(i+(int)(age/80u))/3)/103.f;
            cv_line_aa(ox+31.f*t*sx,oy+12.f*sy,ox+31.f*t*sx,oy+(12.f-h)*sy,blend(C_LINE,col,7));
        }
        cv_poly_aa(xy, n, col);
        return;
    }
    if (role == DG_SOUND) {
        int count=d->max+1;
        float w=30.f/(float)count;
        for(i=0;i<count;i++) {
            int x=(int)(ox+(1.f+(float)i*w)*sx);
            int width=(int)((w-2.f)*sx);
            float h=9.f+(i==val && live?e*4.f:0.f);
            if(i==val)cv_rect(x,(int)(oy+(13.f-h)*sy),width,(int)(h*sy),col);
            else rbox(x,(int)(oy+4.f*sy),width,(int)(9.f*sy),1,C_LINE);
        }
        return;
    }
    if (role == DG_ACCENT) {
        /* four ticks, height follows accent */
        float hmax = (4.0f + 8.0f * q) * (live ? (0.6f+0.4f*e) : 0.6f);
        for (i = 0; i < 4; i++) {
            float hx = 2.0f + i * 8.0f;
            float hh = hmax * (0.25f + 0.25f * (float)i);
            cv_line_aa(ox + hx * sx, oy + 12 * sy, ox + hx * sx, oy + (12.0f - hh) * sy, col);
        }
        return;
    }
    if (role == DG_SNAPPY || role == DG_SHELLS) {
        /* a live burst / two shells, scaled into the slot */
        float k = 0.35f + 0.65f * (live ? e : back);
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
            rbox(3, by, 234, body_h, 9, SOUND_FACE);
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
                    rbox(6, ay, 228, ah, 9, ui_theme_light?SOUND_SELECT:RGB(36,48,54));
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
                    gx = 92; gy = y + 25;
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
            rbox(3, py, 234, ph, 9, SOUND_FACE);
            /* Real voice identity and actual16 steps share the remaining
             * context surface. Rows reflow; no activity or data is invented. */
            int cols=ph>=88?4:8, rows=16/cols;
            int left=cols==4?124:118, pitch=(228-left)/cols;
            int step_top=body_y+19, step_h=(ph-24)/rows;
            float body_scale=ph<64?.65f:1.f;
            int body_height=(int)(47.f*body_scale);
            dv_body_pictogram(t,v,10,body_y+(ph-body_height)/2-off,body_scale,env,col);
            cv_text(11,body_y+3-off,&FONT_T,dv_voice_name(t,v),col);
            pat_i = seq.ppat[ui.part];
            slen = cur_pat()->drum[ui.part].len;
            if (slen < 1) slen = 16;
            {
                char rhs[16];
                put_s(put_i(put_s(put_i(put_s(rhs, "P"), pat_i + 1), " / "), slen > 16 ? 16 : slen), "");
                text_r(226,body_y+3-off,&FONT_T,rhs,gray);
                char id[16];put_i(put_s(id,t==T_808?"808 / ":"909 / "),v+1);
                cv_text(11,body_y+ph-14-off,&FONT_T,id,gray);
            }
            dp = &cur_pat()->drum[ui.part];
            len = dp->len;
            for (s = 0; s < 16; s++) {
                int sx = left+(s%cols)*pitch;
                int sy=step_top+(s/cols)*step_h-off;
                int hit = s < NSTEPS && s < len && sm_get(dp->hit[v], s);
                int curstep = seq.playing && (eng_step[TRK_DRUM + ui.part] % (len ? len : 16)) == s;
                uint16_t sc = hit ? col : SOUND_LINE;
                if (curstep) sc = C_WHITE;
                cv_rect(sx, sy, 11, 4, sc);
                {
                    char nn[4];put_i(nn, s + 1);
                    text_c(sx+5,sy+6,&FONT_T,nn,gray);
                }
            }
        }
    }
}


