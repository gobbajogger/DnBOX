/* Passive text provenance only. Never changes the displayed framebuffer. */
static geo_label visible_labels[8192];
static unsigned visible_count;
static int32_t text_w(const felucca_font_t *f,const char *s);
static void cv_begin(uint32_t w,uint32_t h,uint16_t bg);
static int32_t cv_text(int32_t x,int32_t y,const felucca_font_t *f,const char *s,uint16_t c);
static const char *visible_stale(const char *s) {
 if(!f2_extra_enabled||!geo_enabled||!getenv("VISIBLE_MUTATE_STALE"))return s;
 static const char *const changed[][2]={
  {"PICKUP: UP","PICKUP: MOVE UP TO TARGET"},{"PICKUP: DOWN","PICKUP: MOVE DOWN TO TARGET"},
  {"STARTUP CRASHES: 2","IT CRASHED TWICE"},{"AUDIO: OFF    USB: ON","NO SOUND. USB IS ON:"},
  {"WEB INSTALLER: REINSTALL","REINSTALL FROM THE"},{"PLAY: RETRY","PLAY: TRY AGAIN"},
  {"STARTING DnBOX...","PLAY: TRY AGAIN"},{"CRASH: CORE 1","RESTARTED AFTER A CRASH"},
  {"CRASH: CORE 2","THE SECOND CORE CRASHED"},{"HARDWARE CALIBRATION","TEACH EACH BUTTON AND KNOB"},
  {"PC 12345678","PC 00000000"},{"12345678","00000000"},
  {"1 V01 D00001000","1 V00 D00000000"},{"2 V01 D00001000","2 V00 D00000000"},
  {"VELOCITY","VOLUME"},{"CHANCE","CHNCE"},{"PITCH","P.DPTH"},{"PAN","BAL"}};
 for(unsigned i=0;i<sizeof changed/sizeof changed[0];i++)if(!strcmp(s,changed[i][0]))return changed[i][1];
 return s;
}
static int visible_missing_target(const char *screen,const char *text) {
 if(!getenv("VISIBLE_MUTATE_MISSING"))return 0;
 if(!strncmp(screen,"hardware-calibration-",21))return !strcmp(text,"HARDWARE CALIBRATION")||!strcmp(text,"PRESS")||!strcmp(text,"TURN RIGHT");
 if(!strncmp(screen,"boot-safe",9)||!strncmp(screen,"boot-post-crash",15)||!strcmp(screen,"boot-crash-diagnostics"))return 1;
 if(!strcmp(screen,"smc-explicit-peek-pickup"))return !strncmp(text,"PICKUP:",7);
 if(!strncmp(screen,"edited-part",11))return !strcmp(text,"VELOCITY")||!strcmp(text,"CHANCE")||!strcmp(text,"PITCH")||!strcmp(text,"PAN");
 return 0;
}
static void visible_clear(int x,int y,int w,int h) {
 if(!f2_extra_enabled)return;
 if(x<=0&&y<=0&&x+w>=240&&y+h>=240){visible_count=0;return;}
 unsigned kept=0;
 for(unsigned i=0;i<visible_count;i++) {
  geo_label e=visible_labels[i];int left=e.x-2*e.f->pad,right=e.x+text_w(e.f,e.text)+2*e.f->pad;
  if(left>=x&&e.y>=y&&right<=x+w&&e.y+e.f->h<=y+h)continue;
  visible_labels[kept++]=e;
 }
 visible_count=kept;
}
static void visible_commit(int x,int y) {
 if(!f2_extra_enabled)return;
 visible_clear(x,y,cv_w,cv_h);
 for(unsigned i=0;i<geo_ntile;i++) {
  geo_label e=geo_tile[i];e.x+=x;e.y+=y;
  int duplicate=0;
  for(unsigned j=0;j<visible_count;j++) {
   geo_label *old=&visible_labels[j];
   if(old->f==e.f&&old->x==e.x&&old->y==e.y&&old->col==e.col&&!strcmp(old->text,e.text)){*old=e;duplicate=1;break;}
  }
  if(duplicate)continue;
  if(visible_count>=8192){fprintf(stderr,"visible text ledger overflow\n");exit(1);}
  visible_labels[visible_count++]=e;
 }
}
static void visible_text_dump(const char *screen) {
 if(!f2_extra_enabled)return;
 unsigned old_enabled=geo_enabled,fully=0,partial=0;geo_enabled=0;
 for(unsigned i=0;i<visible_count;i++) {
  geo_label e=visible_labels[i];unsigned expected=0,matched=0;
  cv_begin(240,138,0);cv_text(10,0,e.f,e.text,65535);
  if(visible_missing_target(screen,e.text)) {
   for(int y=0;y<e.f->h;y++)for(int x=0;x<240;x++)if(cv_px[y*240+x]==65535) {
    int sx=e.x+x-10,sy=e.y+y;if(sx>=0&&sx<240&&sy>=0&&sy<240)fb[sy*240+sx]^=65535;
   }
   printf("VISIBLE_MISSING_MUTANT case=%s\n",screen);
  }
  for(int y=0;y<e.f->h;y++)for(int x=0;x<240;x++)if(cv_px[y*240+x]==65535) {
   expected++;int sx=e.x+x-10,sy=e.y+y;
   if(sx>=0&&sx<240&&sy>=0&&sy<240&&fb[sy*240+sx]==swap16(e.col))matched++;
  }
  if(!expected||!matched)continue;
  if(matched==expected)fully++;else partial++;
  printf("VISIBLE_TEXT {\"id\":\"%s\",\"origin\":[%d,%d],\"fontHeight\":%u,\"color565\":%u,\"expectedOpaque\":%u,\"matchedOpaque\":%u,\"fullyVisible\":%s,\"textHex\":\"",screen,e.x,e.y,e.f->h,e.col,expected,matched,matched==expected?"true":"false");
  for(const unsigned char *p=(const unsigned char *)e.text;*p;p++)printf("%02x",*p);
  puts("\"}");
 }
 printf("VISIBLE_FRAME {\"id\":\"%s\",\"fullyVisibleLabels\":%u,\"partialLabels\":%u,\"ledgerLabels\":%u}\n",screen,fully,partial,visible_count);
 geo_enabled=old_enabled;
}
