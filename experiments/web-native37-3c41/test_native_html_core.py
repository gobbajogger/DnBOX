import unittest,sys,re
from pathlib import Path
try: import native_html_core as core
except ModuleNotFoundError: core=None
ROOT=Path(__file__).parent/'source-restoration/native-3c41'
class CoreTests(unittest.TestCase):
 def setUp(self): self.assertIsNotNone(core,'source-native renderer is not implemented yet')
 def test_native_source_pin_and_font(self):
  self.assertTrue(core.verify_source(ROOT))
  fonts=core.decode_fonts(ROOT/'build/gen/felucca_font.h')
  self.assertEqual(set(fonts),{'T','XS','S','B','M','L'})
  self.assertEqual(sum(len(x['glyphs']) for x in fonts.values()),439)
  self.assertEqual(fonts['B']['height'],16)
 def test_section_background_is_not_last_edge_undercolour(self):
  f={'source':core.PIN,'sections':[{'x':0,'y':0,'w':2,'h':2,'background':0x1234,'commands':[[0,0,0,1,1,65535,0,2],[4,0,0,0,8,0,2]]}]}
  pixels,html=core.render_native(f,{})
  self.assertEqual(pixels[1],0x1234)
  self.assertIn('background:'+core.colour(0x1234),html)
  self.assertIn('data-native-section',html)
 def test_unknown_primitive_is_rejected(self):
  f={'source':core.PIN,'sections':[{'x':0,'y':0,'w':2,'h':2,'background':0,'commands':[[99]]}]}
  with self.assertRaises(ValueError):core.render_native(f,{})
 def test_drifted_source_is_rejected(self):
  with self.assertRaises(ValueError):core.render_native({'source':'wrong','sections':[]},{})
 def test_smooth_uses_original_outlines_native_positions_and_clip(self):
  fonts=core.decode_fonts(ROOT/'build/gen/felucca_font.h');outlines=core.outline_fonts(ROOT,fonts)
  self.assertIn('Q',outlines['B']['A']['path'])
  f={'source':core.PIN,'sections':[{'x':0,'y':0,'w':240,'h':20,'background':0,'commands':[[3,4,0,65535,'B','DnBOX 08',0,240,0,20]]}]}
  _,native=core.render_native(f,fonts);smooth=core.render_smooth(f,fonts,outlines)
  for text in ['DnBOX 08']:
   self.assertIn(text,native);self.assertIn(text,smooth)
  self.assertIn('viewBox="0 0 240 240"',smooth)
  self.assertIn('<path',smooth);self.assertIn('clipPath',smooth)
  self.assertNotIn('<image',smooth);self.assertNotIn('<canvas',smooth)
if __name__=='__main__':unittest.main()
