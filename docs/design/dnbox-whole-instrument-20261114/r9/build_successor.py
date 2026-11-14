"""Compile the scoped history/SAVE modifier fixes into exact R8; not firmware."""
from pathlib import Path
import hashlib,json,os,re
R=Path(__file__).parent
B=Path(os.environ.get('DNBOX_R8_BASE',str(R.parent/'r8')))
sha=lambda b:hashlib.sha256(b).hexdigest()
raw=(B/'workbench.html').read_bytes()
assert sha(raw)=='83bef5fb58cacf2119cd2431bdf77e1d97ce6b65a496792f4e58dd8defdf1d3e'
old=(B/'generator-proposal.js').read_bytes();new=(R/'generator-proposal.js').read_bytes()
assert sha(old)=='071c63e0478256662d084b1983a58f29ba19c6d36dc5b7f4a5f6371fdfb75041'
assert raw.count(old)==1
result=raw.replace(old,new)
s=result.decode()
for before,after in [
 ('DnBOX R8 · corrected workflow study','DnBOX R9 · workflow correction prototype'),
 ('R8 · CORRECTIVE WORKFLOW PROPOSAL','R9 · CORRECTIVE WORKFLOW PROPOSAL'),
 ('R8 remains a proposal.','R9 remains a proposal.'),
 ('Exact R7 proposal pixels reused; R8 interaction corrections are separate from these stills.','Exact R7 proposal pixels reused; R9 interaction corrections are separate from these stills.')]:
 assert s.count(before)==1
 s=s.replace(before,after)
s,n=re.subn(r'<p class="notice">R8 corrects.*?</p>','<p class="notice">R9 corrects two prototype input-priority faults: LIST Undo/Redo chords and BT plus lower GEN controls. It retains direct select-only GEN correction, native held-control grammar and all generator controls on release. Fifteen curated stills reuse exact R7 proposal pixels; interaction corrections are separate. Native firmware already supplies the useful ADSR/Juno vocabulary, large values and direct lower controls. The full overhaul remains unfinished across FX/controllers, real connected use, populated BREAK, running/poly recording and durable recovery. This model produces no firmware audio or saved project.</p>',s,count=1)
assert n==1
s,n=re.subn(r'<li>Independent R7 review is complete:.*?</li>','<li>Two reproduced prototype dispatch faults are corrected here: LIST recovery chords and BT plus GEN controls. Independent assessment of this exact successor is pending. The whole-instrument design remains incomplete.</li>',s,count=1)
assert n==1
result=s.encode()
(R/'workbench.html').write_bytes(result)
receipt={'state':'UNFROZEN_WORKING_SUCCESSOR; NO_INDEPENDENT_ACCEPTANCE','base_head':'bece3d1762116d7cd31afd0b83cc93dd9031a009','base_html_sha256':sha(raw),'generator_sha256':sha(new),'html_sha256':sha(result),'bytes':len(result),'changed_scope':'LIST history chord and GEN SAVE+K5–K8 precedence/consumed release; ASK/clip masks retained','unchanged':'All other embedded exact R8 assets/source; no new visual renders or PDF'}
(R/'WORKING-INPUTS.json').write_text(json.dumps(receipt,indent=2)+'\n')
print(json.dumps(receipt))
