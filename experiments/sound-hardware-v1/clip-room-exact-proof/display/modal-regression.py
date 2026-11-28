from pathlib import Path
import sys
root=Path(sys.argv[1])
u=root/'firmware/src/app/ui.c'
s=u.read_text()
a=s.index('static void cv_commit_main_band(unsigned band)')
b=s.index('static void cv_commit_knobs',a)
segment=s[a:b].replace('static void cv_commit_main_band(unsigned band)','static void modal_interrupt(unsigned band);\nstatic void cv_commit_main_band(unsigned band)').replace('force, cv_lcd_emit, &pos);','force, cv_lcd_emit, &pos);\n    modal_interrupt(band);').replace('force, cv_lcd_emit, (void *)&pos);','force, cv_lcd_emit, (void *)&pos);\n    modal_interrupt(band);')
assert segment.count('modal_interrupt(band)')==1
u.with_name('ui.modal-test.inc').write_text(s[:a]+segment+s[b:])
h=root/'host/x0x_host.c'
orig=h.read_text()
start=orig.index('int main(int argc, char')
test=r'''
static int inject, injected;
static void modal_interrupt(unsigned band)
{
    if (band || !inject) return;
    injected++;
    ui.view = V_MIX;
    ui.part = PART_BRK;
    ui.rec ^= 1;
    ui.help = B_EDIT + 1;
    ui.overlay = O_ASK;
    memcpy(ui.ask_q, (char[2][28]){"CHANGED","AFTER UPPER DMA"}, sizeof ui.ask_q);
    palette_set(5);
    const char *l[HELP_LINES];
    help_lines(B_SAVE, l); /* rewrites the source's three static buffers */
}
int main(void)
{
    unsigned n=0;
    static uint16_t target[240*MAIN_ROWS];
    static const int view[] = {V_HOME,V_SEQ,V_PART,V_GEN};
    boot();
    for (unsigned t=0;t<NPALETTES;++t)
        for (unsigned v=0;v<sizeof view/sizeof *view;++v)
            for (unsigned p=0;p<NPARTS;++p)
                for (unsigned b=0;b<=NB;++b) {
                    palette_set(t);
                    ui.view=view[v]; ui.part=p; ui.rec=n&1;
                    ui.overlay=b?O_NONE:O_ASK; ui.help=b;
                    memcpy(ui.ask_q,(char[2][28]){"ARE YOU SURE?","THE ORIGINAL QUESTION"},sizeof ui.ask_q);
                    blit_hash[1] = 0;
                    ui_t before = ui;
                    inject=0;
                    draw_main();
                    memcpy(target,fb+MAIN_Y*240,sizeof target);
                    ui=before;
                    blit_hash[1] = 0;
                    inject=1;
                    draw_main();
                    if (memcmp(target,fb+MAIN_Y*240,sizeof target)) {
                        fprintf(stderr,"paint not atomic: theme=%u view=%u part=%u help=%u\n",t,v,p,b);
                        return 1;
                    }
                    n++;
                }
    if (injected!=(int)n) return 2;
    printf("PASS: %u actual UI modal frames, %u injected state changes, all %u RGB565 pixels exact\n",n,injected,n*240u*MAIN_ROWS);
    return 0;
}
'''
h.with_name('x0x_host_modal.c').write_text(orig[:start]+orig[start:].replace('int main(', 'static int scenario_main(',1)+test)
copy=(root/'host/build_host.sh').read_text().replace('host/x0x_host.c $U','host/x0x_host_modal.c $U')
(root/'host/build_modal.sh').write_text(copy)
