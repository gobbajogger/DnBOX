import ast,contextlib,io,re,subprocess,tempfile,types,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
GATE=(ROOT/'tools/dnbox-gate.sh').read_text()
def func(name,end):
    return GATE[GATE.index(name+'()'):GATE.index(end,GATE.index(name+'()'))]
def run(body):
    return subprocess.run(['bash','-c',body],capture_output=True,text=True)
class GateTests(unittest.TestCase):
    def test_pool_reserve_is_subtracted(self):
        node=next(x for x in ast.parse((ROOT/'tools/build.py').read_text()).body if isinstance(x,ast.FunctionDef) and x.name=='check')
        ns=dict(re=re,APP_XIP=0x2000000,APP_CODE_MAX=100000,APP_SLOT=100000,LINE=re.compile('.'),rom_guard=types.SimpleNamespace(references=lambda x:[]))
        exec(compile(ast.Module(body=[node],type_ignores=[]),'check','exec'),ns)
        for free,margin in [(8364,172),(8704,512),(8820,628)]:
            syms=f'02000000 T _start\n01c08000 B _bss_end\n00000000 B _pool_start\n{0x54000-free:08x} B _pool_end\n'
            out=io.StringIO()
            with contextlib.redirect_stdout(out):ns['check'](bytes.fromhex('04818000'),syms,'','')
            self.assertIn(f'pool margin: {margin} B',out.getvalue())
    def test_both_model_floors(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)
            for model in ['elite','pro']:
                (p/f'{model}-xip.txt').write_text('1024');(p/f'{model}-pool.txt').write_text('512')
            command=f'OUT={d};XIP_MIN=1024;POOL_MIN=512;'+func('budget','ui_defects()')+'\nbudget'
            self.assertEqual(run(command).returncode,0)
            for model in ['elite','pro']:
                (p/f'{model}-pool.txt').write_text('172')
                self.assertNotEqual(run(command).returncode,0)
                (p/f'{model}-pool.txt').write_text('512')
            (p/'pro-pool.txt').unlink();self.assertNotEqual(run(command).returncode,0)
    def test_malformed_budget_numbers_fail_closed(self):
        with tempfile.TemporaryDirectory() as d:
            p=Path(d)
            for model in ['elite','pro']:
                (p/f'{model}-xip.txt').write_text('1024')
                (p/f'{model}-pool.txt').write_text('512')
            command=f'OUT={d};XIP_MIN=1024;POOL_MIN=512;'+func('budget','ui_defects()')+'\nbudget'
            for model in ['elite','pro']:
                for metric,valid in [('xip','1024'),('pool','512')]:
                    path=p/f'{model}-{metric}.txt'
                    for invalid in ['', 'garbage', '512 B', '512\n512', '-1', '1.5',
                                    '0x200', '0512', ' 512', '9999999999999999999999999']:
                        with self.subTest(model=model,metric=metric,invalid=invalid):
                            path.write_text(invalid)
                            result=run(command)
                            self.assertNotEqual(result.returncode,0)
                    path.write_text(valid)
    def test_missing_base_fails_without_network(self):
        command='BASE=;git(){ echo forbidden >&2;return 88;};'+func('no_loss','limits()')+'\nno_loss'
        r=run(command);self.assertNotEqual(r.returncode,0);self.assertNotIn('forbidden',r.stderr)
    def test_missing_local_base_fails(self):
        command='BASE='+'a'*40+';git(){ [ "$1" = cat-file ] && return 1;echo unexpected >&2;return 88;};'+func('no_loss','limits()')+'\nno_loss'
        r=run(command);self.assertNotEqual(r.returncode,0);self.assertNotIn('unexpected',r.stderr)
    def test_missing_reference_never_fetches(self):
        command='pass_env(){ :; };git(){ [ "$1" = rev-parse ] && return 1;echo unexpected >&2;return 88;};'+func('host_tests','budget_from_log()')+'\nhost_tests'
        r=run(command);self.assertNotEqual(r.returncode,0);self.assertNotIn('unexpected',r.stderr)
        self.assertNotRegex(GATE,r'\bgit\s+(fetch|pull|push|clone)\b')
if __name__=='__main__':unittest.main()
