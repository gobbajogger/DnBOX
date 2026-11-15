#!/usr/bin/env python3
"""Authenticate public original inputs and verify the complete sequence oracle body.
No downloading, build or runtime effects. Upstream root must be an exact public
6466a371... checkout or verified reconstruction, not a current unpinned checkout.
"""
import argparse,hashlib,json,re
from pathlib import Path
def sha(b):return hashlib.sha256(b).hexdigest()
def token(s):
 s=re.sub(r'/\*.*?\*/|//[^\n]*','',s,flags=re.S)
 return re.findall(r'[A-Za-z_]\w*|(?:\d+\.?\d*|\.\d+)(?:[eE][+-]?\d+)?[fFlLuU]*|::|==|!=|<=|>=|&&|\|\||\+\+|--|->|<<|>>|[^\s]',s)
def main():
 p=argparse.ArgumentParser();p.add_argument('--upstream-root',type=Path,required=True);a=p.parse_args();root=Path(__file__).resolve().parent
 m=json.loads((root/'SOURCE-INPUTS.json').read_text());auth={}
 for e in m['sources']+[m['parameter']]:
  b=(a.upstream_root/e['path']).read_bytes()
  if sha(b)!=e['sha256'] or hashlib.sha1(b'blob '+str(len(b)).encode()+b'\0'+b).hexdigest()!=e['blob']:raise SystemExit('upstream identity mismatch: '+e['path'])
  auth[e['path'].split('/')[-1]]=b.decode()
 def clean(s):return '\n'.join(x for x in s.split('\n')if not x.startswith(('#','@class')))
 body='\n'.join(clean(auth[n]) for n in ['S1ArpModes.hpp','S1Arpeggiator.hpp','S1Sequencer.hpp','S1Sequencer.mm'])
 body=body.replace('S1Sequencer','OriginalSequence').replace('private:','public:')
 body=body.replace('AEArray *heldNoteNumbersAE','const s1source_sequence::HeldView &held').replace('__weak const s1source_sequence::HeldView &held','const s1source_sequence::HeldView &held')
 body=body.replace('heldNoteNumbersAE.count','held.count')
 body=body.replace('AEArrayEnumeratePointers(heldNoteNumbersAE, NoteNumber *, note)','for(const NoteNumber *note=held.begin();note!=held.end();++note)')
 body=body.replace('AEArrayEnumeratePointers(heldNoteNumbersAE, NoteNumber *, noteStruct)','for(const NoteNumber *noteStruct=held.begin();noteStruct!=held.end();++noteStruct)')
 bound=(root/'original_sequence.hpp').read_text();start=bound.index('//\n//  S1ArpModes.hpp');bound=bound[start:bound.rfind('\n}')]
 bound=re.sub(r'\binline (?=(?:void|int|OriginalSequence::))','',bound)
 x,y=token(body),token(bound)
 if x!=y:
  for i,(u,v)in enumerate(zip(x,y)):
   if u!=v:raise SystemExit('sequence oracle token mismatch '+str(i)+': '+repr(x[max(0,i-8):i+8])+' != '+repr(y[max(0,i-8):i+8]))
  raise SystemExit('sequence oracle token count mismatch')
 enum=auth['S1Parameter.h'].split('typedef AK_ENUM(S1Parameter) {',1)[1].split('} S1Parameter;',1)[0]
 actual=(root/'sequence_types.hpp').read_text().split('enum Param {',1)[1].split('};',1)[0]
 if token(enum)!=token(actual):raise SystemExit('parameter enum mismatch')
 if not re.search(r'struct SeqNoteNumber\s*\{\s*int noteNumber;\s*int onOff;\s*int velocity;\s*\}',re.sub(r'//[^\n]*','',auth['S1SeqNoteNumber.hpp'])):raise SystemExit('original sequence-note ABI changed')
 print(json.dumps({'authenticated_public_inputs':6,'sequence_oracle_tokens':len(x),'parameter_enum_tokens':len(token(enum)),'arithmetic_and_callback_body_equal':True,'allowed_binding_changes':['imports/framework aliases','AEArray pointer-view enumeration','namespace/class name/visibility','inline header linkage','explicit local enum lookup'],'runtime_or_app_fidelity':False}))
if __name__=='__main__':main()
