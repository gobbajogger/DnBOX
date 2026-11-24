/* SPDX-License-Identifier: GPL-3.0-only */
static unsigned audit_pg,audit_field,audit_raw,audit_call,audit_draws,audit_clipped,audit_overlap,audit_overrun;
static int audit_label_right;
static int32_t audit_text(int32_t x,int32_t y,const felucca_font_t*f,const char*s,uint16_t c) {
 int left=999,right=-999,pen=x; unsigned clipped=0;
 for(const unsigned char*p=(const unsigned char*)s;*p;p++) {
  unsigned ch=*p;if(ch<f->first||ch>f->last)ch='?';unsigned gi=ch-f->first,w=font_bitmap_width(f,gi),stride=(w+1)/2;
  const uint8_t*d=f->data+font_offset(f,gi);
  for(unsigned yy=0;yy<f->h;yy++)for(unsigned xx=0;xx<w;xx++) {
   unsigned a=d[yy*stride+xx/2];a=(xx&1)?a&15:a>>4;if(!a)continue;
   int gx=pen-(int)f->pad+xx,gy=y+yy;
   if(gx<left)left=gx;if(gx>right)right=gx;
   if(gx<0||gx>=(int)cv_w||gy<0||gy>=(int)cv_h)clipped++;
  }pen+=font_advance(f,gi);
 }
 unsigned value=audit_call%2,overlap=value&&left<=audit_label_right;
 unsigned cell_right=(audit_call/2<5)?119:239,overrun=right>(int)cell_right;
 if(!value)audit_label_right=right;
 audit_draws++;audit_call++;audit_clipped+=clipped;audit_overlap+=overlap;audit_overrun+=overrun;
 printf("TEXT pg=%u selected=%u raw=%u role=%s left=%d right=%d clipped=%u overlap=%u overrun=%u text=%s\n",audit_pg,audit_field,audit_raw,value?"value":"label",left,right,clipped,overlap,overrun,s);
 return cv_text(x,y,f,s,c);
}
#define cv_text audit_text
