/* SPDX-License-Identifier: GPL-3.0-only */
/* R22 drum voice / SOUND page. Generated paths from
 * from-dot/R22-gallery/views/909-voice-00..10.svg and 808-voice-00..10.svg.
 * Included from ui.c after drum_strip_glyph. */

static const char *const DV_VOICE_NAMES[] = {
    "KICK", "SNARE", "LOW TOM", "MID TOM", "HIGH TOM", "RIMSHOT", "HANDCLAP",
    "CLOSED HAT", "OPEN HAT", "CRASH", "RIDE", "COWBELL", "CYMBAL"};
static const uint8_t DV_808_TAIL[]={11,12,8,7};

static const char *dv_voice_name(int t, int v)
{
    if (v < 0 || v >= NDRUM) return "KIT";
    if (t == T_808 && v >= 7) v=DV_808_TAIL[v-7];
    return DV_VOICE_NAMES[v];
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


#if X0X_UI_SMALL_CANVAS
#define DV_ROWS 3u
typedef struct {
    dv_ink ink;
    pref_t r[DV_ROWS][4];
    float eased[DV_ROWS][4];
    uint8_t value[DV_ROWS][4];
    uint8_t group[DV_ROWS],amp_row[DV_ROWS];
    uint8_t n,page,voice,kit,key_sound,pitch_valid;
    uint8_t live[2];
    uint32_t age[2];
    float env[2],back[2],pitch_ms,pitch_hz;
    uint8_t kick[3];
    uint8_t graph,attack_owner,decay_owner;
    uint8_t variant_decay_slot;
    char variant_decay_value[24];
    uint16_t first,hit,play;
    drum_curve_t b;
    union { drum_curve_t a; dv_amp_scene amp; } special;
} dv_scene;
#define DV_ARG ,const dv_scene *z
#define DV_FORWARD ,z
#define DV_NULL ,0
#define DV(v,default_value) (z ? (v) : (default_value))
#else
#define DV_ARG
#define DV_FORWARD
#define DV_NULL
#define DV(v,default_value) (default_value)
#endif

static int dv_pitch(float *elapsed,float *hz DV_ARG)
{
#if X0X_UI_SMALL_CANVAS
    if(z){*elapsed=z->pitch_ms;*hz=z->pitch_hz;return z->pitch_valid;}
#endif
    return engine_bd909_pitch_view(elapsed,hz);
}
#if X0X_UI_SMALL_CANVAS && !defined(DV_TEST_STOCK)
#undef DV
#define DV(v,default_value) (v)
#endif
static UI_DRAW void dv_amp_paint(int p,int active,int y,int h,int mode DV_ARG)
{
#if X0X_UI_SMALL_CANVAS
#if defined(DV_TEST_STOCK)
    if(z)
#endif
    {dv_drum_amp_inline(z->r[p][0],active,y,h,mode,&z->special.amp,&z->ink);return;}
#endif
    if(mode==0)draw_adsr_inline(p,active,y,h,0);
    else draw_drum_amp_inline(p,active,y,h,mode);
}
#if X0X_UI_SMALL_CANVAS && !defined(DV_TEST_STOCK)
#undef DV
#define DV(v,default_value) (z ? (v) : (default_value))
#endif
static UI_DRAW void dv_body_scene(int t, int v, int x, int y, float scale, float env, uint16_t col DV_ARG)
{
    if (v < 0 || v >= NDRUM) return;
    const dv_body_t *body=&DV_BODY[t==T_808][v];
    float s=3.0f * scale * (1.0f + 0.06f * env);
    uint16_t ink=env > 0.02f ? blend(col, DV(z->ink.C[0],C_WHITE), (int)(env * 6.0f + 0.5f)) : col;
    for (int p=0; p<body->npoly; p++){
        unsigned path=dv_path(body->first + p), offset=path&2047u, n=path>>11;
        if(n>64)n=64;
        float old_x=0,old_y=0;
        for(unsigned i=0;i<n;i++){
            float px=(float)dv_coordinate_next(&offset) / 1000.0f;
            float py=(float)dv_coordinate_next(&offset) / 1000.0f;
            float next_x=(float)x + (px - 11.0f) * s + 11.0f * 3.0f;
            float next_y=(float)y + (py - 5.4f) * s + 5.4f * 3.0f;
            if(i)cv_stroke_aa(old_x,old_y,next_x,next_y,3.6f*scale,ink);
            old_x=next_x;old_y=next_y;
        }
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

/* Common control-to-physical-value boundary for the kick glyph and KIT plot.
 * An eased input is only substituted for the currently drawn glyph control. */
static UI_DRAW void dv_kick_pitch_controls(float *out,int edit,int value DV_ARG)
{
    static const uint8_t pots[3]={0,4,5};
    for(int i=0;i<3;i++){
        int pot=pots[i];
        pref_t r=PR(R_ENG,T_909,DR_BD,pot);
        out[i]=dg_si(r,pot,pot==edit?value:DV(z->kick[i],matrix_value(r)));
    }
    out[1]*=.01f;
}

/* Scale of the selected single-control bank on the standard drum page. */
#define DV_ACTIVE_SINGLE_SCALE 1.8f

/* Live AA glyph in an R22 slot. Box origin is the path origin (unit 0..32, 0..17). */
static UI_DRAW void dv_glyph_scene(pref_t r, float ev, float ox, float oy, float sx, float sy, uint16_t col DV_ARG)
{
    int role = DV(drum_strip_role_at(r,V_PART),drum_strip_role(r));
    int val = (int)(ev + (ev >= 0 ? 0.5f : -0.5f));
    const x0x_param_t *d = pref_desc(r);
    float q = d && d->max ? ev / (float)d->max : 0;
    float e = 0;
    uint32_t age = 0xffffffffu;
    int live;
#if X0X_UI_SMALL_CANVAS
    if(z) { unsigned i=r.b==NDRUM;age=z->age[i];e=z->env[i];live=z->live[i]; }
    else
#endif
    live=drum_live(r,&age,&e);
    int i, n;
    float xy[80];
    if (q < 0) q = 0;
    if (q > 1) q = 1;
    if (r.kind == R_ENG && r.a == T_909 && r.b == DR_BD && r.c == 5) {
        /* PITCH projects its semitone setpoint over 40 ms, independently
         * of how much DEPTH applies that setting to the sounding kick.
         * Ask the same evaluator for the fully applied, settled carrier. */
        float hz=engine_bd909_hz(6.f,1.f,dg_si(r,5,val),1000.f);
        n=0;
        for(i=0;i<=32;i++) {
            float t=(float)i/32.f;
            xy[2*n]=ox+(1.f+30.f*t)*sx;
            xy[2*n+1]=oy+(4.5f-5.f*fm_sinf(6.28318531f*hz*.04f*t))*sy;n++;
        }
        cv_poly_aa(xy,n,col);return;
    }
    if (role == DG_TUNE || (role == DG_NONE && d && name_is(d, "Tune"))) {
        dv_tune_fork(ox, oy, sx, sy, q, col);
        return;
    }
    if (role == DG_ATTACK) {
        if (r.a == T_909 && r.b == DR_BD && sx == 1.375f && sy == 1.5f) {
            /* A fixed x-position beater impulse, with height driven by the
             * linear DSP click gain. Its three-millisecond time is fixed. */
            int x = (int)ox, y = (int)oy + 19;
            int h = (int)(18.f*q+.5f);
            uint16_t ink = live && age < 30u ? DV(z->ink.C[0],C_WHITE) : col;
            cv_rect(x+7,y,35,1,col);
            if (h) cv_rect(x+2,y-h+1,2,h,ink);
            return;
        }
        /* Normalized rising-edge symbol: x encodes attack AMOUNT, not time.
         * The parameter and its percentage retain the engine's click amount.
         * Zero is vertical; larger amounts lengthen the visible rising ramp. */
        float end=1.f+30.f*q;
        n=0;
        xy[2*n]=ox+sx;xy[2*n+1]=oy+12.f*sy;n++;
        xy[2*n]=ox+end*sx;xy[2*n+1]=oy;n++;
        xy[2*n]=ox+31.f*sx;xy[2*n+1]=oy;n++;
        cv_poly_aa(xy, n, live && age < 30u ? DV(z->ink.C[0],C_WHITE) : col);
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
        float back=DV(z->back[r.b==NDRUM],dg_live_back(r,live));
        uint16_t idle_col=!live && back<1.f?blend(DV(z->ink.C[9],C_LINE),col,(int)(16.f*back)):col;
        cv_poly_aa(xy, n, live?DV(z->ink.C[9],C_LINE):idle_col);
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
                        cv_line_aa(px,py,px,oy+12.f*sy,i?col:DV(z->ink.C[0],C_WHITE));
                }
            }
            if(n>1)cv_poly_aa(xy,n,col);
        }
        return;
    }
    if (role == DG_BDSWEEP) { /* Tune and Depth preview the kick pitch drop. */
        float controls[3];
        dv_kick_pitch_controls(controls,r.c,val DV_FORWARD);
        float tau=controls[0],m=controls[1],pm=controls[2];
        if (r.c == 0) {
            /* Centre the 0.0.26 26x19 raster in the Tune slot. Compact
             * summaries sample the same 100 ms window at their own width. */
            int width = (int)(32.f * sx + .5f);
            /* Only whole pixel rows fit above the next bank's panel. */
            int height = (int)(18.f * sy);
            int left = (int)ox, top = (int)oy;
            if (width > OG_W) { left += (width - OG_W) / 2; width = OG_W; }
            if (height > OG_H + 1) { top += (height - OG_H - 1) / 2; height = OG_H + 1; }
            int bottom = top + height - 1;

            /* The hit timestamp uses bit zero as its nonzero sentinel.
             * On an even trigger tick its stored time is 1 ms in the future. */
            if (live && age == UINT32_MAX) age = 0;
            float back = DV(z->back[r.b==NDRUM],dg_live_back(r,live));
            if (!live && back < 1.f) col = blend(DV(z->ink.C[9],C_LINE), col, (int)(16.f * back));
            cv_rect(left, bottom, width, 1, DV(z->ink.C[9],C_LINE));
            int previous = bottom;
            for (i = 0; i < width; i++) {
                float t = i * 100.f / (width - 1);
                float level = dv_sweep_level(engine_bd909_hz(tau, m, pm, t));
                int y = bottom - 1 - (int)(level * (height - 3) + .5f);
                if (i) cv_line(left + i - 1, previous, left + i, y,
                               live && t < (float)age ? DV(z->ink.C[9],C_LINE) : col);
                previous = y;
            }

            if (sx == 1.375f && sy == 1.5f) {
                /* First 8 ms are within the engine's constant-pitch hold.
                 * This trace previews the requested onset, not played audio. */
                float phase = 6.28318531f * .008f * engine_bd909_hz(tau,m,pm,0.f);
                for (i = 0; i < width; i++) {
                    xy[2*i] = left + i;
                    xy[2*i+1] = bottom + 4.2f - 1.6f * fm_sinf(phase * i / (width-1));
                }
                cv_poly_aa(xy, width, col);
            }

            /* X follows hit age. Y follows the oscillator that is sounding,
             * even when the player edits the next kick before it retriggers. */
            float elapsed, hz;
            if (live && age < 100u && dv_pitch(&elapsed,&hz DV_FORWARD)) {
                int x = (int)(age * (width - 1) / 100.f + .5f);
                if (x < 1) x = 1;
                if (x > width - 2) x = width - 2;
                int y = bottom - 1 - (int)(dv_sweep_level(hz) * (height - 3) + .5f);
                ui_curve_cursor(left+x,y,DV(z->ink.C[0],C_WHITE));
            }
            return;
        }
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
        if(dv_pitch(&ms,&hz DV_FORWARD) && ms>=0 && ms<=100) {
            float level=dv_sweep_level(hz);
            int mx=(int)(ox+31.f*ms/100.f*sx+.5f);
            int my=(int)(oy+12.f*(1.f-level)*sy+.5f);
            ui_curve_cursor(mx,my,DV(z->ink.C[0],C_WHITE));
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
        /* The focused standard KIT slot uses the shared strike peaks. */
        if (sx == DV_ACTIVE_SINGLE_SCALE && sy == DV_ACTIVE_SINGLE_SCALE &&
            DV(z->n,pg.n) && DV(z->group[z->page],pg.group[cur_page()]) == GR_KIT &&
            ((r.a == T_808 && DV(z->voice,ui.sel[PART_808]) != D8_CP && name_is(d,"Accent")) ||
             (r.a == T_909 && DV(z->voice,ui.sel[PART_909]) == DR_BD && r.b == NDRUM && r.c == 1))) {
            dg_accent((int)ox+4,(int)oy,43,22,q,live,e,col,DV(z->ink.C[9],C_LINE),DV(z->ink.C[0],C_WHITE));
            return;
        }
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
            float levels[2]={fm_cosf(ang) * 12.0f * k,fm_sinf(ang) * 12.0f * k};
            float base=oy+12*sy;
            for(int shell=0;shell<2;shell++){
                int x=7+shell*10;
                float peak=oy+(12-levels[shell])*sy;
                float middle=ox+(x+4)*sx;
                cv_line_aa(ox+x*sx,base,middle,peak,col);
                cv_line_aa(middle,peak,ox+(x+8)*sx,base,col);
            }
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

#if X0X_UI_SMALL_CANVAS && !defined(DV_TEST_STOCK)
#undef DV
#define DV(v,default_value) (v)
#endif
static UI_DRAW void dv_value_text(pref_t r, int ev_i, int cx, int y, uint16_t col, int large DV_ARG)
{
    char t[24];
    const felucca_font_t *f = large ? &FONT_S : &FONT_T;
#if X0X_UI_SMALL_CANVAS
    if(
#if defined(DV_TEST_STOCK)
       z &&
#endif
       z->variant_decay_slot==r.c && r.b==z->voice) {
        text_c(cx,y,f,z->variant_decay_value,col);return;
    }
#endif
    dv_readout(t,r,ev_i);
    text_c(cx, y, f, t, col);
}

/* Format helpers already exist; text_c used above. */

static UI_DRAW int dv_909_curve(int voice,drum_curve_t *curve)
{
    engine_decay_view_t v;
    if(!engine_drum909_decay_view(voice,&v))return 0;
    int pot=voice<=DR_HT?1:voice==DR_CH || voice==DR_OH?0:2;
    float horizon;
    if(!drum909_param_si(voice,pot,127,&horizon))return 0;
    memset(curve,0,sizeof *curve);
    curve->peak=v.peak;
    curve->elapsed=v.elapsed_ms;
    curve->level=v.level;
    curve->window=horizon;
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
static UI_DRAW int dv_kick_pitch_y(float hz,int top,int bottom)
{
    float octaves=fm_clampf(fm_log2f(hz/20.f),0.f,5.f);
    return bottom-(int)((bottom-top)*octaves*.2f+.5f);
}

static UI_DRAW void dv_curve_row(drum_curve_t *v,int kick,int top,int bottom,uint16_t col DV_ARG)
{
    if(bottom<cv_y0 || top>=cv_y1 || top>=bottom)return;
    float controls[3],elapsed=v->elapsed,level=v->level;
    if(kick) {
        dv_kick_pitch_controls(controls,-1,0 DV_FORWARD);
        if(!dv_pitch(&elapsed,&level DV_FORWARD))elapsed=-1;
    }
    int left=13,width=214;
    /* The admitted 909 KIT graph has only 29 px; preserve the axes of the
     * taller amplitude rows. */
    if(!kick && bottom-top>=36) {
        /* A physical time/level scale explains natural empty plot regions.
         * The plotted window is not the selected decay control. Dense rows
         * keep their existing plot instead of squeezing in unreadable axes. */
        int end=bottom,mid;
        char time[16];
        left=33;width=194;bottom-=FONT_T.h+2;mid=(top+bottom)/2;
        cv_line(left,mid,left+width,mid,DV(z->ink.C[4],SOUND_LINE));
        cv_line(left+width/2,top,left+width/2,bottom,DV(z->ink.C[4],SOUND_LINE));
        text_r(29,top-2,&FONT_T,"100%",DV(z->ink.C[5],SOUND_GRAY));
        text_r(29,mid-3,&FONT_T,"50%",DV(z->ink.C[5],SOUND_GRAY));
        text_r(29,bottom-4,&FONT_T,"0%",DV(z->ink.C[5],SOUND_GRAY));
        cv_text(left,end-FONT_T.h,&FONT_T,"0",DV(z->ink.C[5],SOUND_GRAY));
        put_i(time,(int)(v->window*.5f+.5f));
        text_c(left+width/2,end-FONT_T.h,&FONT_T,time,DV(z->ink.C[5],SOUND_GRAY));
        put_s(put_i(time,(int)(v->window+.5f)),"ms");
        text_r(left+width,end-FONT_T.h,&FONT_T,time,DV(z->ink.C[5],SOUND_GRAY));
    }
    float points[72];
    float onset[3]={v->delay,v->delay+v->rise_end,v->delay+v->rise_end+v->hold};
    int used=0,stage=0,i=0;
    while(i<=32) {
        float t=v->window*i/32.f;
        if(stage<3 && onset[stage]<t)t=onset[stage++];else i++;
        points[used++]=(float)left+(float)width*t/v->window;
        if(kick){
            float hz=engine_bd909_hz(controls[0],controls[1],controls[2],t);
            points[used++]=dv_kick_pitch_y(hz,top,bottom);
        } else points[used++]=bottom-(bottom-top)*fm_minf(1.f,dv_curve_level(v,t));
    }
    cv_poly_aa(points,used/2,col);
    if(elapsed>=0 && elapsed<=v->window){
        int x=left+(int)((float)width*elapsed/v->window+.5f);
        int y=kick?dv_kick_pitch_y(level,top,bottom):
            bottom-(int)((bottom-top)*fm_clampf(level,0,1)+.5f);
        ui_curve_cursor(x,y,DV(z->ink.C[6],SOUND_INK));
    }
}

/* A bounded stage curve is a target-law preview. The separate right-hand
 * level tick is actual owner state, not reconstructed horizontal progress. */
static UI_DRAW void dv_bound_stage(drum_curve_t *v,int stage,int top,int off,uint16_t col DV_ARG)
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
    cv_text(13,bottom-off,&FONT_T,"0",DV(z->ink.C[5],SOUND_GRAY));
    text_r(227,bottom-off,&FONT_T,label,DV(z->ink.C[5],SOUND_GRAY));
    if(v->elapsed>=0 && v->level>=0 && v->level<1.e30f) {
        int y=bottom-off-(int)(height*fm_clampf(v->level,0,1)+.5f);
        cv_rect(230,y,3,1,DV(z->ink.C[6],SOUND_INK));
    }
}
/* Shared identity and white-key window. Preserve selected-owner bounds and
 * the legacy generic window fallback; neither changes sequencer state. */
static UI_DRAW void dv_voice_steps(int t,int v,int identity_y,int step_y,int strict DV_ARG)
{
    uint16_t col=DV(z->ink.col,part_col());
    const dpart_t *dp=DV(0,&cur_pat()->drum[ui.part]);
    int len=dp?dp->len:0;
    unsigned first=DV(z->first,(unsigned)ui.spage*16u);
    if(!strict && first>=NSTEPS)first=0;
    char label[32];
    put_i(put_s(put_s(label,dv_voice_name(t,v)),t==T_808?" 808/":" 909/"),v+1);
    cv_text(11,identity_y,&FONT_T,label,col);
    /* no "P# / length" here: the header carries P#, the step ruler shows the window, and
     * LENGTH is its own control (FINAL-REFERENCES: "1-16 / 17" restates LENGTH) */
    for(int s=0;s<16;s++) {
        int absolute=(int)first+s,sx=9+s*14;
        int valid=absolute<NSTEPS && absolute<len;
        int hit=DV((z->hit>>s)&1u,valid && sm_get(dp->hit[v],absolute));
        int live=DV((z->play>>s)&1u,seq.playing && (!strict||valid) && eng_step[TRK_DRUM+ui.part]%(len?len:16)==absolute);
        cv_rect(sx,step_y,11,4,live?DV(z->ink.C[0],C_WHITE):hit?col:DV(z->ink.C[4],SOUND_LINE));
        put_i(label,absolute+1);text_c(sx+5,step_y+6,&FONT_T,label,DV(z->ink.C[5],SOUND_GRAY));
    }
}

/* Nominal target 99% stage duration, never an observed transition time. */
/* One readout per stage: its physical 99% duration where the owner has a curve, else the knob
 * percentage. Never both (FINAL-REFERENCES 808-HANDCLAP: "% alongside ms"). */
static UI_DRAW void dv_stage_value(char *out,char stage,int lettered,int have,float ms,int raw)
{
    char *q=out;if(lettered){*q++=stage;*q++=' ';}
    if(have)put_s(put_d1(q,ms,0),"ms");else put_s(put_i(q,(raw*100+63)/127),"%");
}

static UI_DRAW void dv_cp_stage_page(int band DV_ARG)
{
    int off=HDR_H+band*BAND_H,page=DV(z->page,cur_page());
    uint16_t col=DV(z->ink.col,part_col()),ink=page==0?DV(z->ink.C[6],SOUND_INK):DV(z->ink.C[5],SOUND_GRAY);
    drum_curve_t attack={.elapsed=-1},decay={.elapsed=-1};
    int raw_a=DV(z->value[0][1],matrix_value(pg.r[0][1]));
    int raw_b=DV(z->value[0][3],matrix_value(pg.r[0][3]));
    int choice=DV(z->value[0][2],matrix_value(pg.r[0][2]));
    int attack_owner;
    int decay_owner;
#if X0X_UI_SMALL_CANVAS
#if defined(DV_TEST_STOCK)
    if(z)
#endif
    {attack=z->special.a;decay=z->b;attack_owner=z->attack_owner;decay_owner=z->decay_owner;}
#if defined(DV_TEST_STOCK)
    else
#endif
#endif
#if !X0X_UI_SMALL_CANVAS || defined(DV_TEST_STOCK)
    {
    attack_owner=engine_drum808_param_curve_preview(D8_CP,3,choice,raw_a,&attack);
    decay_owner=engine_drum808_param_curve_preview(D8_CP,2,choice,raw_b,&decay);
    }
#endif
    int selected=decay_owner==D8S_MA+1;
    rbox(3,24-off,234,213,UI_PANEL_RADIUS,DV(z->ink.C[2],SOUND_FACE));
    cv_text(11,28-off,&FONT_T,page==0?"SOUND":"KIT",ink);
    if(!DV(z->key_sound,key_sounds()))cv_text(91,28-off,&FONT_T,"KEY SOUND OFF",DV(z->ink.C[5],SOUND_GRAY));
    char label[32];put_i(put_s(put_i(label,page+1),"/"),DV(z->n,pg.n));
    text_r(232,28-off,&FONT_T,label,DV(z->ink.C[5],SOUND_GRAY));
    const int slots[2]={0,2};
    for(int i=0;i<2;i++) {
        pref_t r=DV(z->r[0][slots[i]],pg.r[0][slots[i]]);int center=i?155:83,value=DV(z->value[0][slots[i]],matrix_value(r));
        text_c(center,40-off,&FONT_T,i?"TYPE":"TUNE",DV(z->ink.C[5],SOUND_GRAY));   /* the bank title above is SOUND */
        dv_glyph_scene(r,DV(z->eased[0][slots[i]],ui_ease_float(r,value)),center-22,53-off,1.375f,1.f,col DV_FORWARD);
        dv_value_text(r,value,center,72-off,DV(z->ink.C[3],SOUND_WHITE),1 DV_FORWARD);
    }
    cv_text(13,88-off,&FONT_T,selected?"MARACAS AD":"MARACAS ATTACK",DV(z->ink.C[5],SOUND_GRAY));
    int have_a=attack_owner==D8S_MA+1;
    dv_stage_value(label,'A',selected,have_a,attack.rise_end,raw_a);
    if(selected)cv_text(13,100-off,&FONT_T,label,ink);
    else text_r(227,88-off,&FONT_T,label,ink);
    if(selected) {
        /* Both editable stages belong to MA: one AD graph, never two owners. */
        dv_stage_value(label,'D',1,decay_owner==D8S_MA+1,decay.fall*4.60517f,raw_b);
        text_r(227,100-off,&FONT_T,label,ink);
        if(have_a) {
            decay.rise_tau=attack.rise_tau;decay.rise_end=attack.rise_end;
            if(attack.elapsed>=0){decay.elapsed=attack.elapsed;decay.level=attack.level;}
        }
        dv_bound_stage(&decay,0,116,off,col DV_FORWARD);
    } else {
        if(have_a)dv_bound_stage(&attack,1,100,off,col DV_FORWARD);
        text_c(120,132-off,&FONT_T,"CLAP DECAY",DV(z->ink.C[5],SOUND_GRAY));
        dv_stage_value(label,'D',0,decay_owner==D8S_CP+1,decay.fall*4.60517f,raw_b);
        text_r(227,132-off,&FONT_T,label,ink);
        if(decay_owner==D8S_CP+1)dv_bound_stage(&decay,2,144,off,col DV_FORWARD);
    }
    pref_t kit=DV(z->r[1][0],pg.r[1][0]);int kv=DV(z->value[1][0],matrix_value(kit));
    cv_text(11,180-off,&FONT_T,"KIT ACCENT",page==1?DV(z->ink.C[6],SOUND_INK):DV(z->ink.C[5],SOUND_GRAY));
    dv_glyph_scene(kit,DV(z->eased[1][0],ui_ease_float(kit,kv)),105,179-off,.7f,.7f,col DV_FORWARD);
    dv_value_text(kit,kv,214,180-off,DV(z->ink.C[3],SOUND_WHITE),0 DV_FORWARD);
    dv_voice_steps(T_808,D8_CP,200-off,212-off,1 DV_FORWARD);
}

static UI_DRAW void dv_upper_caption(char *caption,const x0x_param_t *d,int max)
{
    if(d && d->name && name_is(d,"P.Dpth")){put_s(caption,"DEPTH");return;}
    int j=0;
    if(d && d->name)for(;j<max && d->name[j];j++){
        char ch=d->name[j];
        caption[j]=ch>='a' && ch<='z'?ch-32:ch;
    }
    caption[j]=0;
}

static UI_DRAW void dv_control_caption(pref_t r,int cx,int y DV_ARG)
{
    char caption[16];   /* the presentation name (TYPE, Depth), not the raw DSP id */
    const char *n=pref_name(r);int j=0;
    for(;j<14 && n && n[j];j++)caption[j]=n[j]>='a' && n[j]<='z'?n[j]-32:n[j];
    caption[j]=0;
    text_c(cx,y,&FONT_T,caption,DV(z->ink.C[5],SOUND_GRAY));
}

/* Existing placements in a standard drum bank, indexed by selection and
 * whether the bank has multiple controls. Offsets are from the row top.
 * A zero visible height keeps the selected single slot's lower-edge test only.
 * The selected multi-control row is 72 px high: value y = 72 - 16, and
 * glyph clipping height = 18 * 1.5 + 2. */
typedef struct {
    float sx, sy;
    uint8_t glyph_y, value_y, visible_height;
} dv_bank_slot_t;
static const dv_bank_slot_t dv_bank_slots[2][2] = {
    { /* Summary */  {1.3f, 1.5f, 5, 15, 20}, {.62f, .76f, 29, 29, 17} },
    { /* Selected */ {DV_ACTIVE_SINGLE_SCALE, DV_ACTIVE_SINGLE_SCALE, 23, 35, 0},
                     {1.375f, 1.5f, 28, 56, 29} }
};

/* The AMP bank's slot zero is K5 on SMK (K1 on the generic host). */
static UI_DRAW __attribute__((noinline)) void dv_kick_decay_caption(int page,int x,int y,uint16_t col DV_ARG)
{
    char caption[32];
    char *value=put_s(caption,"DECAY ")   /* no K-number prefix: "K5DECAY" read as one cryptic word */;
    pref_t r=DV(z->r[page][0],pg.r[page][0]);
    dv_readout(value,r,DV(z->value[page][0],matrix_value(r)));
    text_r(x,y,&FONT_T,caption,col);
}

static UI_DRAW void dv_bank_focus(int y,int height DV_ARG)
{
    rbox(6,y,228,height,UI_PANEL_RADIUS,DV(z->ink.focus,(ui_theme_light?SOUND_SELECT:RGB(36,48,54))));
}

static UI_DRAW void dv_draw(int band DV_ARG)
{
    int off = band * BAND_H, bh = dv_band_h(band);
    int t = DV(z->kit,ui.part==PART_808) ? T_808 : T_909;
    int v = DV(z->voice,ui.sel[ui.part]);
    int page = DV(z->page,cur_page());
    int bd_amp = -1;
    int kick_kit = t == T_909 && v == DR_BD && DV(z->group[page],pg.group[page]) == GR_KIT;
    int n = DV(z->n,pg.n);
    int p, y, active;
    uint16_t col = DV(z->ink.col,part_col());
    uint16_t ink = col, gray = DV(z->ink.C[5],SOUND_GRAY);
    uint32_t age = 0;
    float env = 0;
    int live;
    int body_y, yb;
    cv_rect(0, 0, 240, bh, DV(z->ink.C[1],SOUND_BASE));
    if(t==T_808 && v==D8_CP){dv_cp_stage_page(band DV_FORWARD);return;}

    /* body surface + banks: R22 y=24 (screen) => main y=4, height grows with bank count. */
    yb = DV_MAIN(24);
    /* Keep the lower voice/step panel compact. Spare space belongs to the
     * active controls, not an empty box between the pictogram and steps. */
    {
        int by = yb - off;
        int body_h;
        int used = 0;
        uint8_t bank_cells[MAXPAGES];
        int ah;
        for (p = 0; p < n; p++) {
            if(t==T_909 && v==DR_BD && drum_amp_ref(DV(z->r[p][0],pg.r[p][0]))) {
                bd_amp=p;
                if(p==page || kick_kit) used+=32; /* K5 focus summary, not a second graph */
                continue;
            }
            int cells = 0;
            for(int k=0;k<4;k++)if(DV(z->r[p][k],pg.r[p][k]).kind!=R_NONE)cells++;
            bank_cells[p]=(uint8_t)cells;
            ah = (p == page) ? (cells==1?56:72) : (cells>1?42:32);
            used += ah;
        }
        body_h = used + 4;
        if (by + body_h > 0 && by < bh)
            rbox(3, by, 234, body_h, UI_PANEL_RADIUS, DV(z->ink.C[2],SOUND_FACE));
        /* banks */
        y = yb + 2;
        for (p = 0; p < n; p++) {
            if(p==bd_amp) {
                if(p==page || kick_kit) {
                    if(p==page)
                        dv_bank_focus(y-off,32 DV_FORWARD);
                    cv_text(11,y+5-off,&FONT_T,"AMP",gray);
                    dv_kick_decay_caption(bd_amp,232,y+5-off,p==page?DV(z->ink.C[6],SOUND_INK):gray DV_FORWARD);
                    if(kick_kit)dv_amp_paint(bd_amp,0,y-off,32,-1 DV_FORWARD);
                    y+=32;
                }
                continue;
            }
            int cells = bank_cells[p],k;
            if (cells < 1) cells = 1;
            active = (p == page);
            ah = active ? (cells==1?56:72) : (cells>1?42:32);
            if (active) {
                int ay = y - off;
                if (ay + ah > 0 && ay < bh)
                    dv_bank_focus(ay,ah DV_FORWARD);
            }
            if(DV(z->amp_row[p],adsr_row(p))) {
                cv_text(11,y-off,&FONT_T,"AMP",gray);
                dv_amp_paint(p,active,y-off,ah,0 DV_FORWARD);y+=ah;continue;
            }
            /* labels */
            {
                                int ly = y + 2 - off;
                char pgind[8], a[16], b[16];
                const x0x_param_t *pd0;
                b[0] = 0;
                if (DV(z->r[p][0],pg.r[p][0]).kind == R_ENG && DV(z->r[p][0],pg.r[p][0]).b == NDRUM) {
                    put_s(a,"KIT");
                    put_s(b,kick_kit?"VELOCITY DEPTH":pref_name(DV(z->r[p][0],pg.r[p][0])));
                } else {
                    put_s(a, "SOUND");
                    pd0 = pref_desc(DV(z->r[p][0],pg.r[p][0]));
                    /* Multi-control rows already label each cell; a second
                     * first-control summary would overprint that caption. */
                    if (cells==1) dv_upper_caption(b,pd0,12);
                }
                if (ly + 12 > 0 && ly < bh) {
                    cv_text(11, ly, &FONT_T, a, gray);
                    if(p==0 && !DV(z->key_sound,key_sounds()))cv_text(91,ly,&FONT_T,"KEY SOUND OFF",gray);
                    put_i(put_s(put_i(pgind,p+1),"/"),DV(z->n,pg.n));
                    text_r(232, ly, &FONT_T, pgind, gray);
                }
                /* The second line may enter this strip after the first line
                 * has left it. Test its own bounds, not the first line's. */
                if (b[0] && ly + 24 > 0 && ly + 12 < bh)
                    cv_text(11, ly + 12, &FONT_T, b, gray);
            }
            /* glyphs + values */
            for (k = 0; k < 4; k++) {
                pref_t r = DV(z->r[p][k],pg.r[p][k]);
                int ev_i, vx, gy, gx;
                float ev;
                if (r.kind == R_NONE) continue;
                ev_i = DV(z->value[p][k],matrix_value(r));
                ev = DV(z->eased[p][k],ui_ease_float(r,ev_i));
                /* Only the pictogram eases; numeric state is committed now. */
                int multi = cells > 1;
                const dv_bank_slot_t *slot = &dv_bank_slots[active][multi];
                int row_y = y - off;
                if (multi) {
                    int center = 6 + (228*(2*k+1))/(2*cells);
                    gx = center - 22;
                    vx = active ? center : gx + 40;
                    dv_control_caption(r, center, row_y + 14 DV_FORWARD);
                } else {
                    gx = active ? 92 : 98;
                    vx = 214;
                }
                gy = row_y + slot->glyph_y;
                if (gy < bh && (slot->visible_height == 0 || gy + slot->visible_height > 0))
                    dv_glyph_scene(r,ev,(float)gx,(float)gy,slot->sx,slot->sy,active?ink:gray DV_FORWARD);
                int vy = row_y + slot->value_y;
                if (!active || !multi || (vy < bh && vy + 16 > 0))
                    dv_value_text(r,ev_i,vx,vy,DV(z->ink.C[3],SOUND_WHITE),active || !multi DV_FORWARD);
            }
            y += ah;
        }
        body_y = yb + body_h + 4; /* gap before the lower panel */
    }

    /* lower panel: body pictogram, voice name, P1/16, 16-step strip */
    #if X0X_UI_SMALL_CANVAS
#if defined(DV_TEST_STOCK)
    if(z)
#endif
    {live=z->live[0];env=z->env[0];}
#if defined(DV_TEST_STOCK)
    else
#endif
#endif
#if !X0X_UI_SMALL_CANVAS || defined(DV_TEST_STOCK)
    live=engine_drum_view(ui.part,v,&age,&env);
#endif
    if (!live) env = 0;
    {
        int py = body_y - off;
        int ph = 237 - HDR_H - body_y; /* to screen y=237 */
        if (ph < 40) ph = 40;
        if (py + ph > 0 && py < bh) {
            rbox(3, py, 234, ph, UI_PANEL_RADIUS, DV(z->ink.C[2],SOUND_FACE));
            /* Real voice identity and actual16 steps share the remaining
             * context surface. Rows reflow; no activity or data is invented. */
            /* One continuous16-step window, matching the white-key input.
             * Keep the voice identity above it, never over the step lane. */
            int step_top=body_y+ph-25;
            drum_curve_t curve;
            int graph;
            if(kick_kit){
                memset(&curve,0,sizeof curve);
                curve.window=160;
                graph=1;
            }
#if X0X_UI_SMALL_CANVAS
#if defined(DV_TEST_STOCK)
            else if(z)
#else
            else
#endif
            {curve=z->b;graph=z->graph;}
#endif
#if !X0X_UI_SMALL_CANVAS || defined(DV_TEST_STOCK)
            else graph=t==T_808?engine_drum808_curve(v,&curve):dv_909_curve(v,&curve);
#endif
            if(bd_amp>=0 && !kick_kit) {
                dv_body_scene(t,v,31,body_y-9-off,.32f,env,col DV_FORWARD);
                dv_amp_paint(bd_amp,bd_amp==page,py,step_top-body_y,1 DV_FORWARD);
                if(bd_amp!=page)dv_kick_decay_caption(bd_amp,139,body_y+3-off,gray DV_FORWARD);
            } else if(graph) {
                /* Keep the existing physical knobs, voice and all 16 steps.
                 * The editable envelope uses the full width between them. */
                dv_body_scene(t,v,79,body_y-9-off,.32f,env,col DV_FORWARD);
                const char *role=kick_kit?"PITCH ENV":"AMP ENV";
                if((t==T_909 && v==DR_SD) || (t==T_808 && v==D8_SD))role="NOISE ENV";
                else if((t==T_909 && v==DR_CP) || (t==T_808 && v==D8_CP && curve.delay>0))role="TAIL ENV";
                else if(t==T_808 && v==D8_CB)role="BODY ENV";
                else if(t==T_808 && v==D8_CY)role="SUSTAIN";
                cv_text(133,body_y+3-off,&FONT_T,role,gray);
                dv_curve_row(&curve,kick_kit,body_y+17-off,step_top-4-off,col DV_FORWARD);
            } else if(ph>44) {
                /* Reserve 28 px for the identity and step lane; keep the
                 * larger icon and its live pulse in the remaining body. */
                float scale=fm_minf(3.f,(float)(ph-28)/44.f);
                dv_body_scene(t,v,89,py+(ph-11)/2-16,scale,env,col DV_FORWARD);
            }
            dv_voice_steps(t,v,body_y+3-off,step_top-off,0 DV_FORWARD);
        }
    }
}



static UI_DRAW void dv_body_pictogram(int t,int v,int x,int y,float scale,float env,uint16_t col)
{ dv_body_scene(t,v,x,y,scale,env,col DV_NULL); }
#if !X0X_UI_SMALL_CANVAS || defined(DV_TEST_STOCK)
static UI_DRAW void draw_drum_voice(int band){dv_draw(band DV_NULL);}
#endif
static UI_DRAW void dv_glyph(pref_t r,float ev,float ox,float oy,float sx,float sy,uint16_t col)
{ dv_glyph_scene(r,ev,ox,oy,sx,sy,col DV_NULL); }

#if X0X_UI_SMALL_CANVAS
static int dv_capture(dv_scene *z)
{
    unsigned kit=ui.part==PART_808,v=ui.sel[ui.part],n=pg.n,page=cur_page();
    if(n>DV_ROWS || !n || page>=n || v>=NDRUM)return 0;
    int t=kit?T_808:T_909,clap=kit&&v==D8_CP;
    if(clap && n<2)return 0;
    for(unsigned p=0;p<n;p++)for(unsigned k=0;k<4;k++){
        pref_t r=pg.r[p][k];
        if(r.kind!=R_NONE && (r.kind!=R_ENG || r.a!=t || (r.b!=v && r.b!=NDRUM)))return 0;
    }
    z->kit=kit;z->voice=v;z->n=n;z->page=page;z->key_sound=key_sounds();
    z->ink=(dv_ink){{C_WHITE,SOUND_BASE,SOUND_FACE,SOUND_WHITE,SOUND_LINE,SOUND_GRAY,SOUND_INK,SOUND_DIM,SOUND_SELECT_SIDE,C_LINE},
        part_col(),ui_theme_light?SOUND_SELECT:RGB(36,48,54),C_BLACK};
    z->special.amp.valid=0;z->variant_decay_slot=255;
    for(unsigned p=0;p<n;p++){
        z->group[p]=pg.group[p];z->amp_row[p]=adsr_row(p);
        for(unsigned k=0;k<4;k++){
            pref_t r=pg.r[p][k];z->r[p][k]=r;
            if(r.kind==R_NONE)continue;
            int raw=matrix_value(r);
            z->value[p][k]=(uint8_t)raw;
            if(kit && r.b==v && (v==D8_CB||v==D8_CY||v==D8_OH||v==D8_CH) &&
                name_is(pref_desc(r),"Decay")){
                if(z->variant_decay_slot!=255)return 0;
                z->variant_decay_slot=r.c;
                dv_readout(z->variant_decay_value,r,raw);
            }
            /* The old clap painter advances only TUNE/TYPE/KIT ease. */
            if(!z->amp_row[p] && (!clap || (p==0&&(k==0||k==2)) || (p==1&&k==0)))
                z->eased[p][k]=ui_ease_float(r,raw);
        }
        if(z->amp_row[p])dv_capture_amp(&z->special.amp,z->r[p][0],z->value[p][0]);
    }
    if(!kit && v==DR_BD){
        z->kick[0]=(uint8_t)matrix_value(PR(R_ENG,T_909,DR_BD,0));
        z->kick[1]=(uint8_t)matrix_value(PR(R_ENG,T_909,DR_BD,4));
        z->kick[2]=(uint8_t)matrix_value(PR(R_ENG,T_909,DR_BD,5));
        z->pitch_ms=z->pitch_hz=0;
        z->pitch_valid=engine_bd909_pitch_view(&z->pitch_ms,&z->pitch_hz);
    }
    for(unsigned i=0;i<2;i++)
        z->live[i]=drum_live(PR(R_ENG,t,i?NDRUM:v,0),&z->age[i],&z->env[i]);
    z->back[0]=z->back[1]=1.f;
    unsigned back_seen=0;
    if(!clap)for(unsigned p=0;p<n;p++)for(unsigned k=0;k<4;k++) {
        pref_t r=z->r[p][k];
        int role=drum_strip_role_at(r,V_PART),i=r.b==NDRUM;
        if((role==DG_DECAY || role==DG_NTAIL || (role==DG_BDSWEEP && r.c==0)) && !(back_seen&(1u<<i))) {
            z->back[i]=dg_live_back(r,z->live[i]);back_seen|=1u<<i;
        }
    }
    if(clap){
        int choice=z->value[0][2];
        z->special.a=(drum_curve_t){.elapsed=-1};z->b=(drum_curve_t){.elapsed=-1};
        z->attack_owner=engine_drum808_param_curve_preview(D8_CP,3,choice,z->value[0][1],&z->special.a);
        z->decay_owner=engine_drum808_param_curve_preview(D8_CP,2,choice,z->value[0][3],&z->b);
    } else {
        z->b=(drum_curve_t){0};
        z->graph=kit?engine_drum808_curve(v,&z->b):dv_909_curve(v,&z->b);
    }
    const dpart_t *dp=&cur_pat()->drum[ui.part];
    unsigned first=(unsigned)ui.spage*16u;
    if(!clap && first>=NSTEPS)first=0;
    int len=dp->len,play=seq.playing ? eng_step[TRK_DRUM+ui.part]%(len?len:16) : -1;
    z->first=(uint16_t)first;z->hit=z->play=0;
    for(unsigned i=0;i<16;i++){
        unsigned x=first+i;
        int valid=x<NSTEPS&&x<(unsigned)len;
        if(valid&&sm_get(dp->hit[v],x))z->hit|=(uint16_t)(1u<<i);
        if((!clap||valid)&&(int)x==play)z->play|=(uint16_t)(1u<<i);
    }
    return 1;
}

static void dv_paint_frame(int band,const void *scene)
{ dv_draw(band,(const dv_scene *)scene); }
static UI_DRAW int draw_drum_frame(void)
{
    dv_scene z;
    if(!dv_capture(&z))return 0;
    int force=cv_claim_scene_footer();
    cv_draw_pair(z.ink.bg,&z,dv_paint_frame,0);
    cv_draw_scene_footer(z.ink.bg,&z,dv_paint_frame,force);
    return 1;
}
#endif
#undef DV
#undef DV_ARG
#undef DV_FORWARD
#undef DV_NULL
