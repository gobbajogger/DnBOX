#!/usr/bin/env python3
import importlib.util
import json
from pathlib import Path
import shutil
import tempfile
import unittest

REPO = Path(__file__).resolve().parents[2]
spec = importlib.util.spec_from_file_location('restore', REPO / 'tools/restore-synthone-test-inputs.py')
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class RestoreTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.bundle = self.root / 'bundle'
        self.out = self.root / 'restored'
        shutil.copytree(REPO / 'assets/test-inputs/synthone', self.bundle)

    def test_restore_and_relocate_without_executing(self):
        m.restore(self.bundle, self.out)
        result = json.loads((self.out / 'RESTORE-RECEIPT.json').read_text())
        self.assertEqual(result['files_verified'], 97)
        commands = (self.out / 'commands.json').read_text()
        self.assertNotIn('@SOURCE_BUILD@', commands)
        self.assertIn(str(self.out), commands)
        with self.assertRaisesRegex(ValueError, 'not exist'):
            m.restore(self.bundle, self.out)

    def test_corrupt_part_creates_no_output(self):
        f = self.bundle / 'inputs.tar.gz.part00'
        b = bytearray(f.read_bytes());b[0] ^= 1;f.write_bytes(b)
        with self.assertRaisesRegex(ValueError, 'part hash'):
            m.restore(self.bundle, self.out)
        self.assertFalse(self.out.exists())

    def test_corrupt_member_hash_creates_no_output(self):
        f = self.bundle / 'MANIFEST.json'
        v = json.loads(f.read_text());v['files'][0]['sha256'] = '0' * 64;f.write_text(json.dumps(v))
        with self.assertRaisesRegex(ValueError, 'member hash'):
            m.restore(self.bundle, self.out)
        self.assertFalse(self.out.exists())


if __name__ == '__main__':
    unittest.main()
