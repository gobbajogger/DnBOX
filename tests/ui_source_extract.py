#!/usr/bin/env python3
"""Fail-closed test extraction of actual C definitions, ignoring UI_DRAW only."""
import re
from pathlib import Path

def mask_c(text):
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',
                  lambda m: ''.join('\n' if c=='\n' else ' ' for c in m.group()), text)

def function(text, signature):
    masked=mask_c(text)
    signature=signature.replace('UI_DRAW ', '')
    pattern=re.escape(signature).replace(r'\ ', r'\s+')
    pattern=pattern.replace('void', r'(?:UI_DRAW\s+)?void', 1)
    matches=list(re.finditer(r'^'+pattern+r'\s*\{', masked, re.M))
    if len(matches)!=1: raise ValueError('expected exactly one definition: '+signature)
    start=matches[0].start();pos=matches[0].end();depth=1
    while pos<len(masked) and depth:
        depth+=(masked[pos]=='{')-(masked[pos]=='}');pos+=1
    if depth: raise ValueError('unterminated definition: '+signature)
    return text[start:pos]+'\n'

def autosave(text):
    definitions=[]
    for name in ('AUTOSAVE_QUIET','AUTOSAVE_GAP'):
        matches=re.findall(r'^#define '+name+r'\s+[^\n]+$',text,re.M)
        if len(matches)!=1: raise ValueError('expected exactly one macro: '+name)
        definitions.append(matches[0])
    return '\n'.join(definitions)+'\n'+function(text,'static void autosave(void)')

if __name__=='__main__':
    import sys
    if len(sys.argv)!=3 or sys.argv[1]!='autosave': raise SystemExit('usage: ui_source_extract.py autosave path')
    print(autosave(Path(sys.argv[2]).read_text()),end='')
