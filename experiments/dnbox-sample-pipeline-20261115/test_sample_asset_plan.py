# SPDX-License-Identifier: GPL-3.0-only
import unittest,struct,hashlib,itertools
import sample_asset_plan as m
def record(i,group=None,n=None):
 n=n or 65+i
 return {'id':'personal-'+str(i),'group':group or('BR1'if i<3 else'BR2'),'zone':struct.pack('<5Ihh4B',123,n,0,n-1,32768,576+i*16,0,0,0,127,0),'data':bytes([i+1])*((n+1)//2)}
class Assets(unittest.TestCase):
 def setUp(self):self.c=[record(i)for i in range(5)];self.sha=m.catalog_digest(self.c)
 def p(self,images=None,caps=None,ids=None):return m.plan_install(self.c,self.sha,[None]*3 if images is None else images,[81920,122880,32768]if caps is None else caps,ids)
 def test_fresh_all_32_subsets_preserve_exact_zones_data_and_order(self):
  for mask in range(32):
   ids=[r['id']for i,r in enumerate(self.c)if mask>>i&1];p=self.p(ids=list(reversed(ids)))
   self.assertEqual(p['selected_ids'],ids);self.assertFalse(p['device_write_qualified'])
   actual=[m._zone(z,d)for b,c in zip(p['proposed_images'],p['capacity_bytes'])for z,d in m.parse_image(b,c)]
   self.assertEqual(actual,[m._zone(r['zone'],r['data'])for r in self.c if r['id']in ids])
 def test_default_all_and_none(self):
  self.assertEqual(self.p()['selected_ids'],[r['id']for r in self.c]);self.assertEqual(self.p(ids=[])['writes'],[])
 def test_existing_exact_defaults_noop(self):
  images=[m.compose_group(self.c[:3]),m.compose_group(self.c[3:]),None]
  self.assertEqual(self.p(images)['writes'],[]);self.assertEqual(self.p(images)['proposed_images'],images)
 def test_existing_data_never_rewritten_and_menu_prefix_kept(self):
  old=m.compose_group([record(8,'USER')]);images=[old,None,None]
  p=self.p(images,ids=['personal-0'])
  self.assertEqual(p['writes'][0]['slot_index'],1);self.assertEqual(images,[old,None,None]);self.assertEqual(p['proposed_images'][0],old)
 def test_holes_before_later_user_data_not_filled(self):
  old=m.compose_group([record(8,'USER')])
  with self.assertRaisesRegex(ValueError,'safe append'):self.p([None,None,old],ids=['personal-0'])
 def test_capacity_failure_no_partial_writes(self):
  old=m.compose_group([record(8,'USER')]);images=[old,None,None]
  with self.assertRaises(ValueError):self.p(images,[81920,520,520])
  self.assertEqual(images,[old,None,None])
 def test_unknown_corrupt_or_missing_current_images_reject(self):
  for images in [[b'notempty',None,None],['unknown',None,None],[None,None]]:
   with self.assertRaises(ValueError):self.p(images)
  bad=bytearray(m.compose_group([record(8,'USER')]));bad[-1]^=1
  with self.assertRaises(ValueError):self.p([bad,None,None])
 def test_catalog_authentication_and_zone_validation(self):
  for field,value in [('data',b'other'),('group','OTHER'),('id','wrong')]:
   c=[dict(r)for r in self.c];c[0][field]=value
   with self.assertRaises(ValueError):m.plan_install(c,self.sha,[None]*3,[81920,122880,32768])
  for offset,value in [(4,0),(8,99),(12,99),(16,0)]:
   c=[dict(r)for r in self.c];z=bytearray(c[0]['zone']);struct.pack_into('<I',z,offset,value);c[0]['zone']=bytes(z)
   with self.assertRaises(ValueError):m.plan_install(c,m.catalog_digest(c),[None]*3,[81920,122880,32768])
 def test_mutable_snapshots_are_frozen_before_return(self):
  old=bytearray(m.compose_group([record(8,'USER')]));images=[old,None,None]
  p=self.p(images,ids=['personal-0']);self.assertIsInstance(p['proposed_images'][0],bytes)
  stable=p['proposed_images'][0];old[-1]^=1
  self.assertEqual(p['proposed_images'][0],stable);self.assertEqual(m.sha(stable),p['existing_images_sha256'][0])
 def test_noncanonical_group_order_or_oversized_source_group_rejected(self):
  c=[record(0,'A'),record(1,'B'),record(2,'A')]
  with self.assertRaises(ValueError):m.plan_install(c,m.catalog_digest(c),[None]*3,[81920,122880,32768])
  c=[record(i,'A')for i in range(17)]
  with self.assertRaises(ValueError):m.plan_install(c,m.catalog_digest(c),[None]*3,[81920,122880,32768])
 def test_source_groups_not_flattened(self):
  p=self.p();self.assertEqual([w['asset_ids']for w in p['writes']],[['personal-0','personal-1','personal-2'],['personal-3','personal-4']])
if __name__=='__main__':unittest.main()

