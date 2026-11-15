/* Test-only native-host observer. Never included by guest/device firmware. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TR_MAX 40000
#define TR_SECTIONS 16
typedef struct {int k,a[9];char font[4],text[256];} tr_command;
typedef struct {int x,y,w,h,bg,n;tr_command *cmd;} tr_section;
static tr_command tr_cmd[TR_MAX];static tr_section tr_sections[TR_SECTIONS];
static int tr_n,tr_ns,tr_w,tr_h,tr_bg,tr_enabled,tr_skip;
static tr_command *tr_add(int k) {if(!tr_enabled || tr_skip)return 0;if(tr_n>=TR_MAX){fprintf(stderr,"trace overflow\n");exit(40);}tr_command *p=&tr_cmd[tr_n++];memset(p,0,sizeof *p);p->k=k;return p;}
static void tr_canvas(int w,int h,int bg){if(tr_enabled){tr_n=0;tr_w=w;tr_h=h;tr_bg=bg;}}
static void tr_rect(int x,int y,int w,int h,int c,int y0,int y1){tr_command *p=tr_add(0);if(p){int a[]={x,y,w,h,c,y0,y1};memcpy(p->a,a,sizeof a);}}
static void tr_line(int x,int y,int x1,int y1,int c,int cy0,int cy1){tr_command *p=tr_add(1);if(p){int a[]={x,y,x1,y1,c,cy0,cy1};memcpy(p->a,a,sizeof a);}}
static void tr_point(int x,int y,int c,int y0,int y1){tr_command *p=tr_add(2);if(p){int a[]={x,y,c,y0,y1};memcpy(p->a,a,sizeof a);}}
static void tr_blend(int x,int y,int c,int a,int y0,int y1){tr_command *p=tr_add(4);if(p){int args[]={x,y,c,a,y0,y1};memcpy(p->a,args,sizeof args);}}
static const char *tr_font(const felucca_font_t *f){if(f==&FONT_T)return "T";if(f==&FONT_XS)return "XS";if(f==&FONT_S)return "S";if(f==&FONT_B)return "B";if(f==&FONT_M)return "M";if(f==&FONT_L)return "L";fprintf(stderr,"unknown font\n");exit(41);}
static void tr_text(int x,int y,int c,const felucca_font_t *f,const char *s,int cl,int cr,int y0,int y1){tr_command *p=tr_add(3);if(p){int a[]={x,y,c,cl,cr,y0,y1};memcpy(p->a,a,sizeof a);if(strlen(s)>=sizeof p->text){fprintf(stderr,"trace text overflow\n");exit(42);}strcpy(p->text,s);strcpy(p->font,tr_font(f));}}
static void tr_commit(int x,int y,int r0){if(!tr_enabled || r0>=tr_h)return;if(r0){fprintf(stderr,"partial trace needs explicit support\n");exit(43);}if(tr_ns>=TR_SECTIONS)exit(44);tr_section *s=&tr_sections[tr_ns++];s->x=x;s->y=y;s->w=tr_w;s->h=tr_h;s->bg=tr_bg;s->n=tr_n;s->cmd=malloc(sizeof(tr_command)*tr_n);if(tr_n&&!s->cmd)exit(45);memcpy(s->cmd,tr_cmd,sizeof(tr_command)*tr_n);}
static void tr_reset(void){for(int i=0;i<tr_ns;i++)free(tr_sections[i].cmd);tr_ns=tr_n=tr_skip=0;}
static void tr_json_string(const char *s){putchar('"');for(;*s;s++){unsigned c=(unsigned char)*s;if(c=='"'||c=='\\')printf("\\%c",c);else if(c<32||c>=127)printf("\\u%04x",c);else putchar(c);}putchar('"');}
static void tr_json_sections(void){printf("\"sections\":[");for(int i=0;i<tr_ns;i++){tr_section *s=&tr_sections[i];if(i)putchar(',');printf("{\"x\":%d,\"y\":%d,\"w\":%d,\"h\":%d,\"background\":%d,\"commands\":[",s->x,s->y,s->w,s->h,s->bg);for(int j=0;j<s->n;j++){tr_command *p=&s->cmd[j];if(j)putchar(',');printf("[%d",p->k);if(p->k==3){for(int k=0;k<3;k++)printf(",%d",p->a[k]);putchar(',');tr_json_string(p->font);putchar(',');tr_json_string(p->text);for(int k=3;k<7;k++)printf(",%d",p->a[k]);}else{int n=p->k==0||p->k==1?7:p->k==2?5:6;for(int k=0;k<n;k++)printf(",%d",p->a[k]);}putchar(']');}printf("]}");}putchar(']');}
