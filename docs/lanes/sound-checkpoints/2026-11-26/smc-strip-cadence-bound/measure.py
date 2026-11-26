from pathlib import Path
import tempfile,subprocess
src=Path('../ui-canvas-clip-wired/tree/firmware/src/app/ui.c').read_text()
f=src[src.index('static void cv_lcd_maybe_input(void)'):src.index('typedef struct { uint32_t x, y; } cv_lcd_pos_t;')]
f=f.replace('cv_lcd_maybe_input','dispatch')
fixture=r'''
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <stddef.h>
static uint32_t ms;
static int active,queued,ack;
static uint32_t plat_ms(void){return ms;}
static void cv_lcd_input(void){ if(queued) active=1,queued=0; ++ack; }
struct button {const char *caption;uint16_t ink;uint8_t symbol, led;};
struct strip {struct button b[4];char title[24],knob_label[24],knob_value[32],fader_value[32];
uint16_t inks[5]; uint8_t on[8];};
struct snapshot {struct strip row[8];uint16_t colors[4];uint8_t flags[4];};
'''
main=r'''
#define STOCK 0
#define STRIP 1
static int run(int split) {
 ms=100;active=queued=ack=0;
 /* Header dispatch; then a grid tile. The first two grid tiles share a tick. */
 dispatch();
 ms=101; dispatch();
 queued=1;
 if (split) ms++;  /* Extra second paint can cross a tick, stock already rasterized. */
 dispatch();
 /* Original caller evaluates knob/FILL after grid when resolved above. */
 return active;
}
int main(void){
 assert(run(STOCK)==0);
 ms=99;dispatch(); /* reset driver's static last_ms at another hardware tick */
 assert(run(STRIP)==1);
 printf("Same tile ordinal, same queue schedule, legal tick crossing: different state at downstream read.\n");
 printf("Explicit resolved snapshot witness layout: address width %zu, size %zu.\n",sizeof(void *),sizeof(struct snapshot));
}
'''
with tempfile.TemporaryDirectory() as D:
 p=Path(D);(p/'w.c').write_text(fixture+f+main)
 subprocess.run(['cc','-std=c11','-O2','-Wall','-Wextra','-Werror',str(p/'w.c'),'-o',str(p/'w')],check=True)
 subprocess.run([str(p/'w')],check=True)
