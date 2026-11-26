"""Prove stock Clip Grid rectangle body can draw coherent tile strips; not integrated."""
from pathlib import Path
import tempfile,subprocess,sys
root=Path(__file__).resolve().parent
donor=root.parent/'s1-v8-font-exact/tree'
src=donor/'firmware/src'
t=(src/'app/main_fm1.c').read_text()
a=t.index('        smk_apc40_lcd_map_t m = smk_apc40_lcd_map(',t.index('uint8_t smk_clip_lcd_draw('))
b=t.index('\n    }\n    return 1u;',a)
actual=t[a:b]
# Keep actual stock dimensions, marks, colour, integer arithmetic and raster order.
assert actual.count('cv_rect')==5
with tempfile.TemporaryDirectory() as tmp:
    p=Path(tmp)
    (p/'clip-painter.inc').write_text(actual)
    for strip in (0,1):
        subprocess.run(['cc','-std=c99','-O2',f'-DSTRIP={strip}',f'-I{p}',f'-I{src}',str(root/'clip-test.c'),'-o',str(p/f'test-{strip}')],check=True)
    baseline=subprocess.check_output([p/'test-0',p/'old.raw']).decode()
    compact=subprocess.check_output([p/'test-1',p/'new.raw']).decode()
    assert baseline.splitlines()[0]==compact.splitlines()[0],(baseline,compact)
    subprocess.run(["cmp",p/"old.raw",p/"new.raw"],check=True)
    print(baseline+compact+f"Verified raw stream: {(p/'old.raw').stat().st_size} bytes exact")
