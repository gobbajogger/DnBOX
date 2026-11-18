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
    packed=bytes(map(int,re.findall(r'\d+',body(r'DV_POLY_PACKED\[\]'))))
    if len(raw)!=1385 or len(packed)!=243 or packed[-1]&252:raise ValueError('packed bounds')
    def decode(at,n):
        out=[]
        for _ in range(n):
            v=raw[at];at+=1
            if v<128:v=(v+8)*100
            else:v=((v&127)<<8)|raw[at];at+=1
            out.append(v)
        return out
    words=[(int.from_bytes(packed,'little')>>(17*i))&131071 for i in range(114)]
    bodies=[tuple(map(int,p)) for p in re.findall(r'\{(\d+), (\d+)\}',body(r'DV_BODY\[2\]\[NDRUM\]'))]
    if len(words)!=114 or len(bodies)!=22:raise ValueError('inventory')
    paths=[]
    for word in words:
        off,n=word&2047,word>>11
        if n<1 or n>64 or off>=len(raw):raise ValueError('bounds')
        paths.append(decode(off,2*n))
    if any(first+n>len(paths) for first,n in bodies):raise ValueError('body bounds')
    return hashlib.sha256(json.dumps({'paths':paths,'bodies':bodies},separators=(',',':')).encode()).hexdigest()
class ExactPaths(unittest.TestCase):
    def test_all_published_paths(self):self.assertEqual(expand(SOURCE.read_text()),EXPECTED)
    def test_compiled_coordinate_decoder(self):
        s=SOURCE.read_text();start=s.index('static const uint8_t DV_XY_PACKED');end=s.index('static const uint8_t DV_POLY_PACKED',start)
        with tempfile.TemporaryDirectory(prefix='dnbox-coordinate-') as tmp:
            d=Path(tmp);(d/'check.c').write_text('#include <stdint.h>\n#include <stdio.h>\n'+s[start:end]+'\nint main(void){unsigned at=0;for(unsigned i=0;i<882;i++){unsigned v=dv_coordinate_next(&at);putchar(v&255);putchar(v>>8);}return 0;}\n')
            subprocess.run([os.environ.get('CC','cc'),'-std=c99','-Wall','-Wextra','-Werror',str(d/'check.c'),'-o',str(d/'check')],check=True,capture_output=True)
            data=subprocess.check_output([str(d/'check')]);self.assertEqual(len(data),1764)
            self.assertEqual(hashlib.sha256(data).hexdigest(),'504e54f772a7d2898a2801fd4b55a86b320970c2d604036b50525a0cbc53ed8b')
    def test_compiled_all_path_decoder(self):
        s=SOURCE.read_text();start=s.index('static const uint8_t DV_XY_PACKED');end=s.index('typedef struct',start)
        main='int main(void){for(unsigned p=0;p<114;p++){unsigned word=dv_path(p),at=word&2047,n=word>>11;putchar(n);for(unsigned i=0;i<2*n;i++){unsigned v=dv_coordinate_next(&at);putchar(v&255);putchar(v>>8);}}return 0;}'
        with tempfile.TemporaryDirectory(prefix='dnbox-path-') as tmp:
            d=Path(tmp);(d/'check.c').write_text('#include <stdint.h>\n#include <stdio.h>\n'+s[start:end]+main)
            subprocess.run([os.environ.get('CC','cc'),'-std=c99','-Wall','-Wextra','-Werror',str(d/'check.c'),'-o',str(d/'check')],check=True,capture_output=True)
            data=subprocess.check_output([str(d/'check')]);at=0;paths=[]
            for _ in range(114):
                n=data[at];at+=1;self.assertTrue(1<=n<=64)
                paths.append([int.from_bytes(data[at+2*i:at+2*i+2],'little') for i in range(2*n)]);at+=4*n
            self.assertEqual(at,len(data))
            body=re.search(r'DV_BODY\[2\]\[NDRUM\]\s*=\s*\{(.*?)\};',s,re.S).group(1)
            bodies=[tuple(map(int,p)) for p in re.findall(r'\{(\d+), (\d+)\}',body)]
            self.assertEqual(hashlib.sha256(json.dumps({'paths':paths,'bodies':bodies},separators=(',',':')).encode()).hexdigest(),EXPECTED)
    def test_offset_mutation(self):
        s=SOURCE.read_text();a=s.index('DV_POLY_PACKED[]');m=re.search(r'\d+',s[s.index('{',a)+1:]);start=s.index('{',a)+1+m.start();end=start+len(m.group());v=int(m.group())
        self.assertNotEqual(expand(s[:start]+str(v^1)+s[end:]),EXPECTED)
    def test_count_mutation(self):
        s=SOURCE.read_text();a=s.index('DV_POLY_PACKED[]');begin=s.index('{',a)+1
        m=list(re.finditer(r'\d+',s[begin:]))[1];start=begin+m.start();end=begin+m.end();v=int(m.group())
        self.assertNotEqual(expand(s[:start]+str(v^8)+s[end:]),EXPECTED)
if __name__=='__main__':unittest.main()
