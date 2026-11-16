#!/usr/bin/env python3
"""Factory input closure regressions. These are not audio evidence."""
import importlib.util
from pathlib import Path
import shutil
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('factory_runner', REPO / 'tests/host/run_synthone_factory.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class RunnerGuards(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.repo, self.stage = self.root / 'repo', self.root / 'stage'
        for path in ['assets/factory_banks/synthone', 'tests/fixtures/synthone-factory']:
            shutil.copytree(REPO / path, self.repo / path)
        for path in ['tools/compile-synthone-factory.py', 'firmware/src/dsp/synthone_controls_data.inc']:
            dest = self.repo / path
            dest.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(REPO / path, dest)

    def collect(self):
        return m.collect_factory_inputs(self.repo)

    def test_changed_staged_fixture_cannot_pass(self):
        allowed = m.stage_inputs(self.collect(), self.stage)
        f = self.stage / 'tests/fixtures/synthone-factory/frontend/source_controls.cpp'
        f.write_text(f.read_text() + '\n')
        with self.assertRaisesRegex(ValueError, 'bound input changed'):
            m.verify_files(allowed)

    def test_live_changes_after_snapshot_do_not_change_compiled_input(self):
        data = self.collect()
        f = self.repo / 'tests/fixtures/synthone-factory/frontend/source_controls.cpp'
        f.write_text(f.read_text() + '\n')
        allowed = m.stage_inputs(data, self.stage)
        m.verify_files(allowed)
        self.assertNotEqual(f.read_bytes(), (self.stage / f.relative_to(self.repo)).read_bytes())

    def test_corrupt_fixture_rejected_before_build(self):
        f = self.repo / 'tests/fixtures/synthone-factory/frontend/source_controls.cpp'
        f.write_text(f.read_text() + '\n')
        with self.assertRaisesRegex(ValueError, 'fixture hash'):
            self.collect()

    def test_corrupt_original_bank_rejected_before_build(self):
        f = self.repo / 'assets/factory_banks/synthone/BankA.json'
        f.write_text(f.read_text() + '\n')
        with self.assertRaisesRegex(ValueError, 'factory original hash'):
            self.collect()

    def test_unlisted_include_shadow_not_copied_and_rejected_if_used(self):
        f = self.repo / 'tests/fixtures/synthone-factory/frontend/full_fixture.hpp'
        f.write_text('// unexpected shadow')
        allowed = m.stage_inputs(self.collect(), self.stage)
        self.assertFalse((self.stage / f.relative_to(self.repo)).exists())
        dep = self.root / 'bad.d'
        dep.write_text('test.o: ' + str(f) + '\n')
        with self.assertRaisesRegex(ValueError, 'unlisted include/source'):
            m.verify_dependencies(dep, allowed)

    def test_descriptor_and_generator_are_bound(self):
        allowed = m.stage_inputs(self.collect(), self.stage)
        for name in ['tools/compile-synthone-factory.py', 'firmware/src/dsp/synthone_controls_data.inc']:
            f = self.stage / name
            self.assertIn(f.resolve(), allowed)
        f.write_text(f.read_text() + '\n')
        with self.assertRaisesRegex(ValueError, 'bound input changed'):
            m.verify_files(allowed)

    def test_manifest_path_cannot_escape(self):
        with self.assertRaisesRegex(ValueError, 'escaping'):
            m.stage_inputs({Path('../escaped'): b'x'}, self.stage)


if __name__ == '__main__':
    unittest.main()
