#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Baseline-only unavailable UI contract; the original audio differential is separate."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / 'tests/host/engine_808_scratch_alias_test.sh'
DECLARATIONS = '''#include <stdint.h>
typedef struct { uint32_t sentinel; } drum_curve_t;
typedef struct { uint32_t sentinel; } engine_decay_view_t;
typedef struct { uint32_t sentinel; } drum909_amp_snapshot;
typedef struct { uint32_t sentinel; } juno6_envelope_snapshot;
'''
CALLS = '''int main(void) {
 uint8_t peaks[2]={17,29}; uint32_t revision=73;
 drum909_amp_snapshot drum={91}; juno6_envelope_snapshot juno={97};
 drum_curve_t curve={17};
 if(engine_drum808_curve(0,&curve)!=0 || curve.sentinel!=17)return 9;
 float ms=11,hz=31; engine_decay_view_t decay={41};
 if(engine_bd909_pitch_view(&ms,&hz)!=0 || ms!=11 || hz!=31)return 7;
 if(engine_drum909_decay_view(0,&decay)!=0 || decay.sentinel!=41)return 8;
 if(engine_brk_preview(0)!=0 || engine_brk_preview_active()!=0)return 1;
 engine_brk_preview_end();
 if(engine_brk_outline_revision(0)!=0)return 2;
 if(engine_brk_outline_snapshot(0,peaks,2,&revision)!=0)return 3;
 if(engine_drum909_amp_view(0,&drum)!=0)return 4;
 if(engine_juno_envelope_view(0,&juno)!=0)return 5;
 if(peaks[0]!=17 || peaks[1]!=29 || revision!=73 || drum.sentinel!=91 || juno.sentinel!=97)return 6;
 return 0;
}
'''

class ArchiveUIContract(unittest.TestCase):
    def source(self):
        script = SCRIPT.read_text()
        matches = re.findall(r'cat >> "\$D/engine-base.c" <<\x27BASE_PREVIEW_COMPAT\x27\n(.*?)\nBASE_PREVIEW_COMPAT', script, re.S)
        self.assertEqual(len(matches), 1, 'compatibility must be appended only to archived engine')
        self.assertIn('src=$D/engine-base.c; [ $v = new ] && src=firmware/src/app/engine.c', script)
        return matches[0]

    def compile_run(self, source):
        with tempfile.TemporaryDirectory(prefix='archive-ui-compat-') as tmp:
            tmp = Path(tmp); c = tmp/'contract.c'; exe = tmp/'contract'
            c.write_text(DECLARATIONS + source + '\n' + CALLS)
            result = subprocess.run([os.environ.get('CC', 'cc'), '-std=c99', '-Wall', '-Wextra', '-Werror', str(c), '-o', str(exe)], capture_output=True, text=True)
            if result.returncode: return result.returncode, result.stderr, None
            return 0, '', subprocess.run([str(exe)], capture_output=True).returncode

    def test_all_symbols_refuse_without_mutation(self):
        code, diagnostic, runtime = self.compile_run(self.source())
        self.assertEqual(code, 0, diagnostic)
        self.assertEqual(runtime, 0, 'unavailable UI accessor changed output or reported success')

    def test_missing_symbol_is_not_accepted(self):
        source = self.source()
        source, count = re.subn(r'int engine_juno_envelope_view\([^}]+\}', '', source, count=1)
        self.assertEqual(count, 1)
        code, diagnostic, runtime = self.compile_run(source)
        self.assertNotEqual(code, 0)
        self.assertIn('engine_juno_envelope_view', diagnostic)
        self.assertIsNone(runtime)

    def test_false_available_is_not_accepted(self):
        source = self.source().replace('(void)voice; (void)out; return 0;', '(void)voice; (void)out; return 1;')
        code, diagnostic, runtime = self.compile_run(source)
        self.assertEqual(code, 0, diagnostic)
        self.assertNotEqual(runtime, 0)

if __name__ == '__main__':
    unittest.main()
