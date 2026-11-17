typedef struct { const felucca_font_t *f; char text[128]; int x,y; unsigned col; } geo_label;
static geo_label geo_tile[256],geo_frame[1024];
static unsigned geo_enabled,geo_findings,geo_ntile,geo_nframe;
static void geo_blit(int x,int y) {
 if(!geo_enabled)return;
 for(unsigned i=0;i<geo_ntile;i++) {
  if(geo_nframe>=1024){fprintf(stderr,"geometry label overflow\n");exit(1);}
  geo_frame[geo_nframe]=geo_tile[i];geo_frame[geo_nframe].x+=x;geo_frame[geo_nframe].y+=y;geo_nframe++;
 }
}

