import hashlib
import importlib.util
from pathlib import Path
import tempfile
import unittest

path = Path(__file__).resolve().parents[1] / 'tools/sj6_loader_audit/dependencies.py'
spec = importlib.util.spec_from_file_location('deps', path)
m = importlib.util.module_from_spec(spec)
spec.loader.exec_module(m)


class Dependencies(unittest.TestCase):
    def fixture(self, root):
        p = root / 'capstone/__init__.py'
        p.parent.mkdir()
        p.write_bytes(b'known')
        return {'files': {'capstone/__init__.py': {'bytes': 5, 'sha256': hashlib.sha256(b'known').hexdigest()}}}

    def test_exact(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); m.verify(root, self.fixture(root))

    def test_changed_file(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); pin = self.fixture(root)
            (root / 'capstone/__init__.py').write_bytes(b'other')
            with self.assertRaisesRegex(ValueError, 'hash/length'): m.verify(root, pin)

    def test_unlisted_shadow(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); pin = self.fixture(root)
            (root / 'ctypes.py').write_text('unlisted')
            with self.assertRaisesRegex(ValueError, 'file set'): m.verify(root, pin)

    def test_unlisted_bytecode(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); pin = self.fixture(root)
            p = root / 'capstone/__pycache__/__init__.cpython-312.pyc'
            p.parent.mkdir(); p.write_bytes(b'unlisted')
            with self.assertRaisesRegex(ValueError, 'file set'): m.verify(root, pin)

    def test_symlink(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); pin = self.fixture(root)
            (root / 'shadow').symlink_to(root / 'capstone/__init__.py')
            with self.assertRaisesRegex(ValueError, 'symlink'): m.verify(root, pin)

    def test_missing_file(self):
        with tempfile.TemporaryDirectory() as d:
            root = Path(d); pin = self.fixture(root)
            (root / 'capstone/__init__.py').unlink()
            with self.assertRaisesRegex(ValueError, 'file set'): m.verify(root, pin)


if __name__ == '__main__': unittest.main()
