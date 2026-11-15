import unittest
from pathlib import Path
import preview_server as web
from native_session import NativeSession
class Preview37Tests(unittest.TestCase):
 def test_static_allowlist_serves_only_reviewed_shell_and_counted_module(self):
  asset=getattr(web,'static_asset',None);self.assertIsNotNone(asset,'the37shell module route must be explicit, not arbitrary path traversal')
  self.assertEqual(asset('/browser-input/input.mjs'),Path(web.BASE/'browser-input/input.mjs'))
  self.assertEqual(asset('/'),Path(web.ROOT/'index.html'))
  for x in ('/../native_session.py','/%2e%2e/native_session.py','/source-restoration/native-3c41/firmware/src/app/ui.c','/browser-input/../native_server.c','/missing','//etc/passwd'):
   with self.assertRaises(ValueError):asset(x)
 def test_actual_shell_transitive_module_css_and_font_dependency_closure(self):
  import re,posixpath
  visited=set();pending=['/index.html']
  while pending:
   url=pending.pop()
   if url in visited:continue
   try:path=web.static_asset(url)
   except ValueError:self.fail('actual shell dependency is not served: '+url)
   self.assertTrue(path.is_file(),url);visited.add(url)
   if path.suffix not in ('.html','.js','.mjs','.css'):continue
   source=path.read_text();references=[]
   if path.suffix=='.html':references+=re.findall(r'(?:src|href)=["\']([^"\']+)["\']',source)
   if path.suffix in ('.js','.mjs'):references+=re.findall(r'from\s+["\']([^"\']+)["\']',source)
   if path.suffix=='.css':references+=re.findall(r'url\(([^)]+)\)',source)
   for ref in references:
    ref=ref.strip('"\'')
    if ref.startswith(('http:','https:','#','data:')):continue
    pending.append(posixpath.normpath(posixpath.join(posixpath.dirname(url),ref)))
  self.assertTrue({'/held-input.mjs','/surface.mjs','/browser-input/input.mjs'}.issubset(visited),visited)
 def test_real_C_API_exposes_37bit_acknowledgement_and_same_renderer(self):
  s=NativeSession()
  try:
   web.handle_event({'command':'fixture','args':[8]},s)
   f=web.handle_event({'command':'physical','args':[36,100]},s)
   self.assertEqual(f['physical']['held'],1<<36);self.assertEqual(f['live'][0]['gate'],1)
   self.assertIn('native_html',f);self.assertIn('smooth_html',f);self.assertNotIn('sections',f);self.assertNotIn('native_rgb565',f)
   self.assertTrue(f['proof']['native_bytes_equal'])
   with self.assertRaises(ValueError):web.handle_event({'command':'memory','args':[0]},s)
  finally:s.close()
if __name__=='__main__':unittest.main()
