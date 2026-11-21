#!/usr/bin/env python3
"""Verify visible text plus explicit actual opaque-popup ownership and same-source restoration.
Every raw finding stays in the input. Never grants a frame-wide exception.
"""
from pathlib import Path
import argparse,json,re,hashlib

def verify(log,proof,tree):
 lines=log.read_text().splitlines();proof_path=proof;proof=json.loads(proof.read_text());tree=json.loads(tree.read_text());expected={x['path']:x['sha1'] for x in tree['files']}
 assert proof['status']=='PASS' and all(expected.get(k)==v for k,v in proof['sources'].items()),'restoration proof source mismatch'
 assert (log.parent/'positive.exit').read_text().strip()=='0','positive renderer did not exit0'
 for kind in ['positive','negative']:
  assert hashlib.sha256((proof_path.parent/(kind+'.log')).read_bytes()).hexdigest()==proof[kind+'LogSHA256'],'restoration log hash mismatch'
 frames=proof['frames'];assert [r['state']['id'] for r in frames]==['tempo-layer-base','tempo-layer-overlay','tempo-layer-restored'],'missing actual layer images/state'
 for row in frames:
  assert hashlib.sha256((proof_path.parent/row['png']).read_bytes()).hexdigest()==row['sha256'],'layer image mismatch'
 assert frames[1]['state']['touched']==4 and frames[2]['state']['nowMs']>=frames[2]['state']['touchUntil'],'wrong actual layer state'
 assert len({(r['state']['view'],r['state']['part'],r['state']['overlay']) for r in frames})==1,'background owner changed'
 assert proof['outsidePopupChanges']==0 and proof['baseMainRestoration']=='BYTE_IDENTICAL' and proof['damagedRestorationRejected'] is True,'incomplete actual layer/restoration proof'
 rows=[json.loads(s[9:]) for s in lines if s.startswith('F2_EXTRA ')];occluded=[json.loads(s[13:]) for s in lines if s.startswith('F2_OCCLUSION ')]
 assert rows and [r['frame'] for r in rows]==list(range(len(rows))),'missing/duplicate frame audit'
 captures=[json.loads(s[9:]) for s in lines if s.startswith('F1_FRAME ')];states=[json.loads(s[9:]) for s in lines if s.startswith('F1_STATE ')]
 assert len(captures)>=len(rows) and [r['id'] for r in captures[-len(rows):]]==[r['id'] for r in rows],'actual frame without matching final audit'
 state_ids=[r['capture'] for r in states];assert len(state_ids)==len(set(state_ids)),'duplicate actual UI state'
 assert all(r['capture'] in state_ids for r in captures[-len(rows):]),'missing actual UI state'

 assert all(type(r['findings'])==int and type(r['audited'])==int and r['audited']>=0 for r in rows),'invalid audit counts'
 assert sum(r['findings'] for r in rows)==len(occluded),'raw finding without pixel ownership receipt'
 for item in occluded:
  assert item['id']=='readout-tempo' and item['active']==1 and item['labelLayer']==0,'unexpected/current-layer overlap'
  assert item['rect']==proof['popupBounds'] and item['missing']>0 and item['covered']==item['missing'] and item['visibleMissing']==0,'visible text pixel loss'
  assert item['text'] in ['BRK','CHI','MIA'],'unexpected hidden base text'
 # Text-free grids must execute the source-native pixel/bounds/nonmutation fixture.
 empty=[r for r in rows if not r['audited']]
 grids={re.match(r'ACTUAL_GRID (\S+) ',s)[1] for s in lines if s.startswith('ACTUAL_GRID ')}
 assert all(r['id'] in grids for r in empty),'unqualified text-free frame'
 grid_pass=re.search(r'PASS actual clip-grid renderer: (\d+) checks; five fixture states',log.read_text())
 assert not empty or (grid_pass and int(grid_pass[1])>0 and grids=={'clip-grid-empty','clip-grid-full-view0','clip-grid-queued-view0','clip-grid-queued-view1','clip-grid-compact-offset'}),'missing native grid component checks'
 grid_source=(log.parent/'source/tests/host/ui_clip_grid_render_test.c').read_bytes()
 assert hashlib.sha1(b'blob '+str(len(grid_source)).encode()+b'\0'+grid_source).hexdigest()==expected['tests/host/ui_clip_grid_render_test.c'],'grid component oracle source changed'

 return {'status':'PASS_WITH_EXPLICIT_LAYERS','productionTree':tree['tree'],'frames':len(rows),'opaqueTextAudits':sum(r['audited'] for r in rows),'rawFindingsRetained':len(occluded),'visibleOrUnexpectedFindings':0,'verifiedOccludedUnderlayPixels':sum(x['missing'] for x in occluded),'textFreeFramesWithNativeComponentChecks':len(empty),'nativeGridFixtures':len(grids),'nativeGridCumulativeCheckCounter':int(grid_pass[1]) if grid_pass else 0,'nativeDisplaySlices':sum(r['scope']=='DISPLAY_SLICE_ONLY' for r in rows),'directPreviewPainterFrames':sum(r['id'].startswith('preview-') for r in rows),'completeHostCompositionFrames':sum(r['scope']!='DISPLAY_SLICE_ONLY' and not r['id'].startswith('preview-') for r in rows),'scope':'Final visible opaque text geometry. Partial native display slices and direct previews are separately scoped, not whole native wrapper/R22 acceptance.'}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('log',type=Path);p.add_argument('--layer-proof',type=Path,required=True);p.add_argument('--production-tree',type=Path,required=True);a=p.parse_args()
 try:print(json.dumps(verify(a.log,a.layer_proof,a.production_tree)))
 except (AssertionError,ValueError,KeyError,TypeError) as e:print(json.dumps({'status':'FAIL','reason':str(e)}));raise SystemExit(1)
