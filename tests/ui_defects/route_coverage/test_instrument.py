# SPDX-License-Identifier: GPL-3.0-only
import re,unittest
from run import FUNCTIONS,BRANCHES,ROUTES,instrument

def sources():
    ui='#include "x0x.h"\n'
    for name in FUNCTIONS:
        if name!='draw_drum_voice':ui+='static UI_DRAW void '+name+'(int band)\n{\n    keep();\n}\n'
    ui+='static UI_DRAW void draw_main(void)\n{\n'
    ui+='\n'.join(anchor+' unchanged(); }' for name,anchor in BRANCHES.items() if name not in ('smc-body','project-transfer-header'))
    ui+='\n}\nvoid ui_frame(void)\n{\n'
    ui+='\n'.join(BRANCHES[name]+' unchanged(); }' for name in ('smc-body','project-transfer-header'))+'\n}\n'
    return ui,'static UI_DRAW void draw_drum_voice(int band)\n{\n    keep();\n}\n'
class InstrumentTests(unittest.TestCase):
    def test_exact_observation_only(self):
        ui,drum=sources();u,d=instrument(ui,drum)
        self.assertEqual(sorted(map(int,re.findall(r'fwui_route_hits\[(\d+)\]\+\+',u+d))),list(range(len(ROUTES))))
        u=re.sub(r'\nstatic unsigned fwui_route_hits\[\d+\];','',u)
        u=re.sub(r'\n +fwui_route_hits\[\d+\]\+\+;','',u)
        d=re.sub(r'\n +fwui_route_hits\[\d+\]\+\+;','',d)
        self.assertEqual(u,ui);self.assertEqual(d,drum)
    def test_missing_function_rejected(self):
        ui,d=sources()
        with self.assertRaises(ValueError):instrument(ui.replace('draw_mix(','other_mix('),d)
    def test_duplicate_definition_rejected(self):
        ui,d=sources()
        with self.assertRaises(ValueError):instrument(ui+'\nstatic UI_DRAW void draw_mix(int band)\n{\n}\n',d)
    def test_missing_branch_rejected(self):
        ui,d=sources()
        with self.assertRaises(ValueError):instrument(ui.replace(BRANCHES['clip-wait-body'],'if (other()) {'),d)
    def test_missing_include_rejected(self):
        ui,d=sources()
        with self.assertRaises(ValueError):instrument(ui.replace('x0x.h','other.h'),d)
    def test_similar_outside_branch_not_counted(self):
        ui,d=sources();ui+='void unrelated(void)\n{\n'+BRANCHES['smc-body']+' untouched(); }\n}\n'
        u,_=instrument(ui,d);self.assertIn(BRANCHES['smc-body']+' untouched(); }',u)
if __name__=='__main__':unittest.main()
