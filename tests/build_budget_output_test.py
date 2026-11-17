#!/usr/bin/env python3
"""Build summary regression, without invoking a compiler or inventing build results."""
import ast
from pathlib import Path
import re
from types import SimpleNamespace

source = Path(__file__).resolve().parents[1] / 'tools/build.py'
node = next(n for n in ast.parse(source.read_text()).body if isinstance(n, ast.FunctionDef) and n.name == 'check')
checks = 0
for model in ('elite', 'pro'):
    for xip_free, pool_margin in ((1024, 512), (0, 0), (-4, -1)):
        image = bytes.fromhex('04818000') + bytes(12)
        pool_start = 0x01c20000
        pool_end = pool_start + 0x54000 - 8192 - pool_margin
        symbols = f'02000000 symbol _start\n01c08010 symbol _bss_end\n{pool_start:08x} symbol _pool_start\n{pool_end:08x} symbol _pool_end\n'
        ns = dict(re=re, MODEL=model, APP_XIP=0x02000000, APP_CODE_MAX=len(image)+xip_free,
                  APP_SLOT=0x90000, LINE=re.compile(r'^$'), rom_guard=SimpleNamespace(references=lambda _: []))
        exec(compile(ast.Module(body=[node], type_ignores=[]), str(source), 'exec'), ns)
        errors, notes = ns['check'](image, symbols, '', '')
        assert f'{model} XIP free: {xip_free} bytes' in notes
        assert f'{model} pool margin: {pool_margin} bytes (above 8192-byte reserve)' in notes
        assert any('exceeds the app code area' in e for e in errors) == (xip_free < 0)
        assert any('pool headroom' in e for e in errors) == (pool_margin < 0)
        checks += 1
print(f'PASS {checks}: model-attributable budget output and unchanged overflow checks')
