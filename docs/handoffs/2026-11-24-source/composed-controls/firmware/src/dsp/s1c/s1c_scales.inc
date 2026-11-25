/* SPDX-License-Identifier: GPL-3.0-only */
/* Source-derived ratios in catalog order; octave-reduced and sorted exactly
 * as AudioKit's AKTuningTable.tuningTable(fromFrequencies:) does. */
typedef struct {const char *name;uint8_t npo,offset;} s1_scale_entry;
static const s1_scale_entry s1_scale[S1C_SCALE_COUNT]={
    {"12ET",12,0},
    {"Hobbs 12",12,0}, /* 12 Hobbs Recurrence Relation */
    {"Narushima",19,12}, /* 19 Grady: A-Narushima 19-tone 7-limit */
    {"Triad",5,31}, /* 3 Harmonic+Subharmonic Series: Triad */
    {"Grady Penta",5,36}, /* 5 Grady: S Pentatonic 11-limit Scale 2 */
    {"Pentad",5,41}, /* 5 Harmonic Series: Pentad */
    {"Hexany 1",6,46}, /* 6 Wilson Hexany(1, 3, 5, 7) */
    {"Hexany 3",6,52}, /* 6 Wilson Hexany(3, 5, 15, 27) */
    {"Bagpipes",7,58}, /* 7 Wilson Highland Bagpipes */
    {"MOS G",7,65}, /* 7 Wilson MOS G:0.2641 */
    {"Bhairavi",7,72}, /* 7 Wilson North Indian:Bhairavi */
    {"Madhubanti",7,79}, /* 7 Wilson North Indian:Madhubanti */
    {"Dyad",3,86}, /* Harmonic+Subharmonic Series: Dyad */
    {"31ET",31,89},
};
static const float s1_scale_ratio[]={
    0x1.0000000000000p+0f,0x1.0400000000000p+0f,0x1.2000000000000p+0f,0x1.2800000000000p+0f,0x1.2e00000000000p+0f,
    0x1.5000000000000p+0f,0x1.5800000000000p+0f,0x1.8000000000000p+0f,0x1.8800000000000p+0f,0x1.9000000000000p+0f,
    0x1.c000000000000p+0f,0x1.c800000000000p+0f,0x1.0000000000000p+0f,0x1.0cccccccccccdp+0f,0x1.1800000000000p+0f,
    0x1.2000000000000p+0f,0x1.2aaaaaaaaaaacp+0f,0x1.3333333333333p+0f,0x1.4000000000000p+0f,0x1.5000000000000p+0f,
    0x1.5555555555554p+0f,0x1.6666666666666p+0f,0x1.7555555555554p+0f,0x1.8000000000000p+0f,0x1.8e38e38e38e3bp+0f,
    0x1.999999999999ap+0f,0x1.aaaaaaaaaaaacp+0f,0x1.c000000000000p+0f,0x1.ccccccccccccdp+0f,0x1.e000000000000p+0f,
    0x1.f800000000000p+0f,0x1.0000000000000p+0f,0x1.3333333333333p+0f,0x1.5555555555555p+0f,0x1.8000000000000p+0f,
    0x1.aaaaaaaaaaaabp+0f,0x1.0000000000000p+0f,0x1.4000000000000p+0f,0x1.6000000000000p+0f,0x1.8000000000000p+0f,
    0x1.c000000000000p+0f,0x1.0000000000000p+0f,0x1.3333333333333p+0f,0x1.6666666666666p+0f,0x1.999999999999ap+0f,
    0x1.ccccccccccccdp+0f,0x1.1800000000000p+0f,0x1.4000000000000p+0f,0x1.5000000000000p+0f,0x1.8000000000000p+0f,
    0x1.c000000000000p+0f,0x1.e000000000000p+0f,0x1.0e00000000000p+0f,0x1.2c00000000000p+0f,0x1.4400000000000p+0f,
    0x1.6800000000000p+0f,0x1.9500000000000p+0f,0x1.e000000000000p+0f,0x1.0000000000000p+0f,0x1.2000000000000p+0f,
    0x1.3800000000000p+0f,0x1.5600000000000p+0f,0x1.8000000000000p+0f,0x1.a000000000000p+0f,0x1.c800000000000p+0f,
    0x1.0000000000000p+0f,0x1.0a34c524fd01bp+0f,0x1.336d4ef2e24d0p+0f,0x1.3faef2fbf16edp+0f,0x1.712f5a06ddf28p+0f,
    0x1.7fe74d7833d0fp+0f,0x1.bb596bfa43a24p+0f,0x1.0000000000000p+0f,0x1.0e00000000000p+0f,0x1.2fc0000000000p+0f,
    0x1.5555555555555p+0f,0x1.8000000000000p+0f,0x1.9500000000000p+0f,0x1.c71c71c71c71cp+0f,0x1.0000000000000p+0f,
    0x1.2000000000000p+0f,0x1.2fc0000000000p+0f,0x1.6800000000000p+0f,0x1.8000000000000p+0f,0x1.aaaaaaaaaaaacp+0f,
    0x1.e000000000000p+0f,0x1.0000000000000p+0f,0x1.5555555555555p+0f,0x1.8000000000000p+0f,0x1.0000000000000p+0f,
    0x1.05c9dcf06c4d8p+0f,0x1.0bb53baeed0e4p+0f,0x1.11c2de305e7b8p+0f,0x1.17f38acc55c11p+0f,0x1.1e480c5683e67p+0f,
    0x1.24c13238abaa8p+0f,0x1.2b5fd08d2da59p+0f,0x1.3224c03a2e169p+0f,0x1.3910df0d57d5ap+0f,0x1.40250fd83ff94p+0f,
    0x1.47623a8d6dd0ap+0f,0x1.4ec94c5e0ae9fp+0f,0x1.565b37d83ef1dp+0f,0x1.5e18f5063b4e5p+0f,0x1.6603818dfa6f1p+0f,
    0x1.6e1be0d1b6e23p+0f,0x1.76631c111e56dp+0f,0x1.7eda428b44cb9p+0f,0x1.878269a15c42bp+0f,0x1.905cacfa356cbp+0f,
    0x1.996a2ea68dd54p+0f,0x1.a2ac174630378p+0f,0x1.ac23962debb9fp+0f,0x1.b5d1e18e66ef9p+0f,0x1.bfb8369bd3941p+0f,
    0x1.c9d7d9b688198p+0f,0x1.d432169484399p+0f,0x1.dec8406be5da3p+0f,0x1.e99bb21e53b58p+0f,0x1.f4adce656352dp+0f,
};
