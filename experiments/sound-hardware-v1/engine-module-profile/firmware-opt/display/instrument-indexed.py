from pathlib import Path
import re
p=Path(__file__).resolve().parent/'tree/firmware/src/gfx.c'
s=p.read_text()
anchor='static uint32_t cv_w, cv_h;'
assert s.count(anchor)==1
s=s.replace(anchor,anchor+r'''
#ifdef X0X_HOST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static uint32_t cvp_seen[138][2048],cvp_full[2048],cvp_count[138];
static unsigned cvp_frames,cvp_w,cvp_h,cvp_peak[4],cvp_rowmax,cvp_total,cvp_fullmax;
static uint8_t cvp_indices[240*138];
static uint16_t cvp_palette[256],cvp_reverse[65536]; /* reverse map host-only */
static unsigned cvp_pn,cvp_overflow,cvp_exhaustions,cvp_indexed_frames;

static void cvp_end(void){
 static const unsigned limit[]={8,16,32,64}; unsigned over[4]={0};
 if(cvp_w!=240 || cvp_h!=138)return;
 ++cvp_frames;
 for(unsigned r=0;r<cvp_h;r++){unsigned n=cvp_count[r];if(n>cvp_rowmax)cvp_rowmax=n;for(unsigned i=0;i<4;i++)over[i]+=n>limit[i];}
 for(unsigned i=0;i<4;i++)if(over[i]>cvp_peak[i])cvp_peak[i]=over[i];
 if(cvp_total>cvp_fullmax)cvp_fullmax=cvp_total;
 /* End reads the exact original canvas before it can be filled again. */
 if(cvp_overflow){++cvp_exhaustions;return;}
 for(unsigned px=0;px<240u*138u;px++)
     if(cvp_palette[cvp_indices[px]]!=cv_px[px]) abort();
 ++cvp_indexed_frames;

}
static void cvp_report(void){cvp_end();fprintf(stderr,"CVP frames=%u rowmax=%u globalmax=%u raw-rows(8,16,32,64)=%u/%u/%u/%u\n",cvp_frames,cvp_rowmax,cvp_fullmax,cvp_peak[0],cvp_peak[1],cvp_peak[2],cvp_peak[3]);
 fprintf(stderr,"CVP index_frames=%u overflow_frames=%u canvas_and_palette_B=%zu\n",cvp_indexed_frames,cvp_exhaustions,sizeof cvp_indices+sizeof cvp_palette);
}
static void cvp_begin(unsigned w,unsigned h,uint16_t v){
 static int registered;if(!registered){registered=1;atexit(cvp_report);}cvp_end();cvp_w=w;cvp_h=h;
 if(w!=240||h!=138)return;
 memset(cvp_seen,0,sizeof cvp_seen);memset(cvp_full,0,sizeof cvp_full);
 cvp_full[v>>5]=1u<<(v&31);cvp_total=1;
 memset(cvp_indices,0,sizeof cvp_indices);
 memset(cvp_reverse,0xff,sizeof cvp_reverse);
 cvp_pn=1;cvp_overflow=0;cvp_palette[0]=v;cvp_reverse[v]=0;

 for(unsigned r=0;r<h;r++){cvp_count[r]=1;cvp_seen[r][v>>5]=1u<<(v&31);}
}
static void cvp_store(uint16_t *p,uint16_t v){
 if(cvp_w==240&&cvp_h==138){
  unsigned r=(unsigned)(p-cv_px)/240u,m=1u<<(v&31);uint32_t *q=&cvp_seen[r][v>>5];
  if(!(*q&m)){*q|=m;++cvp_count[r];}
  q=&cvp_full[v>>5];if(!(*q&m)){*q|=m;++cvp_total;}
  if(!cvp_overflow){
   unsigned idx=cvp_reverse[v];
   if(idx==65535u){
    if(cvp_pn==256u)cvp_overflow=1;
    else{idx=cvp_pn++;cvp_palette[idx]=v;cvp_reverse[v]=idx;}
   }
   if(!cvp_overflow)cvp_indices[p-cv_px]=(uint8_t)idx;
  }

 }
 *p=v;
}
#else
#define cvp_begin(w,h,v) ((void)0)
#define cvp_store(p,v) (*(p)=(v))
#endif
''')
q='    cv_h = h;'
assert s.count(q)==1
s=s.replace(q,q+'\n    cvp_begin(w,h,(uint16_t)fill);')
i=s.index('static inline void cv_pset')
body,n=re.subn(r'(?m)^(\s*)(cv_px\[.*?\]) = (.*);$',r'\1cvp_store(&\2, \3);',s[i:])
assert n==4,n
s=s[:i]+body
j=s.index('static void px_blend'); k=s.index('static int cv_abs',j)
body,n=re.subn(r'(?ms)^(\s*)\*q = (.*?);',r'\1cvp_store(q, \2);',s[j:k])
assert n==2,n
p.write_text(s[:j]+body+s[k:])
