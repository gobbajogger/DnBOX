"""Extract the exact folded oscillator body, with no raw/proprietary tables."""
from pathlib import Path
text=Path(__file__).with_name('wire-fold-cached.py').read_text()
a=text.split('static float probe_shape(',1)[1].split('#endif',1)[0]
a='static float probe_shape('+a
stage=Path(__file__).with_name('stage.c').read_text().split('/* Normal is deliberately separate')[0]
stage=stage.replace('#include "halfband.h"','').replace('anchor-generated.private.h','fold-data.h').replace('TBL[s][row][p]','TBL[TBL_MAP[s][row]][p]').replace('TBL[s][row][q]','TBL[TBL_MAP[s][row]][q]')
print(stage.replace('#include <assert.h>',''))
print('typedef struct {float phase,increment,oscillator_history[4];} delay_voice;')
print(a.replace('dnbx_layer_voice','delay_voice'))
