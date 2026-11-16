"""Bind the actual Capstone package before importing or loading its native core."""
import hashlib
import json
import os
from pathlib import Path
import sys


def verify(root, pin):
    expected = pin['files']
    members = list(root.rglob('*'))
    if any(p.is_symlink() for p in members):
        raise ValueError('Capstone symlink is not admitted')
    actual = {str(p.relative_to(root)) for p in members if p.is_file()}
    if actual != set(expected):
        raise ValueError('Capstone package file set differs')
    for name, record in expected.items():
        relative = Path(name)
        if relative.is_absolute() or '..' in relative.parts or relative.parts[0] != 'capstone':
            raise ValueError('invalid dependency path')
        data = (root / relative).read_bytes()
        if len(data) != record['bytes'] or hashlib.sha256(data).hexdigest() != record['sha256']:
            raise ValueError('Capstone file hash/length mismatch: ' + name)


def load_capstone():
    if os.environ.get('LIBCAPSTONE_PATH'):
        raise ValueError('LIBCAPSTONE_PATH override is not admitted')
    pin_bytes = Path(__file__).with_name('capstone-pin.json').read_bytes()
    pin = json.loads(pin_bytes)
    root = Path(os.environ.get('SJ6_CAPSTONE_ROOT', 'sound-v2-evidence/capstone-source-pin/unpacked')).resolve()
    verify(root, pin)
    sys.dont_write_bytecode = True
    sys.path.insert(0, str(root))
    import capstone
    if Path(capstone.__file__).resolve() != root / 'capstone/__init__.py':
        raise ValueError('Capstone import shadowed')
    if Path(capstone._cs._name).resolve() != root / 'capstone/lib/libcapstone.so':
        raise ValueError('Capstone native core shadowed')
    if capstone.__version__ != pin['binding_version'] or list(capstone.cs_version()) != pin['native_api']:
        raise ValueError('Capstone version/API mismatch')
    verify(root, pin)
    return capstone, {'distribution_version': pin['distribution_version'],
        'binding_version': capstone.__version__, 'native_api': list(capstone.cs_version()),
        'wheel_sha256': pin['wheel_sha256'], 'source': pin['source'],
        'package_pin_sha256': hashlib.sha256(pin_bytes).hexdigest(),
        'package_files_checked': len(pin['files'])}
