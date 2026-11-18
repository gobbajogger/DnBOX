# SPDX-License-Identifier: GPL-3.0-only
"""First slice only. All decisions consume actual render traces, never param-name captions.

The 70% content bounding-box / 12% screen empty-rectangle contracts are raw
measurements, not aesthetic acceptance. Thin glyphs and sparse layouts can fail.
"""
from collections import defaultdict

LABEL_WHITELIST = {'Q': 'standard filter resonance notation'}
CRYPTIC_LABELS = {'P.DPTH'}


def largest_empty_rectangle(mask):
    """Exact O(width*height) maximum empty axis-aligned integer-pixel rectangle."""
    if not mask or not mask[0]: return {'area': 0, 'bbox': None}
    heights = [0] * len(mask[0]); best = {'area': 0, 'bbox': None}
    for y, row in enumerate(mask):
        for x, occupied in enumerate(row): heights[x] = 0 if occupied else heights[x]+1
        stack = []
        for x, height in enumerate(heights + [0]):
            start = x
            while stack and stack[-1][1] > height:
                left, h = stack.pop(); area = (x-left)*h; start = left
                if area > best['area']: best = {'area':area,'bbox':[left,y+1-h,x,y+1]}
            if not stack or stack[-1][1] < height: stack.append((start,height))
    return best


def _union(a, b):
    if not a: return b
    if not b: return a
    return [min(a[0],b[0]),min(a[1],b[1]),max(a[2],b[2]),max(a[3],b[3])]


def _coalesce(events):
    out = {}
    for event in events:
        # Multiple clipped band passes share intended origin; distinct positions
        # stay distinct even if they reference the same parameter.
        key = (event['kind'], event.get('ref'), tuple(event.get('origin', [])),
               event.get('row'), event.get('control'), event.get('text'), event.get('glyphType'))
        if key not in out: out[key] = dict(event)
        else:
            out[key]['bbox'] = _union(out[key].get('bbox'),event.get('bbox'))
            out[key]['pixels'] = out[key].get('pixels',0)+event.get('pixels',0)
    return list(out.values())


