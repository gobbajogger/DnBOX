#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Source-derived renderer inventory. Function reach is not visual approval.

IDs use repository path and C symbol, never line numbers or walker order.
This conservative lexical audit inventories every application function that
paints, or calls one that paints. Preprocessor-disabled routes remain visible
and unobserved; they cannot silently disappear from qualification.
"""
import argparse, hashlib, json, re
from pathlib import Path

def masked(text):
    pattern=r'/\*[\s\S]*?\*/|//[^\n]*|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern,lambda m:''.join('\n' if c=='\n' else ' ' for c in m[0]),text)

def functions(path, text):
    clean=masked(text)
    # Function definitions only: declarations, control statements, macros and
    # initializer braces do not match the required return type at line start.
    pattern=r'(?m)^[ \t]*(?:(?:static|inline|extern|UI_DRAW|X0X_COLD|CK_FN|CK_CD_FN)\s+)*(?:__attribute__\s*\(\([^\n]*?\)\)\s*)?(?:void|int|unsigned|float|double|char|uint\d+_t|int\d+_t|pref_t|const\s+char\s*\*)\s*\**\s+(\w+)\s*\([^;{}]*\)\s*\{'
    rows=[]
    for match in re.finditer(pattern,clean):
        opening=match.end()-1; depth=1; end=opening+1
        while depth and end<len(clean):
            depth+=(clean[end]=='{')-(clean[end]=='}');end+=1
        if depth:raise ValueError('unclosed function: '+path+':'+match[1])
        body=clean[opening+1:end-1]
        rows.append(dict(id=path+'::'+match[1],path=path,symbol=match[1],line=text.count('\n',0,match.start())+1,
                         start=match.start(),opening=opening,end=end,calls=sorted(set(re.findall(r'\b(\w+)\s*\(',body)))))
    return rows

def inventory(root):
    rows=[]; sources={}
    for path in sorted((root/'firmware/src').rglob('*')):
        if path.suffix not in ('.c','.h','.inc'):continue
        rel=path.relative_to(root).as_posix();text=path.read_text()
        sources[rel]=hashlib.sha256(path.read_bytes()).hexdigest()
        # Low-level USB/DSP sources can contain alternative preprocessor brace
        # layouts. They are still hashed, but only application/graphics files
        # and files mentioning painting APIs participate in the lexical graph.
        if '/app/' in rel or re.search(r'\b(?:cv_(?:rect|text\w*|poly\w*|line\w*|circle\w*|commit\w*)|lcd_fill|lcd_blit|draw_text_box|draw_boot_screen)\s*\(',masked(text)):
            rows.extend(functions(rel,text))
    # Raster/text primitives and the shared glyph calls define painting, rather
    # than a hand-maintained list of known screen names.
    primitive=re.compile(r'^(?:cv_(?:rect|text\w*|poly\w*|line\w*|circle\w*|commit\w*|lcd\w*)|lcd_blit|lcd_fill|draw_text_box|draw_boot_screen|text_[cr]|ui_pictogram|ui_glyph_\w+|png_write)$')
    selected={r['id'] for r in rows if any(primitive.match(c) for c in r['calls'])}
    while True:
        names={r['symbol'] for r in rows if r['id'] in selected}
        more={r['id'] for r in rows if names.intersection(r['calls'])}
        if more<=selected:break
        selected|=more
    result=[r for r in rows if r['id'] in selected]
    if not result:raise ValueError('empty renderer inventory')
    # Registration follows the actual production translation unit, independently
    # of which files the host walker happens to include. Preserve conditional
    # includes conservatively; inactive variants remain reviewable.
    registered=set();pending=[root/'firmware/src/x0x.c']
    while pending:
        path=pending.pop().resolve()
        if not path.is_file() or path in registered:continue
        registered.add(path)
        for name in re.findall(r'^\s*#\s*include\s*"([^"]+)"',path.read_text(),re.M):
            choices=[path.parent/name,root/'firmware/src'/name]
            for choice in choices:
                if choice.is_file():pending.append(choice);break
    by_name={}
    for row in rows:by_name.setdefault(row['symbol'],[]).append(row)
    reachable=set();pending=['ui_frame','fm1_main','fm1_cstart']
    while pending:
        name=pending.pop()
        if name in reachable:continue
        reachable.add(name)
        for row in by_name.get(name,[]):pending.extend(row['calls'])
    for r in result:
        r['directPaint']=any(primitive.match(c) for c in r['calls'])
        r['registeredInTranslationUnit']=(root/r['path']).resolve() in registered
        r['directCallReachableFromEntry']=r['symbol'] in reachable
        r['sourceDisposition']='UNREGISTERED_SOURCE' if not r['registeredInTranslationUnit'] else 'DIRECT_CALL_REACHABLE' if r['directCallReachableFromEntry'] else 'NO_DIRECT_CALL_PATH_REVIEW_REQUIRED'
    unresolved=[]
    for path in sources:
        text=(root/path).read_text();clean=masked(text)
        owners=[r for r in rows if r['path']==path]
        for match in re.finditer(r'\b(\w+)\s*\(',clean):
            if primitive.match(match[1]) and not any(r['start']<=match.start()<r['end'] for r in owners):
                # A declaration is not a paint invocation. Everything else,
                # including a painter hidden in a macro, remains an audit gap.
                line=text.splitlines()[text.count('\n',0,match.start())]
                if re.match(r'^\s*(?:static|extern|UI_DRAW|void|int|uint\d+_t)\b',line) and line.rstrip().endswith(';'):continue
                unresolved.append({'path':path,'line':text.count('\n',0,match.start())+1,'symbol':match[1],'source':line.strip(),'status':'REVIEW_REQUIRED'})
    semantic=[]
    for row in result:
        text=(root/row['path']).read_text()
        body=text[row['opening']:row['end']]
        for term in ('battery','playhead','pickup'):
            if re.search(r'\b'+term+r'\b',body,re.I):
                semantic.append({'id':row['id']+'::semantic-'+term,'renderer':row['id'],'term':term,
                                 'status':'NOT_RUN','scope':'Source mentions semantic domain; function reach cannot certify this branch or its pixels.'})
    lane_path=root/'firmware/src/app/ui_step_lane.inc';lane_labels=[]
    if lane_path.is_file():
        match=re.search(r'labels\[UI_LANE_COUNT\]\s*=\s*\{([^}]+)\}',lane_path.read_text())
        if match:lane_labels=re.findall(r'"([^"]+)"',match[1])
    return {'schema':1,'sources':sources,'renderers':result,'semanticSites':semantic,'unparsedPaintSites':unresolved,'stepLaneLabels':lane_labels,'productionRoot':'firmware/src/x0x.c',
            'scope':'Application C function painting/call closure, including inactive preprocessor branches. Runtime invocation does not prove branch, pixel or R22 conformance.',
            'status':'REVIEW_REQUIRED','visualApproval':False}

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--source',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    with a.output.open('x') as f:json.dump(inventory(a.source),f,indent=2);f.write('\n')
