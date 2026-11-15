import importlib.util,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
s=importlib.util.spec_from_file_location('build',ROOT/'tools/build.py');b=importlib.util.module_from_spec(s);s.loader.exec_module(b)
EXPECTED=['app/dnr_remote/protocol-v2/'+n+'.c' for n in ('remote_core','remote_wire','boot_incarnation')]
class CommonLinkUnits(unittest.TestCase):
 def test_common_units_declared_once(self):
  self.assertEqual(b.APP_COMMON_UNITS,EXPECTED)
  self.assertEqual(b.HOST_UNITS,b.O2_UNITS+b.APP_COMMON_UNITS)
  self.assertEqual(len(b.HOST_UNITS),len(set(b.HOST_UNITS)))
 def test_common_native_size_policy_and_order(self):
  b.BOARD='smk37';b.MODEL='elite'
  commands,objects=b.app_compile_commands([*b.CFLAGS,*b.FPU,'-DDNR_NATIVE_ENABLE=1'],True,'0')
  common=[c for c in commands if '-c' in c and any(str(c[c.index('-c')+1]).endswith(u)for u in EXPECTED)]
  self.assertEqual(len(common),3);self.assertEqual(len(objects),len(set(objects)))
  self.assertEqual(common,commands[-3:])
  for c in common:self.assertIn('-Os',c);self.assertNotIn('-O2',c);self.assertIn('-ffp-contract=off',c)
 def test_host_entry_uses_shared_manifest(self):
  self.assertIn('build.HOST_UNITS',(ROOT/'host/build_host.sh').read_text())
if __name__=='__main__':unittest.main()
