import os,sys,unittest,hashlib,importlib.util,subprocess,tempfile
from pathlib import Path
TOOLS=Path(os.environ.get('DNBOX_SOURCE_ROOT',str(Path(__file__).resolve().parents[2])))/'tools';sys.path.insert(0,str(TOOLS));import break_pack
MODULE=Path(__file__).with_name('selected_bank.py')
class SelectedBank(unittest.TestCase):
 def setUp(self):
  self.assertTrue(MODULE.exists(),'Missing rate-preserving selector/header implementation')
  spec=importlib.util.spec_from_file_location('selected_bank',MODULE);self.m=importlib.util.module_from_spec(spec);spec.loader.exec_module(self.m)
  self.loops=[{'name':'B'+str(i),'nsamples':65+i,'bars':1,'adpcm':bytes([i+1])*((66+i)//2)}for i in range(5)]
  self.bank=break_pack.encode_pack(self.loops,22050);self.digest=hashlib.sha256(self.bank).hexdigest()
  self.assets=[{'id':'loop-'+str(i),'name':v['name'],'frames':v['nsamples'],'sample_rate':22050,'adpcm_sha256':hashlib.sha256(v['adpcm']).hexdigest()}for i,v in enumerate(self.loops)]
 def choose(self,ids=None,**kw):return self.m.select_pack(self.bank,self.digest,self.assets,ids,**kw)
 def test_default_all_exact_and_all32_subsets(self):
  self.assertEqual(self.choose(),self.bank)
  for mask in range(32):
   ids=[v['id']for i,v in enumerate(self.assets)if mask>>i&1];out=self.choose(list(reversed(ids)))
   if not ids:self.assertIsNone(out);continue
   rate,loops=break_pack.parse_pack(out);self.assertEqual(rate,22050);self.assertEqual(loops,[v for i,v in enumerate(self.loops)if mask>>i&1])
 def test_header_keeps22050_and_c_bytes_frames(self):
  h=self.m.header_for_pack(self.choose());self.assertIn('#define X0X_BREAK_RATE 22050u',h)
  code='#include "bank.h"\n_Static_assert(X0X_BREAK_RATE==22050u,"rate changed");\nint main(void){return X0X_NBREAKS!=5||X0X_BREAKS[4].n!=69||X0X_BREAKS[4].adpcm[0]!=5;}\n'
  with tempfile.TemporaryDirectory(prefix='selected-bank-test-')as td:
   p=Path(td);(p/'bank.h').write_text(h);(p/'probe.c').write_text(code);subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(p/'probe.c'),'-o',str(p/'probe')],check=True);subprocess.run([str(p/'probe')],check=True)
 def test_none_has_no_asset_arrays(self):
  h=self.m.header_for_pack(self.choose([]));self.assertIn('#define X0X_NBREAKS 0',h);self.assertNotIn('X0X_BREAK_0',h)
 def test_bad_selection_and_bank_fail_closed(self):
  for ids in [['unknown'],['loop-0','loop-0'],'loop-0']:
   with self.assertRaises(ValueError):self.choose(ids)
  with self.assertRaises(ValueError):self.m.select_pack(self.bank,'0'*64,self.assets)
  altered=bytearray(self.bank);altered[-1]^=1
  with self.assertRaises(ValueError):self.m.select_pack(altered,hashlib.sha256(altered).hexdigest(),self.assets)
 def test_asset_metadata_and_rate_must_match(self):
  for field,value in [('frames',99),('adpcm_sha256','0'*64),('sample_rate',44100),('name','other'),('id','loop-1')]:
   assets=[dict(v)for v in self.assets];assets[0][field]=value
   with self.assertRaises(ValueError):self.m.select_pack(self.bank,self.digest,assets)
 def test_metadata_types_and_format_fail_closed(self):
  for field,value in [('frames',65.0),('sample_rate',22050.0),('channels',2),('codec','PCM16'),('bars',True)]:
   assets=[dict(v)for v in self.assets];assets[0][field]=value
   with self.assertRaises(ValueError):self.m.select_pack(self.bank,self.digest,assets)
 def test_declared_decoder_format_and_count_must_match(self):
  for field,value in [('channels',True),('channels',1.0),('channels',1+0j),('initial_predictor',123),('initial_index',88),('adpcm_bytes',999)]:
   assets=[dict(v)for v in self.assets];assets[0][field]=value
   with self.assertRaises(ValueError):self.m.select_pack(self.bank,self.digest,assets)
 def test_header_c_question_mark_names_survive_trigraph_translation(self):
  loops=[dict(v)for v in self.loops];loops[0]['name']='??/n';loops[1]['name']='??=x'
  h=self.m.header_for_pack(break_pack.encode_pack(loops,22050))
  code='#include "bank.h"\n#include <string.h>\nint main(void){return strcmp(X0X_BREAKS[0].name,"?""?/n")||strcmp(X0X_BREAKS[1].name,"?""?=x");}\n'
  with tempfile.TemporaryDirectory(prefix='selected-bank-question-')as td:
   p=Path(td);(p/'bank.h').write_text(h);(p/'probe.c').write_text(code);subprocess.run(['cc','-std=c11','-trigraphs','-Wall','-Wextra','-Werror',str(p/'probe.c'),'-o',str(p/'probe')],check=True);subprocess.run([str(p/'probe')],check=True)
 def test_capacity_failure_preserves_input_and_reserved_tail(self):
  src=bytes(range(64))+b'\0'+b'\xff'*64+b'USER_DATA'
  with self.assertRaises(ValueError):break_pack.place_pack(src,self.choose(),129)
  self.assertEqual(src,bytes(range(64))+b'\0'+b'\xff'*64+b'USER_DATA')
if __name__=='__main__':unittest.main()
