#!/usr/bin/env python3
"""Source guard: both snapshot stores clamp abs(-32768); legacy API delegates."""
from __future__ import annotations
import pathlib
import re

ROOT = pathlib.Path(__file__).resolve().parents[2]
ENGINE = ROOT / 'firmware/src/app/engine.c'

def function(text: str, name: str) -> str:
    match = re.search(r'^\s*(?:X0X_COLD\s+int|static\s+uint8_t)\s+' + re.escape(name) + r'\s*\([^)]*\)\s*\{', text, re.M)
    if not match:
        raise ValueError(name + ' definition not found')
    start = match.end(); depth = 1
    for i in range(start, len(text)):
        if text[i] == '{': depth += 1
        elif text[i] == '}':
            depth -= 1
            if depth == 0: return text[start:i]
    raise ValueError(name + ' unterminated')

def compact(text: str) -> str:
    return re.sub(r'\s+', '', text)

def validate(text: str) -> None:
    helper = compact(function(text, 'brk_outline_peak_u8'))
    if helper != 'return(uint8_t)(pk>=32768?255:pk>>7);':
        raise ValueError('negative full-scale clamp law changed')
    wrapper = compact(function(text, 'engine_brk_outline'))
    if wrapper != 'returnengine_brk_outline_snapshot(which,peaks,n,0);':
        raise ValueError('legacy outline must delegate to snapshot')
    snapshot = function(text, 'engine_brk_outline_snapshot')
    stores = [compact(x) for x in re.findall(r'peaks\[[^\]]+\]\s*=\s*([^;]+);', snapshot)]
    if stores != ['brk_outline_peak_u8(pk)', 'brk_outline_peak_u8(pk)', '0']:
        raise ValueError('snapshot must use both clamped stores and zero-only padding')

def main() -> None:
    text = ENGINE.read_text()
    validate(text)
    # Deliberate production-source mutations must fail this exact admission,
    # not merely throw a parser/compiler error for an unrelated reason.
    cases = [
        ('brk_outline_peak_u8(pk);', '(uint8_t)(pk >> 7);', 'snapshot must use'),
        ('brk_outline_peak_u8(pk);', '0;', 'snapshot must use'),
        ('\n    peaks[col] = brk_outline_peak_u8(pk);', '\n    peaks[col] = (uint8_t)(pk >> 7);', 'snapshot must use'),
        ('return engine_brk_outline_snapshot(which, peaks, n, 0);', 'return 0;', 'legacy outline'),
        ('pk >= 32768 ? 255', 'pk >= 32769 ? 255', 'clamp law'),
        ('pk >= 32768 ? 255', 'pk >= 32768 ? 254', 'clamp law'),
    ]
    for old, new, marker in cases:
        if old not in text: raise RuntimeError('mutation anchor missing: ' + old)
        try: validate(text.replace(old, new, 1))
        except ValueError as error:
            if marker not in str(error): raise RuntimeError('unrelated mutant failure') from error
        else: raise RuntimeError('unsafe outline mutant accepted')
    print('PASS: outline snapshot clamps both peak stores; legacy delegation and six negatives verified')

if __name__ == '__main__':
    main()
