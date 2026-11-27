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
static void cvp_end(void){
 static const unsigned limit[]={8,16,32,64}; unsigned over[4]={0};
 if(cvp_w!=240 || cvp_h!=138)return;
 ++cvp_frames;
 for(unsigned r=0;r<cvp_h;r++){unsigned n=cvp_count[r];if(n>cvp_rowmax)cvp_rowmax=n;for(unsigned i=0;i<4;i++)over[i]+=n>limit[i];}
 for(unsigned i=0;i<4;i++)if(over[i]>cvp_peak[i])cvp_peak[i]=over[i];
 if(cvp_total>cvp_fullmax)cvp_fullmax=cvp_total;
}
static void cvp_report(void){cvp_end();fprintf(stderr,"CVP frames=%u rowmax=%u globalmax=%u raw-rows(8,16,32,64)=%u/%u/%u/%u\n",cvp_frames,cvp_rowmax,cvp_fullmax,cvp_peak[0],cvp_peak[1],cvp_peak[2],cvp_peak[3]);}
static void cvp_begin(unsigned w,unsigned h,uint16_t v){
 static int registered;if(!registered){registered=1;atexit(cvp_report);}cvp_end();cvp_w=w;cvp_h=h;
 if(w!=240||h!=138)return;
 memset(cvp_seen,0,sizeof cvp_seen);memset(cvp_full,0,sizeof cvp_full);
 cvp_full[v>>5]=1u<<(v&31);cvp_total=1;
 for(unsigned r=0;r<h;r++){cvp_count[r]=1;cvp_seen[r][v>>5]=1u<<(v&31);}
}
static void cvp_store(uint16_t *p,uint16_t v){
 if(cvp_w==240&&cvp_h==138){
  unsigned r=(unsigned)(p-cv_px)/240u,m=1u<<(v&31);uint32_t *q=&cvp_seen[r][v>>5];
  if(!(*q&m)){*q|=m;++cvp_count[r];}
  q=&cvp_full[v>>5];if(!(*q&m)){*q|=m;++cvp_total;}
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
