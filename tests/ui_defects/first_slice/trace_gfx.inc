/* SPDX-License-Identifier: GPL-3.0-only
 * Test-only hooks copied into an isolated instrumented tree. Never product code. */
static int first_enabled,first_offset,first_row=-1,first_control=-1,first_value;
static int first_ref[4];
static const char *first_name="", *first_glyph_type="unclassified";
static unsigned char first_mask[240*240];
static void first_json_string(const char *s) {
    putchar('"');for(;*s;s++){unsigned char c=(unsigned char)*s;if(c=='"'||c=='\\'){putchar('\\');putchar(c);}else if(c<32)printf("\\u%04x",c);else putchar(c);}putchar('"');
}
static void first_snapshot(uint16_t *before) { if(first_enabled)memcpy(before,cv_px,cv_w*cv_h*sizeof *before); }
static void first_finish(const uint16_t *before,const char *kind,float ox,float oy,const char *text) {
    if(!first_enabled)return;
    int x0=240,y0=240,x1=-1,y1=-1,n=0;
    int row_base=0;
#if X0X_UI_SMALL_CANVAS
    row_base=(int)cv_row_base;
#endif
    for(int y=cv_y0;y<cv_y1;y++)for(unsigned x=0;x<cv_w;x++)if(before[(y-row_base)*cv_w+x]!=cv_px[(y-row_base)*cv_w+x]){
        int sy=y+first_offset;if(sy<0||sy>=240)continue;
        first_mask[sy*240+x]=1;n++;if((int)x<x0)x0=(int)x;if((int)x>x1)x1=(int)x;if(sy<y0)y0=sy;if(sy>y1)y1=sy;
    }
    printf("FIRST:{\"kind\":");first_json_string(kind);
    printf(",\"row\":%d,\"control\":%d,\"ref\":\"%d:%d:%d:%d\",\"name\":",first_row,first_control,first_ref[0],first_ref[1],first_ref[2],first_ref[3]);first_json_string(first_name);
    printf(",\"glyphType\":");first_json_string(first_glyph_type);
    printf(",\"origin\":[%.3f,%.3f],\"pixels\":%d,\"bbox\":",ox,oy+cv_oy+first_offset,n);
    if(n)printf("[%d,%d,%d,%d]",x0,y0,x1+1,y1+1);else printf("null");
    if(text){printf(",\"text\":");first_json_string(text);}puts("}");
}
static void first_panel(const char *id,int x,int y,int w,int h) {
    if(!first_enabled)return;
    printf("FIRST:{\"kind\":\"panel\",\"id\":");first_json_string(id);
    printf(",\"bbox\":[%d,%d,%d,%d]}\n",x,y+cv_oy+first_offset,x+w,y+h+cv_oy+first_offset);
}
/* Independent final-frame caption check: opaque pixels in the actual native
 * font mask must match the final framebuffer, not an incremental-band delta. */
typedef struct {char text[96];const felucca_font_t *font;int x,y,row,control;uint16_t color;} first_label_t;
static first_label_t first_labels[512];static unsigned first_nlabels;
static void first_record_label(int x,int y,const felucca_font_t *f,const char *s,uint16_t col) {
    if(!first_enabled||first_value)return;
    if(first_nlabels>=512){fprintf(stderr,"first slice label overflow\n");exit(2);}
    first_label_t *e=&first_labels[first_nlabels++];snprintf(e->text,sizeof e->text,"%s",s);
    e->font=f;e->x=x;e->y=y+cv_oy+first_offset;e->row=first_row;e->control=first_control;e->color=col;
}