def inspect_screen(screen):
    events = _coalesce(screen['events'])
    glyphs = [e for e in events if e['kind']=='glyph' and e.get('pixels',0)>0]
    checks = {str(i):{'status':'PASS','failures':[]} for i in range(1,5)}
    byref = defaultdict(list); bytype = defaultdict(list)
    for g in glyphs:
        byref[g['ref']].append(g); bytype[(g['ref'],g['glyphType'])].append(g)
    for ref, gs in byref.items():
        if len(gs)>1: checks['1']['failures'].append({'reason':'duplicate-visible-parameter','ref':ref,'controls':gs})
    for (ref, typ), gs in bytype.items():
        if len(gs)>1: checks['2']['failures'].append({'reason':'duplicate-glyph-and-typed-ref','ref':ref,'glyphType':typ})
    # Semantic route observed inside actual renderer, not duplicate-ref fiction.
    for g in glyphs:
        if g.get('name')=='Pitch' and g['glyphType']=='tuning-fork':
            checks['2']['failures'].append({'reason':'pitch-ratio-misrepresented-as-tune-fork','ref':g['ref'],'bbox':g.get('bbox')})
    has_final=any(e['kind']=='finaltext' for e in events)
    texts = [e for e in events if e['kind']==('finaltext' if has_final else 'text') and e.get('pixels',0)>0 and (not has_final or e.get('finalFrameMatchRatio',0)>=.90)]
    label_observations = []
    for g in glyphs:
        labels = [e for e in texts if e.get('row')==g.get('row') and
                  (e.get('control')==g.get('control') or (e.get('control')==-1 and len({x.get('control') for x in glyphs if x.get('row')==g.get('row')})==1)) and
                  (e.get('control')!=-1 or e.get('text') not in {'SOUND','KIT'}) and '/' not in e.get('text','')]
        actual = [e['text'].strip() for e in labels if e.get('text','').strip()]
        valid = [t for t in actual if t.upper() not in CRYPTIC_LABELS and
                 (len(t)>=2 or t in LABEL_WHITELIST)]
        label_observations.append({'ref':g['ref'],'row':g.get('row'),'control':g.get('control'),'paintedCaptions':actual})
        if not valid:
            checks['4']['failures'].append({'reason':'missing-short-or-cryptic-painted-caption','ref':g['ref'],'actual':actual})
    # Axis ink can never stand in for the observed native envelope itself.
    # The body hook marks a requested graph; only the actual polyline hook
    # supplies envelope-curve pixels, separately from guides and labels.
    if any(e['kind']=='envelope-body' for e in events) and not any(
            e['kind']=='envelope-curve' and e.get('pixels',0)>0 for e in events):
        checks['3']['failures'].append({'reason':'missing-native-envelope-curve'})
    width=screen.get('width',240); height=screen.get('height',240)
    mask=[[c=='1' if isinstance(c,str) else bool(c) for c in row] for row in screen['mask']]
    panels=[]
    for panel in screen['panels']:
        x0,y0,x1,y1=panel['bbox']; x0=max(0,x0);y0=max(0,y0);x1=min(width,x1);y1=min(height,y1)
        if x1<=x0 or y1<=y0: continue
        sub=[row[x0:x1] for row in mask[y0:y1]]; points=[(x,y) for y,row in enumerate(sub) for x,v in enumerate(row) if v]
        box=[min(x for x,y in points), min(y for x,y in points), max(x for x,y in points)+1,max(y for x,y in points)+1] if points else None
        ratio=((box[2]-box[0])*(box[3]-box[1])/((x1-x0)*(y1-y0))) if box else 0
        empty=largest_empty_rectangle(sub); er=empty['area']/(width*height)
        p={**panel,'bbox':[x0,y0,x1,y1],'contentBbox':([box[0]+x0,box[1]+y0,box[2]+x0,box[3]+y0] if box else None),
           'contentBboxRatio':ratio,'inkPixels':len(points),'largestEmptyRectangle':empty,'emptyScreenRatio':er}
        panels.append(p)
        if ratio<.70: checks['3']['failures'].append({'reason':'panel-content-bbox-below-70-percent','panel':panel['id'],'actual':ratio})
        if er>.12: checks['3']['failures'].append({'reason':'empty-rectangle-above-12-percent-screen','panel':panel['id'],'actual':er})
    # D01 actual value/glyph bounds collisions are supplemental evidence. They do
    # not silently expand check1 duplicate identity into a layout check.
    overlaps=[]
    for typ in ('glyph','value','text'):
        es=[e for e in events if e['kind']==('finaltext' if typ=='text' and has_final else typ) and e.get('bbox') and e.get('pixels',0)>0]
        for i,a in enumerate(es):
            for b in es[i+1:]:
                if typ!='text' and a.get('ref')==b.get('ref'): continue
                if typ=='text' and a.get('origin')==b.get('origin') and a.get('text')==b.get('text'):continue
                aa=a['bbox'];bb=b['bbox']; area=max(0,min(aa[2],bb[2])-max(aa[0],bb[0]))*max(0,min(aa[3],bb[3])-max(aa[1],bb[1]))
                if area: overlaps.append({'kind':typ,'refs':[a.get('ref'),b.get('ref')],'captions':[a.get('text'),b.get('text')],'intersectionArea':area,'bounds':[aa,bb]})
    for i,c in checks.items():
        if c['failures']: c['status']='FAIL'
        elif (i in {'1','2','4'} and not glyphs) or (i=='3' and not panels):c['status']='NOT_RUN'
    return {'id':screen['id'],'checks':checks,'measurements':{'panels':panels,'labels':label_observations},'overlaps':overlaps,'coalescedEvents':events}
