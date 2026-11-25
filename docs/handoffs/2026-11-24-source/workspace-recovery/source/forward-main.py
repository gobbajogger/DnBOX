from pathlib import Path
import re, hashlib
base=Path('recovery/source/solve2.py').read_text().split('hit=0; miss=[]')[0]
exec(base)
n='firmware/src/app/main_fm1.c'
x=(Path('fw')/n).read_bytes()
for p,hs,_ in groups[n]:
 if p.name=='implemented-controls-aggregate.patch':
  x=apply(x,hs,False); assert x
  print('exact aggregate contexts',hashlib.sha256(x).hexdigest())
  break
s=x.decode()
changes=[
('static void draw_safe_mode(void)', '''#if defined(X0X_COLD_COMPANION_LAB)
#include "app/cold_companion_lab.h"
extern const uint8_t cold_lab_start[], cold_lab_end[], cold_lab_expected[];
static uint8_t cold_lab_bad;
#endif
static void draw_safe_mode(void)'''),
('    draw_text_box(0, 52, 240, &FONT_S, "STARTUP CRASHES: 2", C_WHITE, 1);','''#if defined(X0X_COLD_COMPANION_LAB)
    if (cold_lab_bad)
        draw_text_box(0, 52, 240, &FONT_S, "COMPANION DATA INVALID", C_WHITE, 1);
    else
#endif
    draw_text_box(0, 52, 240, &FONT_S, "STARTUP CRASHES: 2", C_WHITE, 1);'''),
('''            fl_plain_window_init();                 /* user sample slots read as plaintext through XIP */
#if BOARD_SMK37''','''            fl_plain_window_init();                 /* user sample slots read as plaintext through XIP */
#if defined(X0X_COLD_COMPANION_LAB)
            xdata_check(); /* independent recovery font, with its internal fallback */
            cold_lab_bad = !cold_lab_valid(cold_lab_start,
                (uintptr_t)cold_lab_end - (uintptr_t)cold_lab_start, cold_lab_expected);
            if (!cold_lab_bad) {
#endif
#if BOARD_SMK37'''),
('''            ota_boot_cleanup();
        }
    }
    lcd_init();''','''            ota_boot_cleanup();
#if defined(X0X_COLD_COMPANION_LAB)
            } else {
                safe_mode = 1;
            }
#endif
        }
    }
    lcd_init();''')]
for old,new in changes:
 assert s.count(old)==1,(old,s.count(old));s=s.replace(old,new,1)
x=s.encode()
for p,hs,_ in groups[n]:
 if p.name=='boot.diagnostic.diff':
  x=apply(x,hs,False);assert x
  print('guarded diagnostic contexts exact, forward port',hashlib.sha256(x).hexdigest())
  break
q=r/'forward-files'/n;q.parent.mkdir(parents=True,exist_ok=True);q.write_bytes(x)
