/* SPDX-License-Identifier: GPL-3.0-only
 * Finite native-domain specialization of canonical G-FINAL CHANCE.
 * Table generated with (v*127+50)/100 integer normalization followed by
 * IEEE binary64 q/127, q*6,+.5,truncation in that order. No native floating math.
 * Same-role Q4 border union: corners15/16, other border16/16; pips16/16.
 */
#include "ui_chance_exact.h"
static const uint8_t native_states[101]={
0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6
};
int ui_chance_prepare_native(int value,uint8_t *prepared)
{
    if (!prepared) return -3;
    *prepared=255;
    if (value<0 || value>100) return -3;
    *prepared=native_states[value];
    return 0;
}
/* Blend before RGB565 quantization, exact canonical +8 rounding. */
#define COVER(c,b,a) (((b)*(16-(a))+(c)*(a)+8)>>4)
#define PACK(r,g,b) (((r)>>3)<<11|((g)>>2)<<5|((b)>>3))
#define INK565 PACK(255,186,65)
#define CORNER565 PACK(COVER(255,16,15),COVER(186,24,15),COVER(65,24,15))
static void pip(uint16_t *out,int row,int x,int y)
{ if(row>=y && row<y+2){out[x]=INK565;out[x+1]=INK565;} }
int ui_chance_exact_row(uint8_t n,int row,uint16_t out[26])
{
    int x;
    if(n>6 || row<0 || row>=19 || !out) return -3;
    for(x=0;x<26;++x){
        int border=((row==3||row==15)&&x>=6&&x<=18)||((x==6||x==18)&&row>=3&&row<=15);
        int corner=(row==3||row==15)&&(x==6||x==18);
        out[x]=border?(corner?CORNER565:INK565):0x10c3;
    }
    if(n==1||n==3||n==5)pip(out,row,11,8);
    if(n>=2){pip(out,row,8,5);pip(out,row,14,11);}
    if(n>=4){pip(out,row,14,5);pip(out,row,8,11);}
    if(n==6){pip(out,row,8,8);pip(out,row,14,8);}
    return 0;
}
