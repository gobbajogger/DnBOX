static unsigned f2_extra_frames, f2_extra_findings;
static void f2_reset(void) {
 if(!f2_extra_enabled)return;
 geo_nframe=0;geo_ntile=0;presence_nglyph=0;f2_layer_active=f2_layer_generation=0;
 memset(pixel_frame_owner,0,sizeof pixel_frame_owner);
 geo_enabled=1;
}
static void f2_extra_audit(const char *screen) {
 if(!f2_extra_enabled)return;
 unsigned audited=0,findings=0,old_enabled=geo_enabled;
 geo_enabled=0;
 for(unsigned i=0;i<geo_nframe;i++) {
  geo_label e=geo_frame[i];unsigned expected=0,missing=0,covered=0;
  cv_begin(240,138,0);cv_text(10,0,e.f,e.text,65535);
  for(int y=0;y<e.f->h;y++)for(int x=0;x<240;x++)if(cv_px[y*240+x]==65535) {
   expected++;int sx=e.x+x-10,sy=e.y+y;
   if(getenv("F2_EXTRA_MUTATE") && sx>=0&&sx<240&&sy>=0&&sy<240)fb[sy*240+sx]^=65535;
   if(sx<0||sx>=240||sy<0||sy>=240||fb[sy*240+sx]!=swap16(e.col)) {
    missing++;
    if(f2_layer_active && e.layer==0 && sx>=f2_layer_rect[0] && sx<f2_layer_rect[2] && sy>=f2_layer_rect[1] && sy<f2_layer_rect[3])covered++;
   }
  }
  if(expected)audited++;
  if(missing)printf("F2_OCCLUSION {\"id\":\"%s\",\"text\":\"%s\",\"origin\":[%d,%d],\"labelLayer\":%u,\"active\":%u,\"rect\":[%d,%d,%d,%d],\"missing\":%u,\"covered\":%u,\"visibleMissing\":%u}\n",screen,e.text,e.x,e.y,e.layer,f2_layer_active,f2_layer_rect[0],f2_layer_rect[1],f2_layer_rect[2],f2_layer_rect[3],missing,covered,missing-covered);
  if(missing){findings++;printf("F2_EXTRA_FINDING screen=%s text=[%s] at=%d,%d missing=%u/%u\n",screen,e.text,e.x,e.y,missing,expected);}
 }
 printf("F2_EXTRA {\"frame\":%u,\"id\":\"%s\",\"labels\":%u,\"audited\":%u,\"findings\":%u,\"scope\":\"%s\"}\n",f2_extra_frames++,screen,geo_nframe,audited,findings,
  !strncmp(screen,"boot-",5)||!strncmp(screen,"hardware-",9)?"DISPLAY_SLICE_ONLY":"ACTUAL_HOST_COMPOSITION");
 f2_extra_findings+=findings;geo_enabled=old_enabled;
}
