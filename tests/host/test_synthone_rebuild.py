"""Fresh-object regression: no preexisting dependency objects may be linked."""
import pathlib
import subprocess
import tempfile
import unittest
import json
import hashlib

RUNNER = pathlib.Path(__file__).with_name('run_synthone_renderer.py')

class RebuildTest(unittest.TestCase):
    def test_recompiles_dependency_instead_of_trusting_stale_object(self):
        with tempfile.TemporaryDirectory() as d:
            base = pathlib.Path(d)
            source = base/'closure/scheduler'
            (source/'tests').mkdir(parents=True)
            dep = source/'dependency.c'
            dep.write_text('int broken syntax;')
            (base/'COMPILED-SOURCES.json').write_text(json.dumps([
                {'path':'dependency.c','sha256':hashlib.sha256(dep.read_bytes()).hexdigest()}]))
            harness = source/'tests/source_scheduler_test.cpp'
            harness.write_text('// replaced by repository adapter test')
            (base/'CLOSURE-SHA256.json').write_text(json.dumps({
                str(path.relative_to(base/'closure')):hashlib.sha256(path.read_bytes()).hexdigest()
                for path in source.rglob('*') if path.is_file()}))
            # A stale object exists, but a fresh compilation must catch syntax.
            (base/'dep.o').write_bytes(b'stale')
            (base/'commands.json').write_text(json.dumps([
                ['cc','-c',str(dep),'-o',str(base/'dep.o')],
                ['c++','-c',str(source/'tests/source_scheduler_test.cpp'),'-o',str(base/'test.o')],
                ['c++',str(base/'dep.o'),str(base/'test.o'),'-o',str(base/'test')]]))
            result = subprocess.run(['python3',str(RUNNER),'--source-build',str(base),'--output',str(base/'out')],capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0)
            self.assertIn('dependency.c',result.stderr)
            self.assertNotIn('synthone_renderer_test.cpp:',result.stderr)

    def test_rejects_compiling_outside_hashed_closure(self):
        with tempfile.TemporaryDirectory() as d:
            base = pathlib.Path(d)
            source = base/'closure/scheduler'
            source.mkdir(parents=True)
            good = source/'dependency.c'
            good.write_text('int correct = 1;')
            other = base/'different.c'
            other.write_text('int correct = 2;')
            (base/'COMPILED-SOURCES.json').write_text(json.dumps([
                {'path':'dependency.c','sha256':hashlib.sha256(good.read_bytes()).hexdigest()}]))
            (base/'CLOSURE-SHA256.json').write_text(json.dumps({
                'scheduler/dependency.c':hashlib.sha256(good.read_bytes()).hexdigest()}))
            (base/'commands.json').write_text(json.dumps([
                ['cc','-c',str(other),'-o',str(base/'dep.o')],
                ['c++','-c',str(source/'tests/source_scheduler_test.cpp'),'-o',str(base/'test.o')],
                ['c++',str(base/'dep.o'),str(base/'test.o'),'-o',str(base/'test')]]))
            result = subprocess.run(['python3',str(RUNNER),'--source-build',str(base),'--output',str(base/'out')],capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0)
            self.assertIn('Unverified compile source',result.stderr)

    def test_requires_exactly_one_adapter_harness(self):
        with tempfile.TemporaryDirectory() as d:
            base = pathlib.Path(d)
            source = base/'closure/scheduler'
            source.mkdir(parents=True)
            dep = source/'dependency.c'
            dep.write_text('int main(void) {return 0;}')
            digest = hashlib.sha256(dep.read_bytes()).hexdigest()
            (base/'COMPILED-SOURCES.json').write_text(json.dumps([
                {'path':'dependency.c','sha256':digest}]))
            (base/'CLOSURE-SHA256.json').write_text(json.dumps({'scheduler/dependency.c':digest}))
            (base/'commands.json').write_text(json.dumps([
                ['cc','-c',str(dep),'-o',str(base/'dep.o')],
                ['cc',str(base/'dep.o'),'-o',str(base/'test')]]))
            result = subprocess.run(['python3',str(RUNNER),'--source-build',str(base),'--output',str(base/'out')],capture_output=True,text=True)
            self.assertNotEqual(result.returncode,0)
            self.assertIn('Expected exactly one adapter harness',result.stderr)

    def test_runtime_overlay_cannot_replace_independent_oracle(self):
        import importlib.util
        spec = importlib.util.spec_from_file_location('renderer_runner', RUNNER)
        runner = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(runner)
        with tempfile.TemporaryDirectory() as d:
            root = pathlib.Path(d)
            runtime, closure = root/'runtime', root/'closure'
            for group in ('scheduler', 'experiment', 'controls'):
                (runtime/group).mkdir(parents=True)
                (closure/group).mkdir(parents=True)
            for name in ('oracle/state.hpp', 'tests/source_scheduler_test.cpp', 'full_oracle.cpp'):
                with self.subTest(path=name):
                    bad = runtime/'scheduler'/name
                    good = closure/'scheduler'/name
                    bad.parent.mkdir(parents=True, exist_ok=True)
                    good.parent.mkdir(parents=True, exist_ok=True)
                    bad.write_text('self agreeing replacement')
                    good.write_text('independent retained input')
                    with self.assertRaisesRegex(ValueError, 'Protected oracle/test path'):
                        runner.stage_runtime(closure, runtime)
                    self.assertEqual(good.read_text(), 'independent retained input')
                    bad.unlink()

if __name__ == '__main__': unittest.main()
