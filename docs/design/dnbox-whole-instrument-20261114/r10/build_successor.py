"""Compile scoped modal-priority prototype corrections into exact R9, not firmware."""
from pathlib import Path
import hashlib,json,os,re
R=Path(__file__).parent;B=Path(os.environ.get('DNBOX_R9_BASE',str(R.parent/'r9')))
sha=lambda b:hashlib.sha256(b).hexdigest()
raw=(B/'workbench.html').read_bytes();assert sha(raw)=='1b46efbc425e7922b7dd8772d41dc18c0ab88c9612e606eb2f33c2b386917163'
result=raw
for n in ['proposal.js','generator-proposal.js']:
 old=(B/n).read_bytes();new=(R/n).read_bytes();assert result.count(old)==1;result=result.replace(old,new)
s=result.decode();assert s.count('DnBOX R9 · workflow correction prototype')==1;s=s.replace('DnBOX R9 · workflow correction prototype','DnBOX R10 · modal-priority prototype')
s=s.replace('R9 · CORRECTIVE WORKFLOW PROPOSAL','R10 · CORRECTIVE WORKFLOW PROPOSAL').replace('R9 remains a proposal.','R10 remains a proposal.')
s=s.replace('R9 interaction corrections are separate from these stills.','R10 interaction corrections are separate from these stills.')
s=s.replace('noteName(48+i)','noteName(36+i)')
s=s.replace('Printed controller octaves and stored/displayed MIDI pitches remain distinct.','Key buttons label modeled MIDI notes at the current octave when the pitch route is admitted. Accessible labels retain the source hardware-map names. GRID roles and physical pitch/listening remain separate.')
s=s.replace('The37-key hardware is C3–C6.','The source hardware-map names are C3–C6.')
s,n=re.subn(r'<p class="notice">R9 corrects.*?</p>','<p class="notice">R10 restores native input ownership across questions, lists and modifier chords: LIST sound controls edit base values; PATCH plus K2 owns tempo; BT plus REC asks before clearing the selected part. Clear targets the then-current pattern and its owned lanes, with isolated modeled recovery. Question Redo keeps the question and caller; ordinary PLAY answers NO then toggles once. Current glyphs, lower controls and direct GEN correction remain. Curated stills retain R7/R8 identities. The full overhaul remains unfinished across FX/controllers, connected use, populated BREAK, recording and durable recovery. This model produces no firmware audio or saved project.</p>',s,count=1)
assert n==1
s,n=re.subn(r'<li>Two reproduced prototype dispatch faults are corrected here:.*?</li>','<li>R10 corrects the inherited question/Redo policy and caller restoration. Independent assessment of this exact successor is pending. Whole-instrument design remains incomplete.</li>',s,count=1)
assert n==1
result=s.encode();(R/'workbench.html').write_bytes(result)
receipt={'state':'UNFROZEN_WORKING_SUCCESSOR; NO_INDEPENDENT_ACCEPTANCE','base_head':'0fdaa20c8ca641bc5bc930f46307f979db8b6d9f','base_html_sha256':sha(raw),'html_sha256':sha(result),'renderer_sha256':sha((R/'proposal.js').read_bytes()),'generator_sha256':sha((R/'generator-proposal.js').read_bytes()),'bytes':len(result),'scope':'Shared source-priority dispatch, LIST base/lock eligibility, tempo and clear confirmation/recovery, ASK policy/feedback, inspection MIDI labels; R9fixes retained','limits':'Native quiet grouping/pool, active capture endpoint finalization and pattern/lane interleaving not simulated; no firmware/device/whole-overhaul acceptance'}
(R/'WORKING-INPUTS.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt))
