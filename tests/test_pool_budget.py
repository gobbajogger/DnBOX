import ast, contextlib, io, re, subprocess, tempfile, types, unittest
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]

def build_check(symbols):
    node = next(n for n in ast.parse((ROOT/'tools/build.py').read_text()).body if isinstance(n, ast.FunctionDef) and n.name == 'check')
    ns = dict(re=re, APP_XIP=0x2000000, APP_CODE_MAX=100000, APP_SLOT=100000, LINE=re.compile('.'), rom_guard=types.SimpleNamespace(references=lambda _: []))
    exec(compile(ast.Module(body=[node], type_ignores=[]), 'actual-build-check', 'exec'), ns)
    out = io.StringIO()
    with contextlib.redirect_stdout(out):
        errors, _ = ns['check'](bytes.fromhex('04818000'), symbols, '', '')
    return errors, out.getvalue()

def symbols(raw):
    return f'02000000 T _start\n01c08000 B _bss_end\n01c20000 B _pool_start\n{0x1c74000-raw:08x} B _pool_end\n'

def gate_functions():
    s = (ROOT/'tools/dnbox-gate.sh').read_text()
    parser = s[s.index('budget_from_log()'):s.index('fw_build()')]
    budget = s[s.index('budget()'):s.index('ui_defects()')]
    return parser + budget

class PoolBudget(unittest.TestCase):
    def test_reserve_boundaries(self):
        for raw, margin in [(8191,-1),(8192,0),(8364,172),(8703,511),(8704,512),(8984,792)]:
            with self.subTest(raw=raw):
                errors, output = build_check(symbols(raw))
                self.assertIn(f'pool margin: {margin} B\n', output)
                self.assertEqual(bool(errors), raw < 8192)

    def test_invalid_pool_symbols_fail_closed(self):
        valid = symbols(8704)
        cases = [
            valid.replace('01c20000 B _pool_start\n',''),
            '\n'.join(l for l in valid.splitlines() if '_pool_end' not in l)+'\n',
            valid+'01c20000 B _pool_start\n',
            valid+valid.splitlines()[-1]+'\n',
            valid.replace(valid.splitlines()[-1], '01c1ffff B _pool_end'),
            valid.replace(valid.splitlines()[-1], '01c74001 B _pool_end'),
            valid.replace('01c20000 B _pool_start', '01c1ffff B _pool_start'),
        ]
        for case in cases:
            with self.subTest(symbols=case):
                errors, output = build_check(case)
                self.assertTrue(errors)
                self.assertNotRegex(output, r'pool margin:', 'invalid symbols must not emit a usable metric')

    def test_actual_build_output_through_both_model_gate(self):
        for bad_model in ('elite','pro'):
            for raw in (8364,8703,8704):
                with self.subTest(model=bad_model, raw=raw), tempfile.TemporaryDirectory() as d:
                    p=Path(d)
                    for model in ('elite','pro'):
                        _, output=build_check(symbols(raw if model == bad_model else 8704))
                        (p/f'{model}.log').write_text(output)
                    cmd=f'OUT={d};XIP_MIN=1024;POOL_MIN=512;'+gate_functions()+'\nbudget_from_log "$OUT/elite.log" elite && budget_from_log "$OUT/pro.log" pro && budget'
                    result=subprocess.run(['bash','-c',cmd],capture_output=True,text=True)
                    self.assertEqual(result.returncode==0,raw>=8704,result.stdout+result.stderr)

    def test_missing_or_malformed_model_metrics_fail(self):
        for model in ('elite','pro'):
            for value in (None,'garbage','512\n512','999999999999999999999999'):
                with self.subTest(model=model,value=value), tempfile.TemporaryDirectory() as d:
                    p=Path(d)
                    for m in ('elite','pro'):
                        (p/f'{m}-xip.txt').write_text('1024')
                        (p/f'{m}-pool.txt').write_text('512')
                    target=p/f'{model}-pool.txt'
                    if value is None: target.unlink()
                    else: target.write_text(value)
                    result=subprocess.run(['bash','-c',f'OUT={d};XIP_MIN=1024;POOL_MIN=512;'+gate_functions()+'\nbudget'],capture_output=True,text=True)
                    self.assertNotEqual(result.returncode,0)

if __name__=='__main__': unittest.main()
