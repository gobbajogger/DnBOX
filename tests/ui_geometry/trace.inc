static unsigned presence_nglyph,presence_glyph[256][4];
typedef struct { const felucca_font_t *f; char text[128]; int x,y; unsigned col; int value; unsigned ref; } geo_label;
/* #62: text drawn inside a value formatter (1 cell_value, 2 strip_value, 3 strip_value_fit, 4 dv_value_text), minus its unit/caption */
static int geo_value_fn;static const char *geo_skip_a,*geo_skip_b;
/* the control a value formatter is drawing (1+kind/a/b/c), from the last pref_value_of */
static unsigned geo_last_ref,geo_value_ref;
static geo_label geo_tile[256],geo_frame[1024];
static unsigned geo_enabled,geo_findings,geo_ntile,geo_nframe;
/* Raster delta technique reused from tests/ui_defects/first_slice/trace_gfx.inc.
 * Owner masks are translated by actual blits then checked against final pixels. */
static uint32_t pixel_tile_owner[CV_MAX],pixel_frame_owner[240*240];
static uint16_t pixel_tile_color[CV_MAX],pixel_frame_color[240*240];
static void pixel_snapshot(uint16_t *before) {if(geo_enabled)memcpy(before,cv_px,cv_w*cv_h*sizeof *before);}
static void pixel_finish(const uint16_t *before,uint32_t owner) {
 if(!geo_enabled)return;
 for(int y=cv_y0;y<cv_y1;y++)for(unsigned x=0;x<cv_w;x++)if(before[y*cv_w+x]!=cv_px[y*cv_w+x]){
  pixel_tile_owner[y*cv_w+x]=owner;pixel_tile_color[y*cv_w+x]=cv_px[y*cv_w+x];
 }
}

static void geo_blit(int x,int y) {
 if(!geo_enabled)return;
 for(unsigned yy=0;yy<cv_h;yy++)for(unsigned xx=0;xx<cv_w;xx++)if(pixel_tile_owner[yy*cv_w+xx]) {
  int sx=x+(int)xx,sy=y+(int)yy;if(sx<0||sx>=240||sy<0||sy>=240)continue;
  pixel_frame_owner[sy*240+sx]=pixel_tile_owner[yy*cv_w+xx];pixel_frame_color[sy*240+sx]=pixel_tile_color[yy*cv_w+xx];
 }
 for(unsigned i=0;i<geo_ntile;i++) {
  if(geo_nframe>=1024){fprintf(stderr,"geometry label overflow\n");exit(1);}
  geo_frame[geo_nframe]=geo_tile[i];geo_frame[geo_nframe].x+=x;geo_frame[geo_nframe].y+=y;geo_nframe++;
 }
}


static unsigned pan_format_checks,pan_format_bad;
static unsigned r22_roll_bands,r22_roll_screens,r22_roll_missing;
/* Row 7 units: values formatted with a unit during the audited frame. */
typedef struct { char num[16],unit[8]; unsigned ref; } unit_fmt;
static unit_fmt unit_seen[512];static unsigned unit_nseen,unit_checks,unit_missing;
/* Page compliance: raw 0..127 readouts, unit spelling, header contract. */
typedef struct { char num[16],name[16]; unsigned ref; int raw; } raw_fmt;
static raw_fmt raw_seen[512];static unsigned raw_nseen,raw_checks,raw_bad;
static unsigned unitname_checks,unitname_bad,header_checks,header_bad;
