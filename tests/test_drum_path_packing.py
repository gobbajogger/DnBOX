#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""All expanded paths and body mappings equal the published e9b geometry."""
import hashlib,json,re,unittest,os,subprocess,tempfile
from pathlib import Path
SOURCE=Path(__file__).resolve().parents[1]/'firmware/src/app/ui_drum_voice.inc'
EXPECTED='0a91bd0d2f7acb54391857bed726e9be623c183ac51b01e8d30efe998341d1de'
def expand(source):
    def body(name):
        m=re.search(name+r'\s*=\s*\{(.*?)\};',source,re.S)
        if not m:raise ValueError('missing '+name)
        return m.group(1)
    raw=bytes(map(int,re.findall(r'\d+',body(r'DV_XY_PACKED\[\]'))))
    if len(raw)!=1656 or raw[-2:]!=b'\0\0':raise ValueError('packed coordinate bounds')
    bits=int.from_bytes(raw,'little')
    xy=[(bits>>(15*i))&32767 for i in range(882)]
    packed=list(map(int,re.findall(r'\d+',body(r'DV_POLY\[\]'))))
    bodies=[tuple(map(int,p)) for p in re.findall(r'\{(\d+), (\d+)\}',body(r'DV_BODY\[2\]\[NDRUM\]'))]
    if len(packed)!=114 or len(bodies)!=22:raise ValueError('inventory')
    paths=[]
    for word in packed:
        off,n=word&511,word>>9
        if word>=32768 or n<1 or n>64 or 2*(off+n)>len(xy):raise ValueError('bounds')
        paths.append(xy[2*off:2*(off+n)])
    if any(first+n>len(paths) for first,n in bodies):raise ValueError('body bounds')
    return hashlib.sha256(json.dumps({'paths':paths,'bodies':bodies},separators=(',',':')).encode()).hexdigest()
class ExactPaths(unittest.TestCase):
    def test_all_published_paths(self):self.assertEqual(expand(SOURCE.read_text()),EXPECTED)
    def test_compiled_coordinate_decoder(self):
        s=SOURCE.read_text();start=s.index('static const uint8_t DV_XY_PACKED');end=s.index('/* Lossless 9-bit',start)
        with tempfile.TemporaryDirectory(prefix='dnbox-coordinate-') as tmp:
            d=Path(tmp);(d/'check.c').write_text('#include <stdint.h>\n#include <stdio.h>\n'+s[start:end]+'\nint main(void){for(unsigned i=0;i<882;i++){unsigned v=dv_coordinate(i);putchar(v&255);putchar(v>>8);}return 0;}\n')
            subprocess.run([os.environ.get('CC','cc'),'-std=c99','-Wall','-Wextra','-Werror',str(d/'check.c'),'-o',str(d/'check')],check=True,capture_output=True)
            data=subprocess.check_output([str(d/'check')]);self.assertEqual(len(data),1764)
            self.assertEqual(hashlib.sha256(data).hexdigest(),'504e54f772a7d2898a2801fd4b55a86b320970c2d604036b50525a0cbc53ed8b')
    def test_offset_mutation(self):
        s=SOURCE.read_text();a=s.index('DV_POLY[]');m=re.search(r'\d+',s[s.index('{',a)+1:]);start=s.index('{',a)+1+m.start();end=start+len(m.group());v=int(m.group())
        self.assertNotEqual(expand(s[:start]+str(v^1)+s[end:]),EXPECTED)
    def test_count_mutation(self):
        s=SOURCE.read_text();a=s.index('DV_POLY[]');m=re.search(r'\d+',s[s.index('{',a)+1:]);start=s.index('{',a)+1+m.start();end=start+len(m.group());v=int(m.group())
        self.assertNotEqual(expand(s[:start]+str(v^(1<<9))+s[end:]),EXPECTED)
if __name__=='__main__':unittest.main()
