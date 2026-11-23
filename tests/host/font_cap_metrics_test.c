/* SPDX-License-Identifier: GPL-3.0-only */
/* Independently measure generated cap metadata through the production painter. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void lcd_sync(void) {}
static void lcd_blit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t *pixels)
{(void)x;(void)y;(void)w;(void)h;(void)pixels;}
#ifndef GFX_SOURCE
#define GFX_SOURCE "../../firmware/src/gfx.c"
#endif
#ifdef __APPLE__
#define __attribute__(x)
#endif
#include GFX_SOURCE
#ifdef __APPLE__
#undef __attribute__
#endif
_Static_assert(_Generic(((capm_t *)0)->top,int8_t:1,default:0),"cap top must be int8_t");
_Static_assert(_Generic(((capm_t *)0)->bot,int8_t:1,default:0),"cap bottom must be int8_t");
static uint64_t raster_hash=UINT64_C(1469598103934665603);
static void hash_canvas(void)
{
    for(unsigned i=0;i<cv_w*cv_h;i++) {
        raster_hash^=cv_px[i];raster_hash*=UINT64_C(1099511628211);
    }
}
static int check_face(const felucca_font_t *face,const char *role,int damage)
{
    felucca_font_t changed=*face;
    if(damage==1) {changed.cap.top++;face=&changed;}
    if(damage==2) {changed.cap.bot--;face=&changed;}
    /* An absent or deliberately width-zero H cannot define the cap line. */
    int ch='H';
    if(ch<face->first || ch>face->last || !font_bitmap_width(face,(unsigned)(ch-face->first)))ch='0';
    if(ch<face->first || ch>face->last || !font_bitmap_width(face,(unsigned)(ch-face->first))) {
        fprintf(stderr,"CAP_REFERENCE_MISSING role=%s\n",role);return 1;
    }
    char text[2]={(char)ch,0};const int origin_y=16;
    cv_begin(128,80,0);cv_text(32,origin_y,face,text,RGB(255,255,255));
    int top=80,bottom=-1;
    for(unsigned y=0;y<cv_h;y++)for(unsigned x=0;x<cv_w;x++) {
        uint16_t pixel=swap16(cv_px[y*cv_w+x]);
        /* White on literal black: alpha>=8 iff green is at least half scale.
         * This reads final RGB565 pixels, never font bytes or crop metadata. */
        if(((pixel>>5)&63u)>=32u) {
            if((int)y<top)top=(int)y;
            if((int)y>bottom)bottom=(int)y;
        }
    }
    if(bottom<0 || face->cap.top!=top-origin_y || face->cap.bot!=bottom-origin_y) {
        fprintf(stderr,"CAP_METRIC_MISMATCH role=%s ref=%c generated=%d,%d rendered=%d,%d\n",
                role,ch,face->cap.top,face->cap.bot,top-origin_y,bottom-origin_y);return 1;
    }
    assert(top>=origin_y && bottom<origin_y+face->h);hash_canvas();
    unsigned zero_width=0,glyph_cases=0;
    /* Exercise retained blanks and clipped raster decoding under sanitizers.
     * L has no '?' fallback, so callers stay inside its declared range. */
    for(unsigned code=face->first;code<=face->last;code++) {
        unsigned gi=glyph(face,code),width=font_bitmap_width(face,gi);
        zero_width+=width==0;
        char one[2]={(char)code,0};
        for(unsigned variant=0;variant<4;variant++) {
            int x=variant==1?-8:variant==2?125:16;
            int y=variant==3?-12:2;
            cv_begin(128,80,0);cv_text(x,y,face,one,RGB(255,255,255));
            if(!width)for(unsigned p=0;p<cv_w*cv_h;p++)assert(!cv_px[p]);
            hash_canvas();glyph_cases++;
        }
    }
    printf("CAP_FACE role=%s ref=%c top=%d bot=%d width_zero=%u glyph_cases=%u\n",
           role,ch,face->cap.top,face->cap.bot,zero_width,glyph_cases);
    return 0;
}
int main(int argc,char **argv)
{
    const felucca_font_t *faces[]={&FONT_T,&FONT_XS,&FONT_S,&FONT_B,&FONT_M,&FONT_L};
    const char *roles[]={"T","XS","S","B","M","L"};
    int damage=argc>1&&!strcmp(argv[1],"--negative-top")?1:argc>1&&!strcmp(argv[1],"--negative-bottom")?2:0;
    for(unsigned role=0;role<6;role++)if(check_face(faces[role],roles[role],damage&&role==0?damage:0))return 1;
    printf("PASS rendered cap metadata faces=6 raster_hash=%016llx\n",(unsigned long long)raster_hash);
    return 0;
}
